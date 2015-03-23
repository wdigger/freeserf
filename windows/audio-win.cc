/*
 * audio-win.h - Music and sound effects playback.
 *
 * Copyright (C) 2015  Wicked_Digger <wicked_digger@mail.ru>
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

#include "audio-win.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
  #include "freeserf_endian.h"
#ifndef _MSC_VER
}
#endif

#include <string.h>
#include <algorithm>

audio_t *
create_audio()
{
  return new audio_win_t();
}

audio_win_t::audio_win_t()
{
  WAVEFORMATEX format = {0};
  format.wFormatTag = WAVE_FORMAT_PCM;
  format.nChannels = 1;
  format.nSamplesPerSec = 8000;
  format.nAvgBytesPerSec = 16000;
  format.nBlockAlign = 2;
  format.wBitsPerSample = 16;
  format.cbSize = 0;
  MMRESULT res = ::waveOutOpen(&hWaveOut, WAVE_MAPPER, (WAVEFORMATEX*)&format, (DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
  if (res != MMSYSERR_NOERROR) {
    LOGE("audio-win", "Failed to open wave out.");
  }

  UINT uDeviceID = 0;
  res = ::midiStreamOpen(&hMidiStream, &uDeviceID, 1, (DWORD_PTR)midiOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
  if (res != MMSYSERR_NOERROR) {
    LOGE("audio-win", "Failed to open midi stream.");
  }
}

audio_win_t::~audio_win_t()
{
  ::waveOutClose(hWaveOut);
}

int
audio_win_t::get_volume()
{
  if (hWaveOut == NULL) {
    return 0;
  }

  DWORD dwVolume = 0;
  MMRESULT res = ::waveOutGetVolume(hWaveOut, &dwVolume);
  if (res != MMSYSERR_NOERROR) {
    LOGE("audio-win", "Failed to get volume.");
  }

  return (int)((float)dwVolume / (float)0xFFFF * 100.f);
}

void
audio_win_t::set_volume(int volume)
{
  if (hWaveOut == NULL) {
    return;
  }

  DWORD dwVolume = (DWORD)((float)volume / 100.f * (float)0xFFFF);
  MMRESULT res = ::waveOutSetVolume(hWaveOut, dwVolume);
  if (res != MMSYSERR_NOERROR) {
    LOGE("audio-win", "Failed to set wave volume.");
  }
  res = ::midiOutSetVolume((HMIDIOUT)hMidiStream, dwVolume);
  if (res != MMSYSERR_NOERROR) {
    LOGE("audio-win", "Failed to set midi volume.");
  }
}

audio_track_t *
audio_win_t::create_sound_track(void *data, size_t size)
{
  audio_wav_win_t *track = new audio_wav_win_t(this);
  if (!track->create(data, size)) {
    delete track;
    track = NULL;
  }

  return track;
}

audio_track_t *
audio_win_t::create_music_track(void *data, size_t size)
{
  audio_midi_win_t *track = new audio_midi_win_t(this);
  if (!track->create(data, size)) {
    delete track;
    track = NULL;
  }

  return track;
}

void CALLBACK
audio_win_t::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
  audio_win_t *_this = (audio_win_t*)dwInstance;
  if (_this != NULL) {
    _this->waveCallback(hwo, uMsg, dwParam1, dwParam2);
  }
}

void
audio_win_t::waveCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
  if (uMsg == WOM_DONE) {
  }
}

void CALLBACK
audio_win_t::midiOutProc(HMIDIOUT hmo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
  audio_win_t *_this = (audio_win_t*)dwInstance;
  if (_this != NULL) {
    _this->midiCallback(hmo, uMsg, dwParam1, dwParam2);
  }
}

void
audio_win_t::midiCallback(HMIDIOUT hwo, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
  switch (uMsg) {
    case MOM_DONE: {
      MMRESULT res = ::midiStreamRestart(hMidiStream);
      if (res != MMSYSERR_NOERROR) {
        LOGE("audio-win", "Failed to restart midi stream.");
        return;
      }
      if (current_midi_track != NULL) {
        if (!((audio_midi_win_t*)current_midi_track)->play_next_buffer()) {
          on_midi_track_finished();
        }
      }
      break;
    }
  }
}

audio_wav_win_t::audio_wav_win_t(audio_win_t *audio)
{
  this->audio = audio;
  data = NULL;
  size = 0;
  header = NULL;
}

audio_wav_win_t::~audio_wav_win_t()
{
  if (data != NULL) {
    free(data);
    data = NULL;
    size = 0;
  }

  if (header != NULL) {
    free(header);
    header = NULL;
  }
}

bool
audio_wav_win_t::play()
{
  HWAVEOUT hWaveOut = audio->get_waveout();
  if (hWaveOut == NULL) {
    return false;
  }

  size_t size = sizeof(WAVEHDR);
  header = (WAVEHDR*)malloc(size);
  memset(header, 0, size);
  header->lpData = (LPSTR)data;
  header->dwBufferLength = (DWORD)this->size;
  header->dwFlags = 0;
  MMRESULT res = ::waveOutPrepareHeader(audio->get_waveout(), header, sizeof(WAVEHDR));
  if (res != MMSYSERR_NOERROR) {
    LOGE("audio-win", "Failed to prepare header.");
    return false;
  }
  res = ::waveOutWrite(audio->get_waveout(), header, (UINT)size);
  return (res == MMSYSERR_NOERROR);
}

bool
audio_wav_win_t::create(void *data, size_t size)
{
  this->size = size - 40;
  this->data = malloc(size);
  memcpy(this->data, ((unsigned char*)data) + 40, this->size);
  return true;
}

audio_midi_win_t::audio_midi_win_t(audio_win_t *audio)
{
  this->audio = audio;
  current_buffer = NULL;
}

audio_midi_win_t::~audio_midi_win_t()
{
  while (buffers.size()) {
    midi_buffer_t *buffer = buffers.front();
    buffers.pop_front();
    delete buffer;
  }
}

bool
audio_midi_win_t::play()
{
  HMIDISTRM stream = audio->get_midistream();
  if (stream == NULL) {
    return false;
  }

  MIDIPROPTIMEDIV division;
  division.cbStruct = sizeof(MIDIPROPTIMEDIV);
  division.dwTimeDiv = this->division;
  MMRESULT res = ::midiStreamProperty(stream, (LPBYTE)&division, MIDIPROP_SET | MIDIPROP_TIMEDIV);
  if (res != MMSYSERR_NOERROR) {
    LOGE("audio-win", "Failed to set midi property.");
    return false;
  }

  if (current_buffer == NULL) {
    return false;
  }

  res = ::midiStreamRestart(stream);
  if (res != MMSYSERR_NOERROR) {
    LOGE("audio-win", "Failed to restart midi stream.");
    return false;
  }

  return current_buffer->play(stream);
}

bool
audio_midi_win_t::play_next_buffer()
{
  HMIDISTRM stream = audio->get_midistream();
  if (stream == NULL) {
    return false;
  }

  if (current_buffer == NULL) {
    if (buffers.empty()) {
      return false;
    }
    current_buffer = buffers.front();
  }
  else {
    std::list<midi_buffer_t*>::iterator it = std::find(buffers.begin(), buffers.end(), current_buffer);
    if (it == buffers.end()) {
      return false;
    }
    it++;
    if (it == buffers.end()) {
      return false;
    }
    current_buffer = *it;
  }

  return current_buffer->play(stream);
}

static size_t
read_varible_value(unsigned char* bufer, unsigned long &value)
{
  size_t length = 0;
  unsigned char byte = 0;
  value = 0;

  do {
    byte = *(bufer++);
    length++;
    value = (value << 7) + (byte & 0x7f);
  } while (byte & 0x80);

  return length;
}

void
audio_midi_win_t::push_event(DWORD time, DWORD event)
{
  midi_buffer_t *buffer = buffers.back();
  if (!buffer->push_event(time, event)) {
    buffer = new midi_buffer_t();
    buffers.push_back(buffer);
    buffer->push_event(time, event);
  }
}

bool
audio_midi_win_t::create(void *data, size_t size)
{
  buffers.push_back(new midi_buffer_t());

  WORD division = *(((WORD*)data) + 6);
  this->division = (DWORD)(be16toh(division));

  DWORD input_buf_size = *(DWORD*)(((unsigned char*)data) + 18);
  input_buf_size = be32toh(input_buf_size);
  unsigned char *input_buf = ((unsigned char*)data) + 22;
  unsigned char *input_buf_end = input_buf + input_buf_size;

  while (input_buf < input_buf_end) {
    unsigned long event_time = 0;
    input_buf += read_varible_value(input_buf, event_time);
    unsigned long delta_time = event_time;
    unsigned char type = *(input_buf++);
    if (!(type & 0x80)) {
      // ToDo: implement if needed
    }
    else if(type == 0xff){
      unsigned char meta = *(input_buf++);
      unsigned char length = *(input_buf++);
      if (meta == 0x51) {
        if (length == 3) {
          DWORD dwEvent = 0;
          dwEvent |= ((DWORD)MEVT_TEMPO) << 24;
          dwEvent |= ((DWORD)(*(input_buf++))) << 16;
          dwEvent |= ((DWORD)(*(input_buf++))) << 8;
          dwEvent |= ((DWORD)(*(input_buf++))) << 0;
          push_event(delta_time, dwEvent);
        }
        else {
          input_buf += length;
          LOGW("audio-win", "Wrong size (%d) of TEMPO meta event. Skipped.", length);
        }
      }
      else if (meta == 0x2F) {
        break;
      }
      else {
        input_buf += length;
        LOGW("audio-win", "Unsupported midi meta event 0x%02x. Skipped.", meta);
      }
    }
    else if((type & 0xf0) != 0xf0) {
      unsigned char event_data = *(input_buf++);
      DWORD dwEvent = 0;
      dwEvent |= ((DWORD)MEVT_SHORTMSG) << 24;
      dwEvent |= ((DWORD)type) << 0;
      dwEvent |= ((DWORD)event_data) << 8;

      if(!((type & 0xf0) == 0xc0 || (type & 0xf0) == 0xd0)) {
        event_data = *(input_buf++);
        dwEvent |= ((DWORD)event_data) << 16;
      }

      push_event(delta_time, dwEvent);
    }
    else {
      LOGE("audio-win", "Unsupported midi event 0x%02x.", type);
    }
  }

  if (buffers.size() == 0) {
    return false;
  }

  current_buffer = buffers.front();

  return true;
}

midi_buffer_t::midi_buffer_t()
{
  stream = NULL;
  stream_data = NULL;
  stream_size = 0;
  stream_buf_size = 0;
  header = NULL;
}

midi_buffer_t::~midi_buffer_t()
{
  if (stream_data != NULL) {
    free(stream_data);
    stream_size = 0;
    stream_buf_size = 0;
  }

  if (header != NULL) {
    if ((stream != NULL) && (header->dwFlags & MHDR_PREPARED)) {
      MMRESULT res = ::midiOutUnprepareHeader((HMIDIOUT)stream, header, sizeof(MIDIHDR));
      if (res != MMSYSERR_NOERROR) {
        LOGE("audio-win", "Failed to unprepare midi header.");
      }
      stream = NULL;
    }
    free(header);
    header = NULL;
  }
}

bool
midi_buffer_t::push_event(DWORD time, DWORD event)
{
  size_t event_size = 12;
  if (stream_size + event_size > 1000) {
    return false;
  }

  if (stream_buf_size - stream_size < event_size) {
    stream_buf_size += event_size * 1000;
    void *new_stream_data = malloc(stream_buf_size);
    memset(new_stream_data, 0x00, stream_buf_size);
    if (stream_data != NULL) {
      memcpy(new_stream_data, stream_data, stream_size);
      free(stream_data);
    }
    stream_data = new_stream_data;
  }

  DWORD *new_event = (DWORD*)(((unsigned char*)stream_data) + stream_size);
  *(new_event++) = time;
  *(new_event++) = 0;
  *(new_event++) = event;
  stream_size += event_size;

  return true;
}

bool
midi_buffer_t::prepare(HMIDISTRM stream)
{
  this->stream = stream;

  size_t header_size = sizeof(MIDIHDR);
  header = (MIDIHDR*)malloc(header_size);
  memset(header, 0, header_size);
  header->lpData = (LPSTR)stream_data;
  header->dwBufferLength = (DWORD)stream_size;
  header->dwBytesRecorded = (DWORD)stream_size;
  header->dwFlags = 0;
  MMRESULT res = ::midiOutPrepareHeader((HMIDIOUT)stream, header, (UINT)header_size);
  if (res != MMSYSERR_NOERROR) {
    LOGE("audio-win", "Failed to prepare midi header.");
    free(header);
    header = NULL;
    stream = NULL;
    return false;
  }

  return true;
}

bool
midi_buffer_t::play(HMIDISTRM stream)
{
  if ((header == NULL) || ((header->dwFlags & MHDR_PREPARED) == 0)) {
    if (!prepare(stream)) {
      return false;
    }
  }

  if (stream == NULL) {
    return false;
  }

  MMRESULT res = ::midiStreamOut(stream, header, sizeof(MIDIHDR));
  if (res != MMSYSERR_NOERROR) {
    LOGE("audio-win", "Failed to prepare midi header.");
    return false;
  }

  return (res == MMSYSERR_NOERROR);
}
