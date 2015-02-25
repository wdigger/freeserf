/*
 * audio-sdlmixer.h - Music and sound effects playback with SDL Mixer.
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


#ifndef _AUDIO_SDLMIXER_H
#define _AUDIO_SDLMIXER_H

#include "audio.h"

#include "SDL_Mixer.h"

class audio_sdlmixer_t;

class audio_wav_sdl_t
  : public audio_track_t
{
protected:
  audio_sdlmixer_t *audio;
  Mix_Chunk *sound;

public:
  audio_wav_sdl_t(audio_sdlmixer_t *audio);

  virtual bool play();
  bool create(void *data, size_t size);
};

class audio_midi_sdl_t
  : public audio_track_t
{
protected:
  audio_sdlmixer_t *audio;
  Mix_Music *music;

public:
  audio_midi_sdl_t(audio_sdlmixer_t *audio);

  virtual bool play();
  bool create(void *data, size_t size);
};

/* Common audio. */

class audio_sdlmixer_t
  : public audio_t
{
private:
  static audio_sdlmixer_t *audio_sdlmixer;

public:
  audio_sdlmixer_t();
  virtual ~audio_sdlmixer_t();

  virtual int get_volume();
  virtual void set_volume(int volume);

protected:
  virtual audio_track_t *create_sound_track(void *data, size_t size);
  virtual audio_track_t *create_music_track(void *data, size_t size);

  static void midi_track_finished();
};

#endif /* ! _AUDIO_SDLMIXER_H */
