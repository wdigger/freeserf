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

#include "src/audio-sdlmixer.h"

#include <cstring>
#include <cassert>
#include <algorithm>

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "src/misc.h"
BEGIN_EXT_C
  #include "src/log.h"
END_EXT_C
#include "src/data.h"
#include "src/data-source.h"

#ifdef min
# undef min
#endif

#ifdef max
# undef max
#endif

/* Tracks of the sound effect channels and their sounds' volume. */
#define SFX_TRACKS  SFX_CHANNELS

static MIX_Mixer *mixer = NULL;
static MIX_Track *sfx_tracks[SFX_TRACKS];
static float sfx_track_volume[SFX_TRACKS];
static float sfx_master_volume = 1.f;
static MIX_Track *music_track = NULL;

audio_t *
audio_t::get_instance() {
  if (instance == NULL) {
    instance = new audio_sdlmixer_t();
  }
  return instance;
}

audio_sdlmixer_t::audio_sdlmixer_t() {
  LOGI("audio-sdlmixer", "Initializing audio driver `sdlmixer'.");

  if (!SDL_Init(SDL_INIT_AUDIO)) {
    LOGE("audio-sdlmixer", "Could not init SDL audio: %s.", SDL_GetError());
    assert(false);
  }

  if (!MIX_Init()) {
    LOGE("audio-sdlmixer", "Could not init SDL_mixer: %s.", SDL_GetError());
    assert(false);
  }

  SDL_AudioSpec spec;
  spec.format = SDL_AUDIO_S16;
  spec.channels = 2;
  spec.freq = 8000;
  mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
  if (mixer == NULL) {
    LOGE("audio-sdlmixer", "Could not open audio device: %s.", SDL_GetError());
    assert(false);
  }

  for (int i = 0; i < SFX_TRACKS; i++) {
    sfx_track_volume[i] = 1.f;
    sfx_tracks[i] = MIX_CreateTrack(mixer);
    if (sfx_tracks[i] == NULL) {
      LOGE("audio-sdlmixer", "Failed to allocate tracks: %s.", SDL_GetError());
      assert(false);
    }
  }

  music_track = MIX_CreateTrack(mixer);
  if (music_track == NULL) {
    LOGE("audio-sdlmixer", "Failed to allocate tracks: %s.", SDL_GetError());
    assert(false);
  }

  volume = 1.f;

  sfx_player = new sfx_player_t();
  midi_player = new midi_player_t();
}

audio_sdlmixer_t::~audio_sdlmixer_t() {
  if (sfx_player != NULL) {
    delete sfx_player;
    sfx_player = NULL;
  }

  if (midi_player != NULL) {
    delete midi_player;
    midi_player = NULL;
  }

  /* Destroys the tracks too. */
  MIX_DestroyMixer(mixer);
  mixer = NULL;
  music_track = NULL;
  MIX_Quit();
}

float
audio_sdlmixer_t::get_volume() {
  return volume;
}

void
audio_sdlmixer_t::set_volume(float volume) {
  volume = std::max(0.f, std::min(volume, 1.f));
  this->volume = volume;

  if (midi_player != NULL) {
    audio_volume_controller_t *volume_controller =
                                           midi_player->get_volume_controller();
    if (volume_controller != NULL) {
      volume_controller->set_volume(volume);
    }
  }

  if (sfx_player != NULL) {
    audio_volume_controller_t *volume_controller =
                                            sfx_player->get_volume_controller();
    if (volume_controller != NULL) {
      volume_controller->set_volume(volume);
    }
  }
}

void
audio_sdlmixer_t::volume_up() {
  float volume = get_volume();
  set_volume(volume + 0.1f);
}

void
audio_sdlmixer_t::volume_down() {
  float volume = get_volume();
  set_volume(volume - 0.1f);
}

sfx_player_t::sfx_player_t() {
  volume = 1.f;
}

audio_track_t *
sfx_player_t::create_track(int track_id) {
  data_t *data = data_t::get_instance();
  data_source_t *data_source = data->get_data_source();

  size_t size = 0;
  void *wav = data_source->get_sound(track_id, &size);
  if (wav == NULL) {
    return NULL;
  }

  SDL_IOStream *io = SDL_IOFromConstMem(wav, size);
  MIX_Audio *chunk = MIX_LoadAudio_IO(mixer, io, true, true);
  free(wav);
  if (chunk == NULL) {
    LOGE("audio-sdlmixer", "MIX_LoadAudio_IO: %s.", SDL_GetError());
    return NULL;
  }

  return new sfx_track_t(chunk);
}

void
sfx_player_t::enable(bool enable) {
  enabled = enable;
  if (!enabled) {
    stop();
  }
}

void
sfx_player_t::stop() {
  for (int i = 0; i < SFX_TRACKS; i++) {
    MIX_StopTrack(sfx_tracks[i], 0);
  }
}

float
sfx_player_t::get_volume() {
  return volume;
}

void
sfx_player_t::set_volume(float volume) {
  volume = std::max(0.f, std::min(volume, 1.f));
  this->volume = volume;
  sfx_master_volume = volume;
  for (int i = 0; i < SFX_TRACKS; i++) {
    MIX_SetTrackGain(sfx_tracks[i], volume * sfx_track_volume[i]);
  }
}

void
sfx_player_t::volume_up() {
  set_volume(get_volume() + 0.1f);
}

void
sfx_player_t::volume_down() {
  set_volume(get_volume() - 0.1f);
}

