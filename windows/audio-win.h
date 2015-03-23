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


#ifndef _AUDIO_WIN_H
#define _AUDIO_WIN_H

#include "audio.h"

#include <windows.h>
#include <list>

class audio_win_t;

class audio_wav_win_t
  : public audio_track_t
{
protected:
  audio_win_t *audio;
  void *data;
  size_t size;
  WAVEHDR *header;

public:
  audio_wav_win_t(audio_win_t *audio);
  virtual ~audio_wav_win_t();

  virtual bool play();
  bool create(void *data, size_t size);
};

class midi_buffer_t
{
protected:
  HMIDISTRM stream;
  void *stream_data;
  size_t stream_size;
  size_t stream_buf_size;
  MIDIHDR *header;

public:
  midi_buffer_t();
  virtual ~midi_buffer_t();

  bool push_event(DWORD time, DWORD event);
  bool play(HMIDISTRM stream);

protected:
  bool prepare(HMIDISTRM stream);
};

class audio_midi_win_t
  : public audio_track_t
{
protected:
  audio_win_t *audio;
  DWORD division;
  std::list<midi_buffer_t*> buffers;
  midi_buffer_t *current_buffer;

public:
  audio_midi_win_t(audio_win_t *audio);
  virtual ~audio_midi_win_t();

  virtual bool play();
  bool create(void *data, size_t size);

  bool play_next_buffer();

protected:
  void push_event(DWORD time, DWORD event);
};

/* Common audio. */

class audio_win_t
  : public audio_t
{
protected:
  HWAVEOUT hWaveOut;
  HMIDISTRM hMidiStream;

public:
  audio_win_t();
  virtual ~audio_win_t();

  virtual int get_volume();
  virtual void set_volume(int volume);

  HWAVEOUT get_waveout() { return hWaveOut; }
  HMIDISTRM get_midistream() { return hMidiStream; }

protected:
  virtual audio_track_t *create_sound_track(void *data, size_t size);
  virtual audio_track_t *create_music_track(void *data, size_t size);

  static void midi_track_finished();

private:
  static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  void waveCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  static void CALLBACK midiOutProc(HMIDIOUT hmo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  void midiCallback(HMIDIOUT hwo, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
};

#endif /* ! _AUDIO_WIN_H */
