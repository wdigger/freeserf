/*
 * audio-win.cc - Music and sound effects playback.
 *
 * Copyright (C) 2019  Wicked_Digger <wicked_digger@mail.ru>
 *
 * This file is part of freeserf.
 *
 * freeserf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * freeserf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freeserf.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "windows/audio-win.h"

#include <string.h>
#include <algorithm>

#include "src/log.h"
#include "src/freeserf_endian.h"
#include "src/data.h"
#include "src/data-source.h"

Audio &
Audio::get_instance() {
  static AudioWin instance;
  return instance;
}

AudioWin::AudioWin() {
  sfx_player = std::make_shared<PlayerSFX>();
  midi_player = std::make_shared<PlayerMIDI>();
}

AudioWin::~AudioWin() {
}

void
AudioWin::set_volume(float volume_) {
  volume = volume_;

  if (midi_player) {
    Audio::PVolumeController volume_controller =
                                          midi_player->get_volume_controller();
    if (volume_controller) {
      volume_controller->set_volume(volume_controller->get_volume() * volume);
    }
  }

  if (sfx_player) {
    Audio::PVolumeController volume_controller =
                                           sfx_player->get_volume_controller();
    if (volume_controller) {
      volume_controller->set_volume(volume_controller->get_volume() * volume);
    }
  }
}

void
AudioWin::volume_up() {
  float vol = get_volume();
  set_volume(vol + 0.1f);
}

void
AudioWin::volume_down() {
  float vol = get_volume();
  set_volume(vol - 0.1f);
}

AudioWin::PlayerSFX::PlayerSFX() {
  WAVEFORMATEX format = { 0 };
  format.wFormatTag = WAVE_FORMAT_PCM;
  format.nChannels = 1;
  format.nSamplesPerSec = 8000;
  format.nAvgBytesPerSec = 16000;
  format.nBlockAlign = 2;
  format.wBitsPerSample = 16;
  format.cbSize = 0;
  MMRESULT res = ::waveOutOpen(&hWaveOut, WAVE_MAPPER, &format,
                               (DWORD_PTR)waveOutProc, (DWORD_PTR)this,
                               CALLBACK_FUNCTION);
  if (res != MMSYSERR_NOERROR) {
    Log::Error["audio-win"] << "Failed to open wave out.";
  }

  set_volume(1.f);
}

AudioWin::PlayerSFX::~PlayerSFX() {
  MMRESULT res = WAVERR_STILLPLAYING;
  while (res == WAVERR_STILLPLAYING) {
    res = ::waveOutClose(hWaveOut);
    if (res == WAVERR_STILLPLAYING) {
      ::Sleep(100);
    }
  }
}

void
AudioWin::PlayerSFX::enable(bool enable) {
}

Audio::PTrack
AudioWin::PlayerSFX::create_track(int track_id) {
  Data &data = Data::get_instance();
  PDataSource data_source = data.get_data_source();

  PBuffer wav = data_source->get_sound(track_id);
  if (!wav) {
    return nullptr;
  }

  return std::make_shared<TrackSFX>(wav, hWaveOut);
}

AudioWin::TrackSFX::TrackSFX(PBuffer buffer, HWAVEOUT wave_out)
  : data(buffer)
  , hWaveOut(wave_out)
  , wave_header(nullptr) {
}

AudioWin::TrackSFX::~TrackSFX() {
  if (wave_header != nullptr) {
    delete wave_header;
    wave_header = nullptr;
  }
}

void
AudioWin::TrackSFX::play() {
  if (hWaveOut == NULL) {
    return;
  }

  if (wave_header == NULL) {
    size_t header_size = sizeof(WAVEHDR);
    wave_header = new WAVEHDR;
    memset(wave_header, 0, header_size);
    wave_header->lpData = (LPSTR)data->get_data();
    wave_header->dwBufferLength = (DWORD)data->get_size();
    wave_header->dwFlags = 0;
    MMRESULT res = ::waveOutPrepareHeader(hWaveOut, wave_header,
                                          (UINT)header_size);
    if (res != MMSYSERR_NOERROR) {
      Log::Error["audio-win"] << "Failed to prepare header.";
      return;
    }
  }
  MMRESULT res = ::waveOutWrite(hWaveOut, wave_header, sizeof(WAVEHDR));
  if (res != MMSYSERR_NOERROR) {
    Log::Error["audio-win"] << "Failed to play WAVE track.";
    return;
  }
}

void
AudioWin::PlayerSFX::stop() {
  ::waveOutPause(hWaveOut);
}

float
AudioWin::PlayerSFX::get_volume() {
  DWORD  vol = 0;
  MMRESULT res = ::waveOutGetVolume(hWaveOut, &vol);
  if (res != MMSYSERR_NOERROR) {
    Log::Error["audio-win"] << "Failed to get WAVE volume.";
    return 0.f;
  }

  float volume = static_cast<float>(vol & 0xFFFF) / static_cast<float>(0xFFFF);

  return volume;
}

void
AudioWin::PlayerSFX::set_volume(float volume) {
  WORD vol = ((DWORD)(static_cast<float>(0xFFFF) * volume)) & 0xFFFF;
  MMRESULT res = ::waveOutSetVolume(hWaveOut, (vol << 16) | vol);
  if (res != MMSYSERR_NOERROR) {
    Log::Error["audio-win"] << "Failed to set WAVE volume.";
  }
}

void
AudioWin::PlayerSFX::volume_up() {
  float volume = get_volume() + 0.1f;
  if (volume > 1.f) {
    volume = 1.f;
  }
  set_volume(volume);
}

void
AudioWin::PlayerSFX::volume_down() {
  float volume = get_volume() - 0.1f;
  if (volume < 0.f) {
    volume = 0.f;
  }
  set_volume(volume);
}

void CALLBACK
AudioWin::PlayerSFX::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance,
                                 DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
  PlayerSFX *_this = reinterpret_cast<PlayerSFX*>(dwInstance);
  if (_this != NULL) {
    _this->waveCallback(hwo, uMsg, dwParam1, dwParam2);
  }
}

void
AudioWin::PlayerSFX::waveCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwParam1,
                                  DWORD_PTR dwParam2) {
  if (uMsg == WOM_DONE) {
    // ToDo (Digger): notify sound played if needed
  }
}

AudioWin::PlayerMIDI::PlayerMIDI() {
  UINT uDeviceID = 0;
  MMRESULT res = ::midiStreamOpen(&hMidiStream, &uDeviceID, 1,
                                  (DWORD_PTR)midiOutProc, (DWORD_PTR)this,
                                  CALLBACK_FUNCTION);
  if (res != MMSYSERR_NOERROR) {
    hMidiStream = NULL;
    Log::Error["audio-win"] << "Failed to open midi stream.";
    return;
  }

  set_volume(1.f);
}

AudioWin::PlayerMIDI::~PlayerMIDI() {
  if (hMidiStream != NULL) {
    ::midiStreamClose(hMidiStream);
    hMidiStream = NULL;
  }
}

void CALLBACK
AudioWin::PlayerMIDI::midiOutProc(HMIDIOUT hmo, UINT uMsg, DWORD_PTR dwInstance,
                                  DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
  PlayerMIDI *_this = reinterpret_cast<PlayerMIDI*>(dwInstance);
  if (_this != NULL) {
    _this->midiCallback(hmo, uMsg, dwParam1, dwParam2);
  }
}

void
AudioWin::PlayerMIDI::midiCallback(HMIDIOUT hwo, UINT uMsg, DWORD_PTR dwParam1,
                                   DWORD_PTR dwParam2) {
  switch (uMsg) {
    case MOM_DONE: {
      MMRESULT res = ::midiStreamRestart(hMidiStream);
      if (res != MMSYSERR_NOERROR) {
        Log::Error["audio-win"] << "Failed to restart midi stream.";
        return;
      }
      if (current_track != NULL) {
        if (!current_track->play_next_buffer()) {
          midi_track_finished();
        }
      }
      break;
    }
  }
}

Audio::PTrack
AudioWin::PlayerMIDI::create_track(int track_id) {
  Data &data = Data::get_instance();
  PDataSource data_source = data.get_data_source();

  size_t size = 0;
  PBuffer midi = data_source->get_music(track_id);
  if (!midi) {
    return nullptr;
  }

  PTrackMIDI midi_track = std::make_shared<TrackMIDI>((TypeMidi)track_id,
                                                      hMidiStream);
  if (!midi_track->create(midi)) {
    midi_track = nullptr;
  }

  return midi_track;
}

Audio::PTrack
AudioWin::PlayerMIDI::play_track(int track_id) {
  if ((track_id <= TypeMidiNone) || (track_id > TypeMidiTrackLast)) {
    track_id = TypeMidiTrack0;
  }

  Audio::PTrack track = Audio::Player::play_track(track_id);

  if (track) {
    current_track = std::dynamic_pointer_cast<TrackMIDI>(track);
  }

  return track;
}

void
AudioWin::PlayerMIDI::enable(bool enable) {
}

void
AudioWin::PlayerMIDI::stop() {
}

float
AudioWin::PlayerMIDI::get_volume() {
  DWORD  vol = 0;
  MMRESULT res = ::midiOutGetVolume((HMIDIOUT)hMidiStream, &vol);
  if (res != MMSYSERR_NOERROR) {
    Log::Error["audio-win"] << "Failed to get MIDI volume.";
    return 0.f;
  }

  float volume = static_cast<float>(vol & 0xFFFF) / static_cast<float>(0xFFFF);

  return volume;
}

void
AudioWin::PlayerMIDI::set_volume(float volume) {
  WORD vol = ((DWORD)(static_cast<float>(0xFFFF) * volume)) & 0xFFFF;
  MMRESULT res = ::midiOutSetVolume((HMIDIOUT)hMidiStream, (vol << 16) | vol);
  if (res != MMSYSERR_NOERROR) {
    Log::Error["audio-win"] << "Failed to set MIDI volume.";
  }
}

void
AudioWin::PlayerMIDI::volume_up() {
  float volume = get_volume() + 0.1f;
  if (volume > 1.f) {
    volume = 1.f;
  }
  set_volume(volume);
}

void
AudioWin::PlayerMIDI::volume_down() {
  float volume = get_volume() - 0.1f;
  if (volume < 0.f) {
    volume = 0.f;
  }
  set_volume(volume);
}

void
AudioWin::PlayerMIDI::midi_track_finished() {
  if (is_enabled()) {
    play_track(current_track->get_id() + 1);
  }
}

AudioWin::TrackMIDI::TrackMIDI(TypeMidi trackid, HMIDISTRM hMidiStrm)
  : hMidiStream(hMidiStrm)
  , track_id(trackid)
  , current_buffer(nullptr)
  , division(0) {
}

AudioWin::TrackMIDI::~TrackMIDI() {
  while (buffers.size()) {
    BufferMIDI *buffer = buffers.front();
    buffers.pop_front();
    delete buffer;
  }
}

void
AudioWin::TrackMIDI::play() {
  if (hMidiStream == NULL) {
    return;
  }

  MIDIPROPTIMEDIV div;
  div.cbStruct = sizeof(MIDIPROPTIMEDIV);
  div.dwTimeDiv = this->division;
  MMRESULT res = ::midiStreamProperty(hMidiStream, (LPBYTE)&div,
                                      MIDIPROP_SET | MIDIPROP_TIMEDIV);
  if (res != MMSYSERR_NOERROR) {
    Log::Error["audio-win"] << "Failed to set midi property.";
    return;
  }

  if (current_buffer == NULL) {
    return;
  }

  res = ::midiStreamRestart(hMidiStream);
  if (res != MMSYSERR_NOERROR) {
    Log::Error["audio-win"] << "Failed to restart midi stream.";
    return;
  }

  current_buffer->play(hMidiStream);
}

bool
AudioWin::TrackMIDI::play_next_buffer() {
  if (hMidiStream == NULL) {
    return false;
  }

  if (current_buffer == NULL) {
    if (buffers.empty()) {
      return false;
    }
    current_buffer = buffers.front();
  } else {
    std::list<BufferMIDI*>::iterator it = std::find(buffers.begin(),
                                                    buffers.end(),
                                                    current_buffer);
    if (it == buffers.end()) {
      return false;
    }
    ++it;
    if (it == buffers.end()) {
      return false;
    }
    current_buffer = *it;
  }

  return current_buffer->play(hMidiStream);
}

static size_t
read_varible_value(unsigned char* bufer, size_t *value) {
  size_t length = 0;
  unsigned char byte = 0;
  *value = 0;

  do {
    byte = *(bufer++);
    length++;
    *value = (*value << 7) + (byte & 0x7f);
  } while (byte & 0x80);

  return length;
}

void
AudioWin::TrackMIDI::push_event(DWORD time, DWORD event) {
  BufferMIDI *buffer = buffers.back();
  if (!buffer->push_event(time, event)) {
    buffer = new BufferMIDI();
    buffers.push_back(buffer);
    buffer->push_event(time, event);
  }
}

bool
AudioWin::TrackMIDI::create(PBuffer data_) {
  void *data = data_->get_data();
  buffers.push_back(new BufferMIDI());

  WORD div = *(reinterpret_cast<WORD*>(data) + 6);
  division = (DWORD)(betoh<uint16_t>(div));

  DWORD input_buf_size = *reinterpret_cast<DWORD*>(((unsigned char*)data) + 18);
  input_buf_size = betoh<uint32_t>(input_buf_size);
  unsigned char *input_buf = ((unsigned char*)data) + 22;
  unsigned char *input_buf_end = input_buf + input_buf_size;

  while (input_buf < input_buf_end) {
    size_t event_time = 0;
    input_buf += read_varible_value(input_buf, &event_time);
    size_t delta_time = event_time;
    unsigned char type = *(input_buf++);
    if (!(type & 0x80)) {
      // ToDo: implement if needed
    } else if (type == 0xff) {
      unsigned char meta = *(input_buf++);
      unsigned char length = *(input_buf++);
      if (meta == 0x51) {
        if (length == 3) {
          DWORD dwEvent = 0;
          dwEvent |= ((DWORD)MEVT_TEMPO) << 24;
          dwEvent |= ((DWORD)(*(input_buf++))) << 16;
          dwEvent |= ((DWORD)(*(input_buf++))) << 8;
          dwEvent |= ((DWORD)(*(input_buf++))) << 0;
          push_event((DWORD)delta_time, dwEvent);
        } else {
          input_buf += length;
          Log::Warn["audio-win"] << "Wrong size (" << length
                                 << ") of TEMPO meta event. Skipped.";
        }
      } else if (meta == 0x2F) {
        break;
      } else if (meta == 0x58) {
        // Drop this event
        input_buf += (length);
      } else {
        input_buf += length;
        Log::Warn["audio-win"] << "Unsupported midi meta event 0x" << std::hex
                               << static_cast<int>(meta)
                               << ". Skipped.";
      }
    } else if ((type & 0xf0) != 0xf0) {
      unsigned char event_data = *(input_buf++);
      DWORD dwEvent = 0;
      dwEvent |= ((DWORD)MEVT_SHORTMSG) << 24;
      dwEvent |= ((DWORD)type) << 0;
      dwEvent |= ((DWORD)event_data) << 8;

      if (!((type & 0xf0) == 0xc0 || (type & 0xf0) == 0xd0)) {
        event_data = *(input_buf++);
        dwEvent |= ((DWORD)event_data) << 16;
      }

      push_event((DWORD)delta_time, dwEvent);
    } else {
      Log::Error["audio-win"] << "Unsupported midi event 0x"
                              << std::hex << static_cast<int>(type);
    }
  }

  if (buffers.size() == 0) {
    return false;
  }

  current_buffer = buffers.front();

  return true;
}

AudioWin::BufferMIDI::BufferMIDI() {
  stream = NULL;
  stream_data = NULL;
  stream_size = 0;
  stream_buf_size = 0;
  header = NULL;
}

AudioWin::BufferMIDI::~BufferMIDI() {
  if (stream_data != NULL) {
    free(stream_data);
    stream_size = 0;
    stream_buf_size = 0;
  }

  if (header != NULL) {
    if ((stream != NULL) && (header->dwFlags & MHDR_PREPARED)) {
      MMRESULT res = ::midiOutUnprepareHeader((HMIDIOUT)stream,
                                              header, sizeof(MIDIHDR));
      if (res != MMSYSERR_NOERROR) {
        Log::Error["audio-win"] << "Failed to unprepare midi header.";
      }
      stream = NULL;
    }
    free(header);
    header = NULL;
  }
}

bool
AudioWin::BufferMIDI::push_event(DWORD time, DWORD event) {
  size_t event_size = 12;
  if (stream_size + event_size > 1000) {
    return false;
  }

  if (stream_buf_size - stream_size < event_size) {
    stream_buf_size += event_size * 1000;
    void *new_stream_data = malloc(stream_buf_size);
    if (new_stream_data == nullptr) {
      throw(std::exception());
    }
    memset(new_stream_data, 0x00, stream_buf_size);
    if (stream_data != nullptr) {
      memcpy(new_stream_data, stream_data, stream_size);
      free(stream_data);
    }
    stream_data = new_stream_data;
  }

  DWORD *new_event =
    reinterpret_cast<DWORD*>(reinterpret_cast<unsigned char*>(stream_data) +
                             stream_size);
  *(new_event++) = time;
  *(new_event++) = 0;
  *(new_event++) = event;
  stream_size += event_size;

  return true;
}

bool
AudioWin::BufferMIDI::prepare(HMIDISTRM strm) {
  stream = strm;

  size_t header_size = sizeof(MIDIHDR);
  header = reinterpret_cast<MIDIHDR*>(malloc(header_size));
  if (header == nullptr) {
    throw(std::exception());
  }
  memset(header, 0, header_size);
  header->lpData = (LPSTR)stream_data;
  header->dwBufferLength = (DWORD)stream_size;
  header->dwBytesRecorded = (DWORD)stream_size;
  header->dwFlags = 0;
  MMRESULT res = ::midiOutPrepareHeader((HMIDIOUT)stream,
                                        header, (UINT)header_size);
  if (res != MMSYSERR_NOERROR) {
    Log::Error["audio-win"] << "Failed to prepare midi header.";
    free(header);
    header = NULL;
    stream = NULL;
    return false;
  }

  return true;
}

bool
AudioWin::BufferMIDI::play(HMIDISTRM strm) {
  if ((header == NULL) || ((header->dwFlags & MHDR_PREPARED) == 0)) {
    if (!prepare(strm)) {
      return false;
    }
  }

  if (stream == NULL) {
    return false;
  }

  MMRESULT res = ::midiStreamOut(stream, header, sizeof(MIDIHDR));
  if (res != MMSYSERR_NOERROR) {
    Log::Error["audio-win"] << "Failed to prepare midi header.";
    return false;
  }

  return true;
}
