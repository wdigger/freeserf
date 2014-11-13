/*
 * audio-sdlmixer.c - Music and sound effects playback using SDL_mixer.
 *
 * Copyright (C) 2012-2014  Wicked_Digger <wicked_digger@mail.ru>
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

static list_t sfx_clips_to_play;
static list_t midi_tracks;
static int sfx_enabled = 1;
static int midi_enabled = 1;
static midi_t current_track = MIDI_TRACK_NONE;

static void midi_track_finished();


int
audio_init()
{
  SDL_InitSubSystem(SDL_INIT_AUDIO);

  LOGI("audio-sdlmixer", "Initializing audio driver `sdlmixer'.");

  list_init(&sfx_clips_to_play);
  list_init(&midi_tracks);

  int r = Mix_Init(0);
  if (r != 0) {
    LOGE("audio-sdlmixer", "Could not init SDL_mixer: %s.", Mix_GetError());
    return -1;
  }

  r = Mix_OpenAudio(8000, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 512);
  if (r < 0) {
    LOGE("audio-sdlmixer", "Could not open audio device: %s.", Mix_GetError());
    return -1;
  }

  r = Mix_AllocateChannels(16);
  if (r != 16) {
    LOGE("audio-sdlmixer", "Failed to allocate channels: %s.", Mix_GetError());
    return -1;
  }

  Mix_HookMusicFinished(midi_track_finished);

  return 0;
}

void
audio_deinit()
{
  while (!list_is_empty(&sfx_clips_to_play)) {
    list_elm_t *elm = list_remove_head(&sfx_clips_to_play);
    audio_clip_t *audio_clip = (audio_clip_t*)elm;
    Mix_FreeChunk(audio_clip->chunk);
    free(elm);
  }

  while (!list_is_empty(&midi_tracks)) {
    list_elm_t *elm = list_remove_head(&midi_tracks);
    track_t *track = (track_t*)elm;
    Mix_FreeMusic(track->music);
    free(elm);
  }

  Mix_CloseAudio();
  Mix_Quit();
}

int
audio_volume()
{
  return (int)((float)Mix_Volume(-1, -1) / (float)MIX_MAX_VOLUME * 99.f + 0.5f);
}

void
audio_set_volume(int volume)
{
  /* TODO Increment by one only works for MIX_MAX_VALUE > 50*/
  volume = (int)((float)volume / 99.f * (float)MIX_MAX_VOLUME + 0.5f);
  Mix_Volume(-1, volume);
  Mix_VolumeMusic(volume);
}

void
audio_volume_up()
{
  int volume = audio_volume();
  audio_set_volume(volume + 1);
}

void
audio_volume_down()
{
  int volume = audio_volume();
  audio_set_volume(volume - 1);
}


void
sfx_play_clip(sfx_t sfx)
{
  if (0 == sfx_enabled) {
    return;
  }

  audio_clip_t *audio_clip = NULL;
  list_elm_t *elm;
  list_foreach(&sfx_clips_to_play, elm) {
    if (sfx == ((audio_clip_t*)elm)->num) {
      audio_clip = (audio_clip_t*)elm;
    }
  }

  if (NULL == audio_clip) {
    audio_clip = (audio_clip_t*)malloc(sizeof(audio_clip_t));
    if (audio_clip == NULL) abort();
    audio_clip->num = sfx;

    size_t size = 0;
    void *wav = (char*)data_get_sound(sfx, &size);
    if (!wav) {
      free(audio_clip);
      return;
    }

    SDL_RWops *rw = SDL_RWFromMem(wav, (int)size);
    audio_clip->chunk = Mix_LoadWAV_RW(rw, 0);
    free(wav);
    if (!audio_clip->chunk) {
      LOGE("audio-sdlmixer", "Mix_LoadWAV_RW: %s.", Mix_GetError());
      free(audio_clip);
      return;
    }

    list_prepend(&sfx_clips_to_play, (list_elm_t*)audio_clip);
  }

  int r = Mix_PlayChannel(-1, audio_clip->chunk, 0);
  if (r < 0) {
    LOGE("audio-sdlmixer", "Could not play SFX clip: %s.", Mix_GetError());
    return;
  }
}

void
sfx_enable(int enable)
{
  sfx_enabled = enable;
}

int
sfx_is_enabled()
{
  return sfx_enabled;
}


/* Play music. */

void
midi_play_track(midi_t midi)
{
  if (0 == midi_enabled) {
    return;
  }

  track_t *track = NULL;
  list_elm_t *elm;
  list_foreach(&midi_tracks, elm) {
    if (midi == ((track_t*)elm)->num) {
      track = (track_t*)elm;
    }
  }

  current_track = midi;

  if (NULL == track) {
    track = (track_t *)malloc(sizeof(track_t));
    if (track == NULL) abort();

    track->num = midi;

    size_t size = 0;
    char *data = (char*)data_get_music(midi, &size);
    if (NULL == data) {
      free(track);
      return;
    }

    SDL_RWops *rw = SDL_RWFromMem(data, (int)size);
    track->music = Mix_LoadMUS_RW(rw, 0);
    free(data);
    if (NULL == track->music) {
      free(track);
      return;
    }

    list_append(&midi_tracks, (list_elm_t*)track);
  }

  int r = Mix_PlayMusic(track->music, 0);
  if (r < 0) {
    LOGW("audio-sdlmixer", "Could not play MIDI track: %s\n", Mix_GetError());
    return;
  }

  return;
}

void
midi_enable(int enable)
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

int
midi_is_enabled()
{
  return midi_enabled;
}

static void
midi_track_finished()
{
  if (midi_enabled) {
    midi_play_track(current_track);
  }
}
