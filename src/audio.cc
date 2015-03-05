/*
 * audio.cc - Base for music and sound effects playback.
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

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
#ifndef _MSC_VER
}
#endif

#include "stdlib.h"

audio_t::audio_t()
{
  sfx_enabled = true;
  midi_enabled = true;
  current_track = MIDI_TRACK_NONE;
}

audio_t::~audio_t()
{
  for (std::map<int, audio_track_t*>::iterator it = sfx_clips.begin(); it != sfx_clips.end(); ++it) {
    delete it->second;
  }
  sfx_clips.clear();

  for (std::map<int, audio_track_t*>::iterator it = midi_tracks.begin(); it != midi_tracks.end(); ++it) {
    delete it->second;
  }
  midi_tracks.clear();
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

/* Play sound. */

void
audio_t::sfx_play_clip(sfx_t sfx)
{
  if (!sfx_enabled) {
    return;
  }

  audio_track_t *audio_clip = NULL;
  std::map<int, audio_track_t*>::iterator it = sfx_clips.find(sfx);
  if (it != sfx_clips.end()) {
    audio_clip = it->second;
  }

  if (audio_clip == NULL) {
    size_t size = 0;
    void *wav = (char*)data_get_sound(sfx, &size);
    if (wav == NULL) {
      return;
    }

    audio_clip = create_sound_track(wav, size);
    free(wav);
    if (audio_clip == NULL) {
      LOGE("audio", "Failed to create sound track from WAV.");
      return;
    }

    sfx_clips[sfx] = audio_clip;
  }

  if (!audio_clip->play()) {
    LOGE("audio", "Could not play SFX clip #%d.", sfx);
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

  audio_track_t *track = NULL;
  std::map<int, audio_track_t*>::iterator it = midi_tracks.find(midi);
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

    track = create_music_track(data, size);
    free(data);
    if (track == NULL) {
      return;
    }

    midi_tracks[midi] = track;
  }

  if (!track->play()) {
    LOGW("audio", "Could not play MIDI track: #%d\n", midi);
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
//    Mix_HaltMusic();
  }
}

bool
audio_t::midi_is_enabled()
{
  return midi_enabled;
}
