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

#ifndef _MSC_VER
extern "C" {
#endif
  #include "mission.h"
  #include "random.h"
#ifndef _MSC_VER
}
#endif

#include <algorithm>

typedef enum {
  ACTION_START_GAME,
  ACTION_TOGGLE_GAME_TYPE,
  ACTION_SHOW_OPTIONS,
  ACTION_SHOW_LOAD_GAME,
  ACTION_INCREMENT,
  ACTION_DECREMENT,
  ACTION_CLOSE
} action_t;


static void
draw_box_icon(int x, int y, int sprite, frame_t *frame)
{
  gfx_draw_sprite(8*x+20, y+16, DATA_ICON_BASE + sprite, frame);
}

static void
draw_box_string(int x, int y, frame_t *frame, const char *str)
{
  gfx_draw_string(8*x+20, y+16, 31, 1, frame, str);
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
  gfx_fill_rect(0, 0, width, height, 1, frame);

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

    draw_box_string(10, 0, frame, "Start new game");
    draw_box_string(10, 14, frame, "Map size:");
    draw_box_string(20, 14, frame, map_size);

    draw_box_icon(25, 0, 265, frame);
  } else {
    draw_box_icon(5, 0, 260, frame);

    char level[4] = {0};
    snprintf(level, 4, "%d", game_mission+1);

    draw_box_string(10, 0, frame, "Start mission");
    draw_box_string(10, 14, frame, "Mission:");
    draw_box_string(20, 14, frame, level);

    draw_box_icon(28, 0, 237, frame);
    draw_box_icon(28, 16, 240, frame);
  }

  /* Game info */
  for (int i = 0; i < GAME_MAX_PLAYER_COUNT; i++) {
    draw_player_box(i, 10*i, 40, frame);
  }

  draw_box_icon(38, 128, 60, frame); /* exit */
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
    gfx_fill_rect(x + 64, y + 76 - supplies, 4, supplies, 67, frame);

    int intelligence = mission->player[player].intelligence;
    gfx_fill_rect(x + 70, y + 76 - intelligence, 4, intelligence, 30, frame);

    int reproduction = mission->player[player].reproduction;
    gfx_fill_rect(x + 76, y + 76 - reproduction, 4, reproduction, 75, frame);
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
    game_init();
    if (game_mission < 0) {
      random_state_t rnd = random_generate_random_state();
      int r = game_load_random_map(map_size, &rnd);
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
    } else {
      game_mission = -1;
      map_size = 3;
      mission = &custom_mission;
    }
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
    }
    break;
  case ACTION_DECREMENT:
    if (game_mission < 0) {
      map_size = std::max(3, map_size-1);
    } else {
      game_mission = std::max(0, game_mission-1);
    }
    break;
  case ACTION_CLOSE:
    interface->close_game_init();
    break;
  default:
    break;
  }
}

int
game_init_box_t::handle_click_left(int x, int y)
{
  const int clickmap[] = {
    ACTION_START_GAME, 20, 16, 32, 32,
    ACTION_TOGGLE_GAME_TYPE, 60, 16, 32, 32,
    ACTION_SHOW_OPTIONS, 268, 16, 32, 32,
    ACTION_SHOW_LOAD_GAME, 308, 16, 32, 32,
    ACTION_INCREMENT, 244, 16, 16, 16,
    ACTION_DECREMENT, 244, 32, 16, 16,
    ACTION_CLOSE, 324, 144, 16, 16,
    -1
  };

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
  for (int i = 0; i < GAME_MAX_PLAYER_COUNT; i++) {
    if (x >= 80*i+20 && x <= 80*(i+1)+20 &&
        y >= 48 && y <= 132) {
      x -= 80*i + 20;

      if (x >= 8 && x < 8+32 &&
          y >= 48 && y < 132) {
        /* Face */
        int in_use = 0;
        do {
          mission->player[i].face = (mission->player[i].face + 1) % 14;

          /* Check that face is not already in use
             by another player */
          in_use = 0;
          for (int j = 0; j < GAME_MAX_PLAYER_COUNT; j++) {
            if (i != j &&
                mission->player[i].face != 0 &&
                mission->player[j].face == mission->player[i].face) {
              in_use = 1;
              break;
            }
          }
        } while (in_use);
      } else if (x >= 48 && x < 72 &&
           y >= 64 && y < 80) {
        /* Controller */
        /* TODO */
      } else if (x >= 52 && x < 52+4 &&
           y >= 84 && y < 124) {
        /* Supplies */
        mission->player[i].supplies = clamp(0, 124 - y, 40);
      } else if (x >= 58 && x < 58+4 &&
           y >= 84 && y < 124) {
        /* Intelligence */
        mission->player[i].intelligence = clamp(0, 124 - y, 40);
      } else if (x >= 64 && x < 64+4 &&
           y >= 84 && y < 124) {
        /* Reproduction */
        mission->player[i].reproduction = clamp(0, 124 - y, 40);
      }
      break;
    }
  }

  return 0;
}

game_init_box_t::game_init_box_t(interface_t *interface)
{
  this->interface = interface;
  map_size = 3;
  game_mission = -1;

  set_size(360, 174);

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

  mission = &custom_mission;
}
