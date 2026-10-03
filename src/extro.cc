/*
 * extro.cc - End sequence after the last mission
 *
 * Copyright (C) 2013  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "src/extro.h"

#include "src/misc.h"
BEGIN_EXT_C
  #include "src/game.h"
END_EXT_C
#include "src/gfx.h"
#include "src/data.h"
#include "src/audio.h"
#include "src/interface.h"

#define EXTRO_WIDTH   320
#define EXTRO_HEIGHT  200

/* Frames (one per tick, 50 per second as the PAL original). */
#define EXTRO_HOLD_START    200   /* Picture still before scrolling. */
#define EXTRO_SCROLL_STEP    10   /* Frames per scrolled pixel. */
#define EXTRO_SCROLL_MAX    319
#define EXTRO_HOLD_END      160   /* Still at the end before the text. */
#define EXTRO_TEXT_TIME    2700   /* Text shown until the end. */

#define EXTRO_SCROLL_END  (EXTRO_HOLD_START + \
                           EXTRO_SCROLL_MAX * EXTRO_SCROLL_STEP)
#define EXTRO_TEXT_START  (EXTRO_SCROLL_END + EXTRO_HOLD_END)
#define EXTRO_END         (EXTRO_TEXT_START + EXTRO_TEXT_TIME)

/* Positions of the animations in the landscape picture. */
#define EXTRO_FLAG_X  506
#define EXTRO_FLAG_Y   10
#define EXTRO_OWL_X   142
#define EXTRO_OWL_Y    78

extro_t::extro_t(interface_t *interface) {
  this->interface = interface;
  view = NULL;
  counter = 0;
}

extro_t::~extro_t() {
  if (view != NULL) {
    delete view;
    view = NULL;
  }
}

void
extro_t::start() {
  counter = 0;
  set_displayed(true);
  set_enabled(true);
  set_redraw();

  audio_t *audio = audio_t::get_instance();
  audio_player_t *player = audio->get_music_player();
  if (player != NULL && player->is_enabled()) {
    player->play_track(MIDI_TRACK_2);
  }
}

/* Advance one frame; at the end return to the game. */
void
extro_t::step() {
  if (!displayed) return;

  counter += 1;
  if (counter >= EXTRO_END) {
    finish();
  } else {
    set_redraw();
  }
}

/* Leave the sequence and return to the game. */
void
extro_t::finish() {
  if (!displayed) return;

  set_displayed(false);
  set_enabled(false);

  audio_t *audio = audio_t::get_instance();
  audio_player_t *player = audio->get_music_player();
  if (player != NULL && player->is_enabled()) {
    player->play_track(MIDI_TRACK_0);
  }

  game_pause(0);
}

void
extro_t::internal_draw() {
  /* Owl eyes: frame by table @0x1def2, one entry per 8 frames. */
  static const int owl_frame[] = {
    1, 2, 2, 1, 0, 3, 4, 4, 4, 3, 0, 5, 6, 6, 5, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  };

  /* Closing text (@0x1dfbc), column 40 of the picture. The Amiga rows
     are 5 lower; the last line would not fit the 200 rows of the DOS
     picture there. */
  static const struct {
    int row;
    const char *text;
  } line[] = {
    {   5, "THE FIRST RAYS OF THE" },
    {  14, "SUN GLINT OFF THE" },
    {  23, "PARAPITS OF YOUR CASTLE." },
    {  32, "THE DAY BEGINS. THE" },
    {  41, "BIRDS START TO SING AND" },
    {  50, "THE SETTLERS RISE FROM" },
    {  59, "THEIR FLUMBER TO BEGIN" },
    {  68, "ANOTHER DAY OF JOLLY" },
    {  77, "TOIL." },
    { 101, "ALL IS WELL AS YOU" },
    { 110, "GO TO YOUR WINDOW" },
    { 119, "GAZE UPON THE UN-" },
    { 128, "SPOILED COUNTRYSIDE" },
    { 137, "AND TAKE A LUNG-FULL" },
    { 146, "OF PURE COUNTRY AIR." },
    { 155, "FOR TODAY IS THE DAY" },
    { 164, "YOU CAN AT LAST PRO-" },
    { 173, "CLAIM YOURSELF BE" },
    { 182, "UNDESPUTED RULER OF" },
    { 191, "YOUR KINGDOM." },
    {   0, NULL }
  };

  if (view == NULL) {
    view = gfx_t::get_instance()->create_frame(EXTRO_WIDTH, EXTRO_HEIGHT);
  }

  int scroll = 0;
  if (counter >= EXTRO_SCROLL_END) {
    scroll = EXTRO_SCROLL_MAX;
  } else if (counter >= EXTRO_HOLD_START) {
    scroll = (counter - EXTRO_HOLD_START) / EXTRO_SCROLL_STEP;
  }

  /* The animation counters advance before the first frame is shown. */
  int owl = owl_frame[((counter + 1) & 0xff) >> 3];
  int flag = ((counter + 1) % 28) >> 2;

  view->draw_sprite(-scroll, 0, DATA_ART_LANDSCAPE);
  view->draw_sprite(EXTRO_OWL_X - scroll, EXTRO_OWL_Y,
                    DATA_ART_OWL_BASE + owl);
  view->draw_sprite(EXTRO_FLAG_X - scroll, EXTRO_FLAG_Y,
                    DATA_ART_FLAG_BASE + flag);

  if (counter >= EXTRO_TEXT_START) {
    for (int i = 0; line[i].text != NULL; i++) {
      view->draw_string(EXTRO_WIDTH - scroll, line[i].row, 47, 1,
                        line[i].text);
    }
  }

  frame->fill_rect(0, 0, width, height, 1);
  frame->draw_frame((width - EXTRO_WIDTH) / 2, (height - EXTRO_HEIGHT) / 2,
                    0, 0, view, EXTRO_WIDTH, EXTRO_HEIGHT);
}
