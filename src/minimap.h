/*
 * minimap.h - Minimap GUI component
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

#ifndef _MINIMAP_H
#define _MINIMAP_H

#include "gui.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "map.h"
  #include "game.h"
#ifndef _MSC_VER
}
#endif

class interface_t;

class minimap_t
  : public gui_object_t
{
protected:
  int offset_x, offset_y;
  int scale;

  int advanced;
  int flags;

  map_t *map;

public:
  minimap_t(map_t *map);

  void set_map(map_t *map);

  virtual void internal_draw();

  void move_to_map_pos(map_pos_t pos);
  void move_by_pixels(int x, int y);
  map_pos_t get_current_map_pos();

  void screen_pix_from_map_pos(map_pos_t pos, int *sx, int *sy);
  map_pos_t map_pos_from_screen_pix(int x, int y);

  void set_scale(int scale);
  int get_scale() { return scale; }
  int get_flags() { return flags; }
  int get_advanced() { return advanced; }
  void set_advanced(int advanced);
  void set_flags(int flags);
  void clear_flags(int flags);
  void invert_flags(int flags);

protected:
  void draw_minimap_point(int col, int row, uint8_t color,
                          int density, frame_t *frame);
  void draw_minimap_map(frame_t *frame);
  void draw_minimap_grid(frame_t *frame);
  void draw_minimap_rect(frame_t *frame);

  virtual int handle_drag(int dx, int dy);
  int handle_scroll(int up);

	void map_pix_from_map_coord(map_pos_t pos, int *mx, int *my);
};

class game_minimap_t
  : public minimap_t
{
protected:
  interface_t *interface;

  game_t *game;

public:
  game_minimap_t(interface_t *interface, game_t *game);

protected:
  void draw_minimap_ownership(int density, frame_t *frame);
  void draw_minimap_roads(frame_t *frame);
  void draw_minimap_buildings(frame_t *frame);
  void draw_minimap_traffic(frame_t *frame);

  virtual void internal_draw();
  virtual int handle_click_left(int x, int y);
};

#endif /* !_MINIMAP_H */
