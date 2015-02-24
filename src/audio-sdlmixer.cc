/*
 * audio-sdlmixer.cc - Music and sound effects playback using SDL_mixer.
 *
 * Copyright (C) 2012-2015  Wicked_Digger <wicked_digger@mail.ru>
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

#include "audio-sdlmixer.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
#ifndef _MSC_VER
}
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#include "SDL.h"
#include "SDL_mixer.h"

audio_sdlmixer_t::audio_sdlmixer_t()
{
  SDL_InitSubSystem(SDL_INIT_AUDIO);

  LOGI("audio-sdlmixer", "Initializing audio driver `sdlmixer'.");

  int r = Mix_Init(0);
  if (r != 0) {
    LOGE("audio-sdlmixer", "Could not init SDL_mixer: %s.", Mix_GetError());
    return;
  }

  r = Mix_OpenAudio(8000, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 512);
  if (r < 0) {
    LOGE("audio-sdlmixer", "Could not open audio device: %s.", Mix_GetError());
    return;
  }

  r = Mix_AllocateChannels(16);
  if (r != 16) {
    LOGE("audio-sdlmixer", "Failed to allocate channels: %s.", Mix_GetError());
    return;
  }

  Mix_HookMusicFinished(midi_track_finished);
}

audio_sdlmixer_t::~audio_sdlmixer_t()
{
  Mix_HookMusicFinished(NULL);
  Mix_CloseAudio();
  Mix_Quit();
}

int
audio_sdlmixer_t::get_volume()
{
  return (int)((float)Mix_Volume(-1, -1) / (float)MIX_MAX_VOLUME * 99.f + 0.5f);
}

void
audio_sdlmixer_t::set_volume(int volume)
{
  /* TODO Increment by one only works for MIX_MAX_VALUE > 50*/
  volume = (int)((float)volume / 99.f * (float)MIX_MAX_VOLUME + 0.5f);
  Mix_Volume(-1, volume);
  Mix_VolumeMusic(volume);
}

audio_track_t *
audio_sdlmixer_t::create_sound_track(void *data, size_t size)
{
  audio_wav_sdl_t *track = new audio_wav_sdl_t(this);
  if (!track->create(data, size)) {
    delete track;
    track = NULL;
  }

  return track;
}

audio_track_t *
audio_sdlmixer_t::create_music_track(void *data, size_t size)
{
  audio_midi_sdl_t *track = new audio_midi_sdl_t(this);
  if (!track->create(data, size)) {
    delete track;
    track = NULL;
  }

  return track;
}

void
audio_sdlmixer_t::midi_track_finished()
{
  audio_sdlmixer_t *audio = (audio_sdlmixer_t*)audio_t::get_audio();

  if (audio->midi_enabled) {
    audio->midi_play_track(audio->current_track);
  }
}

audio_wav_sdl_t::audio_wav_sdl_t(audio_sdlmixer_t *audio)
{
  this->audio = audio;
  sound = NULL;
}

bool
audio_wav_sdl_t::play()
{
  if (sound == NULL) {
    return false;
  }

  int r = Mix_PlayChannel(-1, sound, 0);
  return (r >= 0);
}

bool
audio_wav_sdl_t::create(void *data, size_t size)
{
  SDL_RWops *rw = SDL_RWFromMem(data, (int)size);
  sound = Mix_LoadWAV_RW(rw, 0);
  return (sound != NULL);
}

audio_midi_sdl_t::audio_midi_sdl_t(audio_sdlmixer_t *audio)
{
  this->audio = audio;
  music = NULL;
}

bool
audio_midi_sdl_t::play()
{
  if (music == NULL) {
    return false;
  }

  int r = Mix_PlayMusic(music, 0);
  return (r >= 0);
}

bool
audio_midi_sdl_t::create(void *data, size_t size)
{
  SDL_RWops *rw = SDL_RWFromMem(data, (int)size);
  music = Mix_LoadMUS_RW(rw, 0);
  return (music != NULL);
}
