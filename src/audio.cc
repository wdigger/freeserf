/*
 * audio.cc - Music and sound effects playback base.
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

#include "src/audio.h"

#include <cstring>
#include <cassert>
#include <algorithm>

#include "src/misc.h"
BEGIN_EXT_C
  #include "src/log.h"
END_EXT_C

audio_t *audio_t::instance = NULL;

/* Sound effect parameters of the original (Amiga sfx_table, 0x2f334):
   base period, period random mask, base volume (0..64), volume random
   mask, and the frames (1/50 s) the channel stays reserved. */
typedef struct {
  int period;
  int period_mask;
  int volume;
  int volume_mask;
  int duration;
} sfx_params_t;

static const sfx_params_t sfx_params[SFX_COUNT] = {
  {0, 0, 0, 0, 0}, {427, 0, 64, 0, 96}, {427, 0, 48, 0, 40},  /* 0 */
  {0, 0, 0, 0, 0}, {427, 0, 64, 0, 86}, {0, 0, 0, 0, 0},  /* 3 */
  {427, 0, 20, 0, 36}, {0, 0, 0, 0, 0}, {220, 0, 25, 0, 1},  /* 6 */
  {0, 0, 0, 0, 0}, {412, 31, 25, 15, 75}, {0, 0, 0, 0, 0},  /* 9 */
  {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {412, 31, 25, 15, 94},  /* 12 */
  {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},  /* 15 */
  {412, 31, 25, 15, 80}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},  /* 18 */
  {0, 0, 0, 0, 0}, {412, 31, 25, 15, 51}, {0, 0, 0, 0, 0},  /* 21 */
  {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {412, 31, 29, 7, 147},  /* 24 */
  {0, 0, 0, 0, 0}, {396, 63, 23, 15, 32}, {0, 0, 0, 0, 0},  /* 27 */
  {420, 15, 9, 7, 54}, {0, 0, 0, 0, 0}, {396, 63, 25, 15, 39},  /* 30 */
  {0, 0, 0, 0, 0}, {1774, 31, 25, 15, 266}, {0, 0, 0, 0, 0},  /* 33 */
  {412, 31, 9, 7, 39}, {0, 0, 0, 0, 0}, {412, 31, 17, 7, 27},  /* 36 */
  {0, 0, 0, 0, 0}, {396, 63, 25, 15, 17}, {0, 0, 0, 0, 0},  /* 39 */
  {879, 31, 9, 3, 145}, {729, 31, 9, 3, 145}, {412, 31, 7, 3, 32},  /* 42 */
  {0, 0, 0, 0, 0}, {205, 31, 25, 15, 8}, {0, 0, 0, 0, 0},  /* 45 */
  {412, 31, 25, 15, 21}, {0, 0, 0, 0, 0}, {945, 31, 25, 15, 152},  /* 48 */
  {0, 0, 0, 0, 0}, {492, 31, 17, 7, 49}, {0, 0, 0, 0, 0},  /* 51 */
  {248, 0, 32, 0, 6}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},  /* 54 */
  {0, 0, 0, 0, 0}, {412, 31, 25, 15, 24}, {0, 0, 0, 0, 0},  /* 57 */
  {362, 255, 12, 7, 48}, {0, 0, 0, 0, 0}, {1156, 63, 7, 3, 228},  /* 60 */
  {0, 0, 0, 0, 0}, {825, 63, 9, 7, 243}, {0, 0, 0, 0, 0},  /* 63 */
  {887, 15, 7, 3, 200}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},  /* 66 */
  {465, 63, 13, 7, 139}, {205, 31, 5, 15, 17}, {0, 0, 0, 0, 0},  /* 69 */
  {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {205, 31, 5, 15, 65},  /* 72 */
  {0, 0, 0, 0, 0}, {396, 63, 17, 7, 210}, {0, 0, 0, 0, 0},  /* 75 */
  {251, 31, 5, 15, 99}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},  /* 78 */
  {0, 0, 0, 0, 0}, {205, 31, 5, 15, 135}, {0, 0, 0, 0, 0},  /* 81 */
  {412, 31, 9, 7, 405}, {0, 0, 0, 0, 0}, {831, 127, 0, 0, 1114},  /* 84 */
  {0, 0, 0, 0, 0}, {533, 127, 0, 0, 764}, {0, 0, 0, 0, 0},  /* 87 */
  {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},  /* 90 */
  {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},  /* 93 */
};