sfx_track_t::sfx_track_t(MIX_Audio *chunk) {
  this->chunk = chunk;
}

sfx_track_t::~sfx_track_t() {
  MIX_DestroyAudio(chunk);
}

void
sfx_track_t::play() {
  /* Play on the first free track. */
  for (int i = 0; i < SFX_TRACKS; i++) {
    MIX_Track *track = sfx_tracks[i];
    if (MIX_TrackPlaying(track)) continue;

    if (!MIX_SetTrackAudio(track, chunk) || !MIX_PlayTrack(track, 0)) {
      LOGE("audio-sdlmixer", "Could not play SFX clip: %s.", SDL_GetError());
    }
    return;
  }

  LOGE("audio-sdlmixer", "Could not play SFX clip: no free track.");
}

void
sfx_track_t::play_on_channel(int channel, float volume, float ratio) {
  MIX_Track *track = sfx_tracks[channel];
  MIX_StopTrack(track, 0);
  sfx_track_volume[channel] = volume;
  MIX_SetTrackGain(track, sfx_master_volume * volume);
  MIX_SetTrackFrequencyRatio(track, ratio);
  if (!MIX_SetTrackAudio(track, chunk) || !MIX_PlayTrack(track, 0)) {
    LOGE("audio-sdlmixer", "Could not play SFX clip: %s.", SDL_GetError());
  }
}

midi_player_t::midi_player_t() {
  if (current_midi_player != NULL) {
    LOGE("audio-sdlmixer", "Only one midi player is allowed.");
    assert(0);
  }
  current_track = MIDI_TRACK_NONE;
  current_midi_player = this;
  MIX_SetTrackStoppedCallback(music_track, music_finished_hook, NULL);
}

midi_player_t::~midi_player_t() {
  if (music_track != NULL) {
    MIX_SetTrackStoppedCallback(music_track, NULL, NULL);
  }
  current_midi_player = NULL;
}

audio_track_t *
midi_player_t::create_track(int track_id) {
  data_t *data = data_t::get_instance();
  data_source_t *data_source = data->get_data_source();

  size_t size = 0;
  void *midi = data_source->get_music(track_id, &size);
  if (midi == NULL) {
    return NULL;
  }

  /* MIDI is played by FluidSynth (needs a SoundFont, path taken from
     SDL_SOUNDFONTS like SDL2_mixer did) or Timidity. */
  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER,
                         SDL_IOFromConstMem(midi, size));
  SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, true);
  SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER,
                         mixer);
  const char *soundfont = SDL_getenv("SDL_SOUNDFONTS");
  if (soundfont != NULL) {
    SDL_SetStringProperty(props,
                          "SDL_mixer.decoder.fluidsynth.soundfont_path",
                          soundfont);
  }
  MIX_Audio *music = MIX_LoadAudioWithProperties(props);
  SDL_DestroyProperties(props);
  free(midi);
  if (music == NULL) {
    LOGW("audio-sdlmixer", "Could not load MIDI track: %s.", SDL_GetError());
    return NULL;
  }

  return new midi_track_t(music);
}

void
midi_player_t::play_track(int track_id) {
  if ((track_id <= MIDI_TRACK_NONE) || (track_id > MIDI_TRACK_LAST)) {
    track_id = MIDI_TRACK_0;
  }
  current_track = static_cast<midi_t>(track_id);
  audio_player_t::play_track(track_id);
}

void
midi_player_t::enable(bool enable) {
  enabled = enable;
  if (!enabled) {
    stop();
  }
}

void
midi_player_t::stop() {
  MIX_StopTrack(music_track, 0);
}

float
midi_player_t::get_volume() {
  return MIX_GetTrackGain(music_track);
}

void
midi_player_t::set_volume(float volume) {
  volume = std::max(0.f, std::min(volume, 1.f));
  MIX_SetTrackGain(music_track, volume);
}

void
midi_player_t::volume_up() {
  set_volume(get_volume() + 0.1f);
}

void
midi_player_t::volume_down() {
  set_volume(get_volume() - 0.1f);
}

midi_player_t *midi_player_t::current_midi_player = NULL;

void
midi_player_t::music_finished_hook(void *userdata, MIX_Track *track) {
  if (current_midi_player != NULL) {
    event_loop_t *event_loop = event_loop_t::get_instance();
    event_loop->deferred_call(current_midi_player, NULL);
  }
}

void
midi_player_t::deferred_call(void *data) {
  music_finished();
}

void
midi_player_t::music_finished() {
  if (is_enabled()) {
    play_track(current_track + 1);
  }
}

midi_track_t::midi_track_t(MIX_Audio *chunk) {
  this->chunk = chunk;
}

midi_track_t::~midi_track_t() {
  MIX_DestroyAudio(chunk);
}

void
midi_track_t::play() {
  /* Replacing the playing track must not fire the finished hook. */
  MIX_LockMixer(mixer);
  MIX_SetTrackStoppedCallback(music_track, NULL, NULL);
  MIX_StopTrack(music_track, 0);
  bool r = MIX_SetTrackAudio(music_track, chunk) &&
           MIX_PlayTrack(music_track, 0);
  MIX_SetTrackStoppedCallback(music_track, midi_player_t::music_finished_hook,
                              NULL);
  MIX_UnlockMixer(mixer);
  if (!r) {
    LOGW("audio-sdlmixer", "Could not play MIDI track: %s\n", SDL_GetError());
  }
}
