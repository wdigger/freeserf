/*
 * game-init.c - Game initialization GUI component
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

#include "game-init.h"
#include "interface.h"
#include "viewport.h"
#include "minimap.h"
#include "text-input.h"
#include "data.h"
#include "version.h"

#include <algorithm>

class random_input_t
  : public text_input_t
{
protected:
  std::string saved_text;

public:
  random_input_t() {
    set_filter(text_input_filter);
    set_size(34, 34);
    set_max_length(16);
  }

  void set_random(const random_state_t &rnd) {
    char *str = random_to_string(&rnd);
    set_text(str);
    free(str);
  }
  random_state_t get_random() {
    return string_to_random(text.c_str());
  }

protected:
  static bool
  text_input_filter(const char key, text_input_t *text_input)
  {
    if (key < '1' || key > '8') {
      return false;
    }

    if (strlen(text_input->get_text()) > 16) {
      return false;
    }
    
    return true;
  }

  virtual bool handle_click_left(int x, int y) {
    text_input_t::handle_click_left(x, y);
    saved_text = text;
    text.clear();
    return true;
  }

  virtual bool handle_focus_loose() {
    text_input_t::handle_focus_loose();
    if ((text.length() < 16) && (saved_text.length() == 16)) {
      text = saved_text;
      saved_text.clear();
    }
    return true;
  }
};

typedef enum {
  ACTION_START_GAME,
  ACTION_TOGGLE_GAME_TYPE,
  ACTION_SHOW_OPTIONS,
  ACTION_SHOW_LOAD_GAME,
  ACTION_INCREMENT,
  ACTION_DECREMENT,
  ACTION_CLOSE,
  ACTION_GEN_RANDOM,
  ACTION_APPLY_RANDOM,
} action_t;


static void
draw_box_icon(int x, int y, int sprite, frame_t *frame)
{
  frame->draw_sprite(8*x+20, y+16, DATA_ICON_BASE + sprite);
}

static void
draw_box_string(int x, int y, frame_t *frame, const char *str)
{
  frame->draw_string(8*x+20, y+16, 31, 1, str);
}

/* Get the sprite number for a face. */
static int
get_player_face_sprite(int face)
{
  if (face != 0) return 0x10b + face;
  return 0x119; /* sprite_face_none */
}

void
game_init_box_t::internal_draw()
{
  /* Background */
  frame->fill_rect(0, 0, width, height, 1);

  const int layout[] = {
    266, 0, 0,
    267, 31, 0,
    316, 36, 0,
    -1
  };

  const int *i = layout;
  while (i[0] >= 0) {
    draw_box_icon(i[1], i[2], i[0], frame);
    i += 3;
  }

  /* Game type settings */
  if (game_mission < 0) {
    draw_box_icon(5, 0, 263, frame);

    char map_size[4] = {0};
    snprintf(map_size, 4, "%d", this->map_size);

    draw_box_string(10, 2, frame, "New game");
    draw_box_string(10, 18, frame, "Mapsize:");
    draw_box_string(18, 18, frame, map_size);

    draw_box_icon(20, 0, 265, frame);
  } else {
    draw_box_icon(5, 0, 260, frame);

    char level[4] = {0};
    snprintf(level, 4, "%d", game_mission+1);

    draw_box_string(10, 2, frame, "Start mission");
    draw_box_string(10, 18, frame, "Mission:");
    draw_box_string(20, 18, frame, level);

    draw_box_icon(28, 0, 237, frame);
    draw_box_icon(28, 16, 240, frame);
  }

  /* Game info */
  int x = 0;
  int y = 0;
  for (int i = 0; i < GAME_MAX_PLAYER_COUNT; i++) {
    draw_player_box(i, 10 * x, 40 + y * 80, frame);
    x++;
    if (i == 1) {
      y++;
      x = 0;
    }
  }

  /* Display program name and version in caption */
  char caption[64];
  snprintf(caption, 64, "Version: %s", FREESERF_VERSION);

  draw_box_string(0, 208, frame, caption);

  draw_box_icon(38, 208, 60, frame); /* exit */
}

