/*
 * viewport.h - Viewport GUI component
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

#ifndef _VIEWPORT_H
#define _VIEWPORT_H

#include "gui.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "map.h"
#ifndef _MSC_VER
}
#endif

typedef enum {
  VIEWPORT_LAYER_LANDSCAPE = 1<<0,
  VIEWPORT_LAYER_PATHS = 1<<1,
  VIEWPORT_LAYER_OBJECTS = 1<<2,
  VIEWPORT_LAYER_SERFS = 1<<3,
  VIEWPORT_LAYER_CURSOR = 1<<4,
  VIEWPORT_LAYER_ALL = (1<<5)-1,

  VIEWPORT_LAYER_GRID = 1<<5
} viewport_layer_t;

class interface_t;

class viewport_t
  : public gui_object_t
{
public:
  int offset_x, offset_y;
  viewport_layer_t layers;
  interface_t *interface;
  uint last_tick;
  int show_possible_build;

  viewport_t(interface_t *interface);

  virtual void internal_draw(frame_t *frame);
  virtual int internal_handle_event(const gui_event_t *event);

  void move_to_map_pos(map_pos_t pos);
  void move_by_pixels(int x, int y);
  map_pos_t get_current_map_pos();

  void screen_pix_from_map_pix(int mx, int my, int *sx, int *sy);
  void map_pix_from_map_coord(map_pos_t pos, int h, int *mx, int *my);
  map_pos_t map_pos_from_screen_pix(int x, int y);

  void map_reinit();
  void map_deinit();
  void redraw_map_pos(map_pos_t pos);

  void update();
};

#endif /* ! _VIEWPORT_H */
