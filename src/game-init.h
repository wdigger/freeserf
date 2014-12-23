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
  #include "game.h"
#ifndef _MSC_VER
}
#endif

class interface_t;

class game_init_box_t : public gui_object_t {
public:
  interface_t *interface;

  int map_size;
  int game_mission;

  uint face[GAME_MAX_PLAYER_COUNT];
  uint intelligence[GAME_MAX_PLAYER_COUNT];
  uint supplies[GAME_MAX_PLAYER_COUNT];
  uint reproduction[GAME_MAX_PLAYER_COUNT];

  game_init_box_t(interface_t *interface);

  virtual void internal_draw(frame_t *frame);
  virtual int internal_handle_event(const gui_event_t *event);

protected:
  int handle_event_click(int x, int y);
};

#endif /* !_GAME_INIT_H */