void
game_init_box_t::draw_player_box(int player, int x, int y, frame_t *frame)
{
  const int layout[] = {
    251, 0, 0,
    252, 0, 72,
    253, 0, 8,
    254, 5, 8,
    255, 9, 8,
    -1
  };

  const int *i = layout;
  while (i[0] >= 0) {
    draw_box_icon(x+i[1], y+i[2], i[0], frame);
    i += 3;
  }

  y += 8;
  x += 1;

  draw_box_icon(x, y, get_player_face_sprite(mission->player[player].face), frame);
  draw_box_icon(x+5, y, 282, frame);

  x *= 8;

  if (mission->player[player].face != 0) {
    int supplies = mission->player[player].supplies;
    frame->fill_rect(x + 64, y + 76 - supplies, 4, supplies, 67);

    int intelligence = mission->player[player].intelligence;
    frame->fill_rect(x + 70, y + 76 - intelligence, 4, intelligence, 30);

    int reproduction = mission->player[player].reproduction;
    frame->fill_rect(x + 76, y + 76 - reproduction, 4, reproduction, 75);
  }
}

void
game_init_box_t::handle_action(int action)
{
  const uint default_player_colors[] = {
    64, 72, 68, 76
  };

  switch (action) {
  case ACTION_START_GAME:
    game_init(0);
    if (game_mission < 0) {
      int r = game_load_random_map(map_size, &mission->rnd);
      if (r < 0) return;

      for (int i = 0; i < GAME_MAX_PLAYER_COUNT; i++) {
        if (mission->player->face == 0) continue;
        int p = game_add_player(mission->player[i].face,
              default_player_colors[i],
              mission->player[i].supplies,
              mission->player[i].reproduction,
              mission->player[i].intelligence);
        if (p < 0) return;
      }
    } else {
      int r = game_load_mission_map(game_mission);
      if (r < 0) return;
    }

    interface->get_viewport()->map_reinit();
    interface->set_player(0);
    interface->close_game_init();
    break;
  case ACTION_TOGGLE_GAME_TYPE:
    if (game_mission < 0) {
      game_mission = 0;
      mission = get_mission(game_mission);
      field->set_displayed(false);
    } else {
      game_mission = -1;
      map_size = 3;
      mission = &custom_mission;
      field->set_displayed(true);
      field->set_random(custom_mission.rnd);
    }
    generate_map_priview();
    set_redraw();
    break;
  case ACTION_SHOW_OPTIONS:
    break;
  case ACTION_SHOW_LOAD_GAME:
    break;
  case ACTION_INCREMENT:
    if (game_mission < 0) {
      map_size = std::min(map_size+1, 10);
    } else {
      game_mission = std::min(game_mission+1, get_mission_count()-1);
      mission = get_mission(game_mission);
    }
    generate_map_priview();
    break;
  case ACTION_DECREMENT:
    if (game_mission < 0) {
      map_size = std::max(3, map_size-1);
    } else {
      game_mission = std::max(0, game_mission-1);
      mission = get_mission(game_mission);
    }
    generate_map_priview();
    break;
  case ACTION_GEN_RANDOM: {
    random_state_t rnd = random_generate_random_state();
    field->set_random(rnd);
    set_redraw();
    break;
  }
  case ACTION_APPLY_RANDOM: {
    const char *str = field->get_text();
    if (strlen(str) == 16) {
      custom_mission.rnd = field->get_random();
      generate_map_priview();
    }
    break;
  }
  case ACTION_CLOSE:
    interface->close_game_init();
    break;
  default:
    break;
  }
}

bool
game_init_box_t::handle_click_left(int x, int y)
{
  const int clickmap_mission[] = {
    ACTION_START_GAME,        20,  16, 32, 32,
    ACTION_TOGGLE_GAME_TYPE,  60,  16, 32, 32,
    ACTION_SHOW_OPTIONS,     268,  16, 32, 32,
    ACTION_SHOW_LOAD_GAME,   308,  16, 32, 32,
    ACTION_INCREMENT,        244,  16, 16, 16,
    ACTION_DECREMENT,        244,  32, 16, 16,
    ACTION_CLOSE,            324, 216, 16, 16,
    -1
  };

  const int clickmap_custom[] = {
    ACTION_START_GAME,        20,  16, 32, 32,
    ACTION_TOGGLE_GAME_TYPE,  60,  16, 32, 32,
    ACTION_SHOW_OPTIONS,     268,  16, 32, 32,
    ACTION_SHOW_LOAD_GAME,   308,  16, 32, 32,
    ACTION_INCREMENT,        180,  24, 24, 24,
    ACTION_DECREMENT,        180,  16,  8,  8,
    ACTION_GEN_RANDOM,       204,  16, 16,  8,
    ACTION_APPLY_RANDOM ,    204,  16, 24, 32,
    ACTION_CLOSE,            324, 216, 16, 16,
    -1
  };

  const int *clickmap = clickmap_mission;
  if (game_mission < 0) {
    clickmap = clickmap_custom;
  }

  const int *i = clickmap;
  while (i[0] >= 0) {
    if (x >= i[1] && x < i[1]+i[3] &&
        y >= i[2] && y < i[2]+i[4]) {
      handle_action(i[0]);
      break;
    }
    i += 5;
  }

  /* Check player area */
  int cx = 0;
  int cy = 0;
  for (int i = 0; i < GAME_MAX_PLAYER_COUNT; i++) {
    int px = 20 + cx*80;
    int py = 56 + cy*80;
    if ((x > px) && (x < px + 80) && (y > py) && (y < py + 80)) {
      if (handle_player_click(i, x - px, y - py)) {
        break;
      }
    }
    cx++;
    if (i == 1) {
      cy++;
      cx = 0;
    }
  }

  return true;
}

