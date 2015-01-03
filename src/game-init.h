/*
 * game-init.h - Game initialization GUI component header
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

#ifndef _GAME_INIT_H
#define _GAME_INIT_H

#include "gui.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "map.h"
  #include "mission.h"
#ifndef _MSC_VER
}
#endif

class interface_t;
class minimap_t;
class random_input_t;

class game_init_box_t : public gui_object_t {
protected:
  interface_t *interface;

  int map_size;
  int game_mission;

  mission_t custom_mission;
  mission_t *mission;

  map_t *map;
  minimap_t *minimap;

  random_input_t *field;

public:
  game_init_box_t(interface_t *interface);
  ~game_init_box_t();

protected:
  virtual void internal_draw();
  void draw_player_box(int player, int x, int y, frame_t *frame);

  virtual int handle_click_left(int x, int y);
  void handle_action(int action);
  int handle_player_click(int player, int x, int y);

  void generate_map_priview();
};

#endif /* !_GAME_INIT_H */
