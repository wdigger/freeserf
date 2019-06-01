/*
 * audio-win.h - Music and sound effects playback.
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


#ifndef WINDOWS_AUDIO_WIN_H_
#define WINDOWS_AUDIO_WIN_H_

#include <windows.h>
#include <list>
#include <memory>

#include "src/audio.h"
#include "src/buffer.h"

/* Common audio. */

class AudioWin : public Audio, public Audio::VolumeController {
 public:
  class TrackSFX : public Audio::Track {
   protected:
    PBuffer data;
    HWAVEOUT hWaveOut;
    WAVEHDR *wave_header;

   public:
    explicit TrackSFX(PBuffer data, HWAVEOUT hWaveOut);
    virtual ~TrackSFX();

    virtual void play();
  };

  class PlayerSFX : public Audio::Player, public Audio::VolumeController,
                    public std::enable_shared_from_this<PlayerSFX> {
   protected:
    HWAVEOUT hWaveOut;

   public:
    PlayerSFX();
    virtual ~PlayerSFX();

    virtual void enable(bool enable);
    virtual Audio::PVolumeController get_volume_controller() {
        return shared_from_this();
    }

   protected:
    virtual Audio::PTrack create_track(int track_id);
    virtual void stop();

   public:
    virtual float get_volume();
    virtual void set_volume(float volume);
    virtual void volume_up();
    virtual void volume_down();

   protected:
    static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg,
                                     DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                     DWORD_PTR dwParam2);
    void waveCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwParam1,
                      DWORD_PTR dwParam2);
  };

  class BufferMIDI {
   protected:
    HMIDISTRM stream;
    void *stream_data;
    size_t stream_size;
    size_t stream_buf_size;
    MIDIHDR *header;

   public:
    BufferMIDI();
    virtual ~BufferMIDI();

    bool push_event(DWORD time, DWORD event);
    bool play(HMIDISTRM stream);

   protected:
    bool prepare(HMIDISTRM stream);
  };

  class TrackMIDI : public Audio::Track {
   protected:
    HMIDISTRM hMidiStream;
    DWORD division;
    std::list<BufferMIDI*> buffers;
    BufferMIDI *current_buffer;
    TypeMidi track_id;

   public:
    TrackMIDI(TypeMidi track_id, HMIDISTRM hMidiStream);
    virtual ~TrackMIDI();

    virtual void play();

    TypeMidi get_id() const { return track_id; }
    bool create(PBuffer data);
    bool play_next_buffer();

   protected:
    void push_event(DWORD time, DWORD event);
  };
  typedef std::shared_ptr<TrackMIDI> PTrackMIDI;

  class PlayerMIDI : public Audio::Player, public Audio::VolumeController,
                     public std::enable_shared_from_this<PlayerMIDI> {
   protected:
    PTrackMIDI current_track;
    HMIDISTRM hMidiStream;

   public:
    PlayerMIDI();
    virtual ~PlayerMIDI();

    virtual Audio::PTrack play_track(int track_id);
    virtual void enable(bool enable);
    virtual Audio::PVolumeController get_volume_controller() {
        return shared_from_this();
    }

   protected:
    virtual Audio::PTrack create_track(int track_id);
    virtual void stop();

   public:
    virtual float get_volume();
    virtual void set_volume(float volume);
    virtual void volume_up();
    virtual void volume_down();

   protected:
    static void CALLBACK midiOutProc(HMIDIOUT hmo, UINT uMsg,
                                     DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                     DWORD_PTR dwParam2);
    void midiCallback(HMIDIOUT hwo, UINT uMsg, DWORD_PTR dwParam1,
                      DWORD_PTR dwParam2);
    void midi_track_started(TrackMIDI *track);
    void midi_track_finished();
  };

 protected:
  Audio::PPlayer sfx_player;
  Audio::PPlayer midi_player;

 public:
  AudioWin();
  virtual ~AudioWin();

  virtual Audio::VolumeController *get_volume_controller() { return this; }
  virtual Audio::PPlayer get_sound_player() { return sfx_player; }
  virtual Audio::PPlayer get_music_player() { return midi_player; }

 public:
  virtual float get_volume() { return volume; }
  virtual void set_volume(float volume);
  virtual void volume_up();
  virtual void volume_down();
};

#endif  // WINDOWS_AUDIO_WIN_H_