int
game_init_box_t::handle_player_click(int player, int x, int y)
{
  if (x < 8 || x > 8 + 64 || y < 8 || y > 76) {
    return 0;
  }

  if (x >= 8 && x < 8+32 &&
      y >= 8 && y < 72) {
    /* Face */
    int in_use = 0;
    do {
      mission->player[player].face = (mission->player[player].face + 1) % 14;

      /* Check that face is not already in use
       by another player */
      in_use = 0;
      for (int j = 0; j < GAME_MAX_PLAYER_COUNT; j++) {
        if (player != j &&
            mission->player[player].face != 0 &&
            mission->player[j].face == mission->player[player].face) {
          in_use = 1;
          break;
        }
      }
    } while (in_use);
  }
  else {
    x -= 8 + 32 + 8 + 3;
    if (x < 0) {
      return 0;
    }
    if (y >= 27 && y < 69) {
      if (x > 0 && x < 6) {
        /* Supplies */
        mission->player[player].supplies = clamp(0, 68 - y, 40);
      }
      else if (x > 6 && x < 12) {
        /* Intelligence */
        mission->player[player].intelligence = clamp(0, 68 - y, 40);
      }
      else if (x > 12 && x < 18) {
        /* Reproduction */
        mission->player[player].reproduction = clamp(0, 68 - y, 40);
      }
    }
  }

  set_redraw();

  return 0;
}

void
game_init_box_t::generate_map_priview()
{
  if (map != NULL) {
    map_deinit(map);
    free(map);
    map = NULL;
  }

  map = (map_t*)malloc(sizeof(map_t));
  map_init(map, map_size);

  random_state_t rnd = mission->rnd;
  rnd.state[0] ^= 0x5a5a;
  rnd.state[1] ^= 0xa5a5;
  rnd.state[2] ^= 0xc3c3;

  map_generate(map, 0, &rnd);

  minimap->set_map(map);

  set_redraw();
}

game_init_box_t::game_init_box_t(interface_t *interface)
{
  this->interface = interface;
  map_size = 3;
  game_mission = -1;

  set_size(360, 254);

  /* Clear player settings */
  for (int i = 0; i < GAME_MAX_PLAYER_COUNT; i++) {
    custom_mission.player[i].face = 0;
    custom_mission.player[i].intelligence = 0;
    custom_mission.player[i].supplies = 0;
    custom_mission.player[i].reproduction = 0;
  }

  /* Create default game setup */
  custom_mission.player[0].face = 12;
  custom_mission.player[0].intelligence = 40;
  custom_mission.player[0].supplies = 40;
  custom_mission.player[0].reproduction = 40;

  custom_mission.player[1].face = 1;
  custom_mission.player[1].intelligence = 20;
  custom_mission.player[1].supplies = 30;
  custom_mission.player[1].reproduction = 40;

  custom_mission.rnd = random_generate_random_state();

  mission = &custom_mission;

  minimap = new minimap_t(NULL);
  minimap->set_displayed(true);
  minimap->set_size(150, 160);
  add_float(minimap, 190, 55);

  map = NULL;
  generate_map_priview();

  field = new random_input_t();
  field->set_random(custom_mission.rnd);
  field->set_displayed(true);
  add_float(field, 19 + 26*8, 15);
}

game_init_box_t::~game_init_box_t()
{
  if (map != NULL) {
    map_deinit(map);
    free(map);
    map = NULL;
  }

  if (minimap != NULL) {
    delete minimap;
    minimap = NULL;
  }

  if (field != NULL) {
    delete field;
    field = NULL;
  }
}
