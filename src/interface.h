/*
 * interface.h - Top-level GUI interface
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

#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "gui.h"
#include "viewport.h"
#include "panel.h"
#include "game-init.h"
#include "notification.h"
#include "popup.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "list.h"
  #include "random.h"
#ifndef _MSC_VER
}
#endif

/* The length between game updates in miliseconds. */
#define TICK_LENGTH  20
#define TICKS_PER_SEC  (1000/TICK_LENGTH)

#define MAX_ROAD_LENGTH  256

static const int map_building_sprite[] = {
  0, 0xa7, 0xa8, 0xae, 0xa9,
  0xa3, 0xa4, 0xa5, 0xa6,
  0xaa, 0xc0, 0xab, 0x9a, 0x9c, 0x9b, 0xbc,
  0xa2, 0xa0, 0xa1, 0x99, 0x9d, 0x9e, 0x98, 0x9f, 0xb2
};

typedef enum {
  MAP_CURSOR_TYPE_NONE = 0,
  MAP_CURSOR_TYPE_FLAG,
  MAP_CURSOR_TYPE_REMOVABLE_FLAG,
  MAP_CURSOR_TYPE_BUILDING,
  MAP_CURSOR_TYPE_PATH,
  MAP_CURSOR_TYPE_CLEAR_BY_FLAG,
  MAP_CURSOR_TYPE_CLEAR_BY_PATH,
  MAP_CURSOR_TYPE_CLEAR
} map_cursor_type_t;

typedef struct {
  int sprite;
  int x, y;
} sprite_loc_t;

class interface_t
  : public gui_container_t
{
public:
  gui_object_t *top;
  int redraw_top;
  list_t floats;

  gui_object_t *cursor_lock_target;

  uint32_t *serf_animation_table;

  random_state_t random;

  viewport_t *viewport;
  panel_bar_t *panel;
  popup_box_t *popup;
  game_init_box_t *init_box;
  notification_box_t *notification_box;

  map_pos_t map_cursor_pos;
  map_cursor_type_t map_cursor_type;
  panel_btn_t panel_btn_type;

  uint last_const_tick;

  int building_road;
  map_pos_t building_road_source;
  dir_t building_road_dirs[MAX_ROAD_LENGTH];
  int building_road_length;
  int building_road_valid_dir;

  int sfx_queue[4];

  int panel_btns[5];

  player_t *player;
  int config;
  int msg_flags;

  sprite_loc_t map_cursor_sprites[7];

  int current_stat_8_mode;
  int current_stat_7_item;

  int water_in_view;
  int trees_in_view;

  int return_timeout;
  int return_pos;

  interface_t();

  virtual void internal_draw(frame_t *frame);
  virtual int internal_handle_event(const gui_event_t *event);
  virtual void internal_set_size(int width, int height);

  virtual void internal_set_redraw_child(gui_object_t *child);
  virtual int internal_get_child_position(gui_object_t *child, int *x, int *t);

  viewport_t *get_top_viewport();
  panel_bar_t *get_panel_bar();
  popup_box_t *get_popup_box();

  void open_popup(int box);
  void close_popup();

  void open_game_init();
  void close_game_init();

  void open_message();
  void return_from_message();
  void close_message();

  void set_player(uint player);
  void update_map_cursor_pos(map_pos_t pos);

  void build_road_begin();
  void build_road_end();
  int build_road_segment(dir_t dir);
  int remove_road_segment();
  int extend_road(dir_t *dirs, uint length);

  void demolish_object();

  void build_flag();
  void build_building(building_type_t type);
  void build_castle();

  void set_top(gui_object_t *obj);
  void add_float(gui_object_t *obj, int x, int y, int width, int height);

  void update();
};

#endif /* !_INTERFACE_H */