audio_t::audio_t() {
  volume = 1.f;
  for (int i = 0; i < 4; i++) sfx_queue[i] = 0xff;
  for (int i = 0; i < SFX_CHANNELS; i++) {
    sfx_channel[i] = 0xff;
    sfx_timer[i] = 0;
  }
  for (int i = 0; i < SFX_COUNT; i++) sfx_volume[i] = sfx_params[i].volume;
  sfx_random.state[0] = 0x5a5a;
  sfx_random.state[1] = 0x1234;
  sfx_random.state[2] = 0x8765;
}

/* Insert into the sorted queue, a lower id takes precedence (Amiga
   enqueue_sfx_clip @0x1beb0). */
void
audio_t::enqueue_sfx(int sfx) {
  if (sfx < 0 || sfx >= SFX_COUNT || sfx_params[sfx].period == 0) return;

  for (int i = 0; i < 4; i++) {
    if (sfx == sfx_queue[i]) return;
    if (sfx < sfx_queue[i]) {
      for (int j = 3; j > i; j--) sfx_queue[j] = sfx_queue[j-1];
      sfx_queue[i] = sfx;
      return;
    }
  }
}

/* Base volume of a sound, the ambient sounds set it before playing. */
void
audio_t::set_sfx_volume(int sfx, int volume) {
  sfx_volume[sfx] = volume;
}

/* Called every 1/50 s: the queued sounds take the channels holding the
   least important sounds, a sound replaces one of the same or a higher
   id. The queue is emptied (Amiga audio_vbl_update @0x2dd6). The original
   draws the random variation from the game generator in the interrupt;
   a separate generator keeps the game deterministic. */
void
audio_t::update_sfx() {
  for (int i = 0; i < SFX_CHANNELS; i++) {
    if (sfx_timer[i] != 0 && --sfx_timer[i] == 0) sfx_channel[i] = 0xff;
  }

  audio_player_t *player = get_sound_player();
  if (player == NULL || !player->is_enabled() || sfx_queue[0] == 0xff) {
    for (int i = 0; i < 4; i++) sfx_queue[i] = 0xff;
    return;
  }

  /* Channels by the id of their sound, free ones first. */
  int order[SFX_CHANNELS];
  for (int i = 0; i < SFX_CHANNELS; i++) order[i] = i;
  std::stable_sort(order, order + SFX_CHANNELS, [this](int a, int b) {
    return sfx_channel[a] > sfx_channel[b];
  });

  for (int i = 0; i < 4 && sfx_queue[i] != 0xff; i++) {
    int channel = order[i];
    int sfx = sfx_queue[i];
    if (sfx_channel[channel] < sfx) break;

    const sfx_params_t *params = &sfx_params[sfx];
    int r = random_int(&sfx_random);
    int period = params->period + (r & params->period_mask);
    int volume = std::min(64, (sfx_volume[sfx] + (r & params->volume_mask)) &
                              0xff);

    sfx_channel[channel] = sfx;
    sfx_timer[channel] = params->duration;
    player->play_track_on_channel(sfx, channel, volume / 64.f,
                                  static_cast<float>(params->period) / period);
  }

  for (int i = 0; i < 4; i++) sfx_queue[i] = 0xff;
}

audio_player_t::audio_player_t() {
  enabled = true;
}

audio_player_t::~audio_player_t() {
  while (track_cache.size()) {
    audio_track_t *track = track_cache.begin()->second;
    track_cache.erase(track_cache.begin());
    delete track;
  }
}

audio_track_t *
audio_player_t::get_track(int track_id) {
  audio_track_t *track = NULL;
  track_cache_t::iterator it = track_cache.find(track_id);
  if (it == track_cache.end()) {
    track = create_track(track_id);
    if (track != NULL) {
      track_cache[track_id] = track;
    }
  } else {
    track = it->second;
  }

  return track;
}

void
audio_player_t::play_track(int track_id) {
  if (!is_enabled()) {
    return;
  }

  audio_track_t *track = get_track(track_id);
  if (track != NULL) {
    track->play();
  }
}

void
audio_player_t::play_track_on_channel(int track_id, int channel,
                                      float volume, float ratio) {
  if (!is_enabled()) {
    return;
  }

  audio_track_t *track = get_track(track_id);
  if (track != NULL) {
    track->play_on_channel(channel, volume, ratio);
  }
}
