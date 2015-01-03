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
#include "panel.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "random.h"
  #include "map.h"
  #include "player.h"
  #include "building.h"
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

class viewport_t;
class panel_bar_t;
class popup_box_t;
class game_init_box_t;
class notification_box_t;

class interface_t
  : public gui_object_t
{
protected:
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

  bool building_road;
  map_pos_t building_road_source;
  dir_t building_road_dirs[MAX_ROAD_LENGTH];
  int building_road_length;
  int building_road_valid_dir;

  int sfx_queue[4];

  player_t *player;
  int config;
  int msg_flags;

  int map_cursor_sprites[7];

  int water_in_view;
  int trees_in_view;

  int return_timeout;
  int return_pos;

public:
  interface_t();
  virtual ~interface_t();

  viewport_t *get_viewport();
  panel_bar_t *get_panel_bar();
  popup_box_t *get_popup_box();
  notification_box_t *get_notification_box() { return notification_box; }

  void open_popup(int box);
  void close_popup();

  void open_game_init();
  void close_game_init();

  void open_message();
  void return_from_message();
  void close_message();

  player_t *get_player() { return player; }
  void set_player(uint player);

  map_pos_t get_map_cursor_pos() { return map_cursor_pos; }
  void update_map_cursor_pos(map_pos_t pos);
  map_cursor_type_t current_map_cursor_type() { return map_cursor_type; }
  dir_t *get_building_road_dirs() { return building_road_dirs; }

  void build_road_begin();
  void build_road_end();
  int build_road_segment(dir_t dir);
  int remove_road_segment();
  int extend_road(dir_t *dirs, uint length);

  void demolish_object();

  void build_flag();
  void build_building(building_type_t type);
  void build_castle();

  void update();

  int get_config() { return config; }
  void set_config(int config) { this->config = config; }

  bool get_building_road() { return building_road; }
  map_pos_t get_building_road_source() { return building_road_source; }
  int get_building_road_length() { return building_road_length; }
  void set_building_road_length(int building_road_length) { this->building_road_length = building_road_length; }
  int get_building_road_valid_dir() { return building_road_valid_dir; }

  int get_msg_flags() { return msg_flags; }
  void set_msg_flags(int msg_flags) { this->msg_flags = msg_flags; }

  random_state_t *get_random() { return &random; }

  int *get_map_cursor_sprites() { return map_cursor_sprites; }

protected:
  virtual void internal_draw();
  virtual void layout();
  virtual int handle_key_pressed(char key, int modifier);

  void interface_determine_map_cursor_type();
  void interface_determine_map_cursor_type_road();
  void interface_update_interface();
};

#endif /* !_INTERFACE_H */
