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
  #include "building.h"
  #include "serf.h"
#ifndef _MSC_VER
}
#endif

#include <map>

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
protected:
  int offset_x, offset_y;
  viewport_layer_t layers;
  interface_t *interface;
  uint last_tick;
  int show_possible_build;

  uint32_t *serf_animation_table;

  /* Cache prerendered tiles of the landscape. */
  typedef std::map<uint, frame_t*> tiles_map_t;
  tiles_map_t landscape_tiles;

public:
  viewport_t(interface_t *interface);
  virtual ~viewport_t();

  virtual void internal_draw();
  virtual void layout();

  void move_to_map_pos(map_pos_t pos);
  void move_by_pixels(int x, int y);
  map_pos_t get_current_map_pos();

  void screen_pix_from_map_pix(int mx, int my, int *sx, int *sy);
  void map_pix_from_map_coord(map_pos_t pos, int h, int *mx, int *my);
  map_pos_t map_pos_from_screen_pix(int x, int y);

  void map_reinit();
  void redraw_map_pos(map_pos_t pos);

  void update();

  void switch_possible_build();
  void switch_layer(viewport_layer_t layer);
  void set_layers(viewport_layer_t layers);
  void move_offset(int dx, int dy);

protected:
  void map_deinit();
  frame_t *get_tile_frame(uint tid, int tc, int tr);

  void draw_landscape(frame_t *frame);
  void draw_paths_and_borders(frame_t *frame);
  void draw_game_objects(int layers, frame_t *frame);
  void draw_map_cursor_possible_build(interface_t *interface, frame_t *frame);
  void draw_map_cursor(frame_t *frame);
  void draw_base_grid_overlay(int color, frame_t *frame);
  void draw_height_grid_overlay(int color, frame_t *frame);
  void draw_unharmed_building(building_t *building, int x, int y, frame_t *frame);
  void draw_burning_building(building_t *building, int x, int y, frame_t *frame);
  void draw_building(map_pos_t pos, int x, int y, frame_t *frame);
  void draw_map_objects_row(map_pos_t pos, int y_base, 
                            int cols, int x_base, frame_t *frame);
  void draw_active_serf(serf_t *serf, map_pos_t pos, int x_base, int y_base,
                        frame_t *frame);
  void draw_serf_row_behind(map_pos_t pos, int y_base, int cols, int x_base,
                            frame_t *frame);
  void draw_serf_row(map_pos_t pos, int y_base, int cols, int x_base,
                     frame_t *frame);
  virtual bool handle_click_left(int x, int y);
  virtual bool handle_dbl_click(int x, int y, event_button_t button);
  virtual bool handle_drag(int x, int y);

  void load_serf_animation_table();
};

#endif /* ! _VIEWPORT_H */
