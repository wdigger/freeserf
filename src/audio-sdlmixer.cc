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

#include "audio.h"
#include "data.h"
#include "freeserf_endian.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "pqueue.h"
  #include "list.h"
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

/* Play sound. */

typedef struct {
  list_elm_t elm;
  int num;
  Mix_Chunk *chunk;
} audio_clip_t;

typedef struct {
  list_elm_t elm;
  int num;
  Mix_Music *music;
} track_t;

audio_t *audio_t::audio = NULL;

audio_t *
audio_t::get_audio()
{
  if (audio != NULL) {
    audio = new audio_t();
  }

  return audio;
}

audio_t::audio_t()
{
  sfx_enabled = true;
  midi_enabled = true;
  current_track = MIDI_TRACK_NONE;

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

audio_t::~audio_t()
{
  for (std::map<int, Mix_Chunk*>::iterator it = sfx_clips_to_play.begin(); it != sfx_clips_to_play.end(); it++) {
    Mix_FreeChunk(it->second);
  }
  sfx_clips_to_play.clear();

  for (std::map<int, Mix_Music*>::iterator it = midi_tracks.begin(); it != midi_tracks.end(); it++) {
    Mix_FreeMusic(it->second);
  }
  midi_tracks.clear();

  Mix_CloseAudio();
  Mix_Quit();
}

int
audio_t::get_volume()
{
  return (int)((float)Mix_Volume(-1, -1) / (float)MIX_MAX_VOLUME * 99.f + 0.5f);
}

void
audio_t::set_volume(int volume)
{
  /* TODO Increment by one only works for MIX_MAX_VALUE > 50*/
  volume = (int)((float)volume / 99.f * (float)MIX_MAX_VOLUME + 0.5f);
  Mix_Volume(-1, volume);
  Mix_VolumeMusic(volume);
}

void
audio_t::volume_up()
{
  int volume = get_volume();
  set_volume(volume + 1);
}

void
audio_t::volume_down()
{
  int volume = get_volume();
  set_volume(volume - 1);
}

void
audio_t::sfx_play_clip(sfx_t sfx)
{
  if (!sfx_enabled) {
    return;
  }

  Mix_Chunk *audio_clip = NULL;
  std::map<int, Mix_Chunk*>::iterator it = sfx_clips_to_play.find(sfx);
  if (it != sfx_clips_to_play.end()) {
    audio_clip = it->second;
  }

  if (audio_clip == NULL) {
    size_t size = 0;
    void *wav = (char*)data_get_sound(sfx, &size);
    if (wav == NULL) {
      return;
    }

    SDL_RWops *rw = SDL_RWFromMem(wav, (int)size);
    audio_clip = Mix_LoadWAV_RW(rw, 0);
    free(wav);
    if (audio_clip == NULL) {
      LOGE("audio-sdlmixer", "Mix_LoadWAV_RW: %s.", Mix_GetError());
      return;
    }

    sfx_clips_to_play[sfx] = audio_clip;
  }

  int r = Mix_PlayChannel(-1, audio_clip, 0);
  if (r < 0) {
    LOGE("audio-sdlmixer", "Could not play SFX clip: %s.", Mix_GetError());
    return;
  }
}

void
audio_t::sfx_enable(bool enable)
{
  sfx_enabled = enable;
}

bool
audio_t::sfx_is_enabled()
{
  return sfx_enabled;
}

/* Play music. */

void
audio_t::midi_play_track(midi_t midi)
{
  if (0 == midi_enabled) {
    return;
  }

  Mix_Music *track = NULL;
  std::map<int, Mix_Music*>::iterator it = midi_tracks.find(midi);
  if (it != midi_tracks.end()) {
    track = it->second;
  }

  current_track = midi;

  if (track == NULL) {
    size_t size = 0;
    char *data = (char*)data_get_music(midi, &size);
    if (data == NULL) {
      return;
    }

    SDL_RWops *rw = SDL_RWFromMem(data, (int)size);
    track = Mix_LoadMUS_RW(rw, 0);
    free(data);
    if (track == NULL) {
      return;
    }

    midi_tracks[midi] = track;
  }

  int r = Mix_PlayMusic(track, 0);
  if (r < 0) {
    LOGW("audio-sdlmixer", "Could not play MIDI track: %s\n", Mix_GetError());
    return;
  }

  return;
}

void
audio_t::midi_enable(bool enable)
{
  midi_enabled = enable;
  if (0 != enable) {
    if (current_track != -1) {
      midi_play_track(current_track);
    }
  } else {
    Mix_HaltMusic();
  }
}

bool
audio_t::midi_is_enabled()
{
  return midi_enabled;
}

void
audio_t::midi_track_finished()
{
  audio_t *audio = audio_t::get_audio();

  if (audio->midi_enabled) {
    audio->midi_play_track(audio->current_track);
  }
}
