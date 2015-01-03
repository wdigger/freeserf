/*
 * popup.h - Popup GUI component
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

#ifndef _POPUP_H
#define _POPUP_H

#include "gui.h"

typedef enum {
  BOX_MAP = 1,
  BOX_MAP_OVERLAY, /* UNUSED */
  BOX_MINE_BUILDING,
  BOX_BASIC_BLD,
  BOX_BASIC_BLD_FLIP,
  BOX_ADV_1_BLD,
  BOX_ADV_2_BLD,
  BOX_STAT_SELECT,
  BOX_STAT_4,
  BOX_STAT_BLD_1,
  BOX_STAT_BLD_2,
  BOX_STAT_BLD_3,
  BOX_STAT_BLD_4,
  BOX_STAT_8,
  BOX_STAT_7,
  BOX_STAT_1,
  BOX_STAT_2,
  BOX_STAT_6,
  BOX_STAT_3,
  BOX_START_ATTACK,
  BOX_START_ATTACK_REDRAW,
  BOX_GROUND_ANALYSIS,
  BOX_LOAD_ARCHIVE,
  BOX_LOAD_SAVE,
  BOX_25,
  BOX_DISK_MSG,
  BOX_SETT_SELECT,
  BOX_SETT_1,
  BOX_SETT_2,
  BOX_SETT_3,
  BOX_KNIGHT_LEVEL,
  BOX_SETT_4,
  BOX_SETT_5,
  BOX_QUIT_CONFIRM,
  BOX_NO_SAVE_QUIT_CONFIRM,
  BOX_SETT_SELECT_FILE, /* UNUSED */
  BOX_OPTIONS,
  BOX_CASTLE_RES,
  BOX_MINE_OUTPUT,
  BOX_ORDERED_BLD,
  BOX_DEFENDERS,
  BOX_TRANSPORT_INFO,
  BOX_CASTLE_SERF,
  BOX_RESDIR,
  BOX_SETT_8,
  BOX_SETT_6,
  BOX_BLD_1,
  BOX_BLD_2,
  BOX_BLD_3,
  BOX_BLD_4,
  BOX_MESSAGE,
  BOX_BLD_STOCK,
  BOX_PLAYER_FACES,
  BOX_GAME_END,
  BOX_DEMOLISH,
  BOX_JS_CALIB,
  BOX_JS_CALIB_UPLEFT,
  BOX_JS_CALIB_DOWNRIGHT,
  BOX_JS_CALIB_CENTER,
  BOX_CTRLS_INFO
} box_t;

class interface_t;
class game_minimap_t;

class popup_box_t
  : public gui_object_t
{
protected:
  interface_t *interface;
  game_minimap_t *minimap;

  box_t box;

  int current_stat_8_mode;
  int current_stat_7_item;

public:
  popup_box_t(interface_t *interface);
  virtual ~popup_box_t();

  game_minimap_t *get_minimap() { return minimap; }
  box_t get_box() { return box; }
  void set_box(box_t box);

protected:
  virtual void internal_draw();

  void draw_transport_info_box(frame_t *frame);
  void draw_mine_building_box(frame_t *frame);
  void draw_basic_building_box(frame_t *frame, int flip);
  void draw_adv_2_building_box(frame_t *frame);
  void draw_stat_4_box(frame_t *frame);
  void draw_stat_bld_1_box(frame_t *frame);
  void draw_stat_bld_2_box(frame_t *frame);
  void draw_stat_bld_3_box(frame_t *frame);
  void draw_stat_bld_4_box(frame_t *frame);
  void draw_stat_8_box(frame_t *frame);
  void draw_stat_7_box(frame_t *frame);
  void draw_stat_1_box(frame_t *frame);
  void draw_stat_2_box(frame_t *frame);
  void draw_stat_6_box(frame_t *frame);
  void draw_stat_3_box(frame_t *frame);
  void draw_start_attack_redraw_box(frame_t *frame);
  void draw_start_attack_box(frame_t *frame);
  void draw_ground_analysis_box(frame_t *frame);
  void draw_sett_1_box(frame_t *frame);
  void draw_sett_2_box(frame_t *frame);
  void draw_sett_3_box(frame_t *frame);
  void draw_knight_level_box(frame_t *frame);
  void draw_sett_4_box(frame_t *frame);
  void draw_sett_5_box(frame_t *frame);
  void draw_options_box(frame_t *frame);
  void draw_castle_res_box(frame_t *frame);
  void draw_mine_output_box(frame_t *frame);
  void draw_ordered_building_box(frame_t *frame);
  void draw_defenders_box(frame_t *frame);
  void draw_castle_serf_box(frame_t *frame);
  void draw_resdir_box(frame_t *frame);
  void draw_sett_8_box(frame_t *frame);
  void draw_sett_6_box(frame_t *frame);
  void draw_bld_1_box(frame_t *frame);
  void draw_building_stock_box(frame_t *frame);
  void draw_map_box(frame_t *frame);

  virtual int handle_click_left(int x, int y);
  void handle_action(int action, int x, int y);
  int handle_clickmap(int x, int y, const int clkmap[]);
  void handle_box_close_clk(int x, int y);
  void handle_box_options_clk(int x, int y);
  void handle_mine_building_clk(int x, int y);
  void handle_basic_building_clk(int x, int y, int flip);
  void handle_adv_1_building_clk(int x, int y);
  void handle_adv_2_building_clk(int x, int y);
  void handle_stat_select_click(int x, int y);
  void handle_stat_3_4_6_click(int x, int y);
  void handle_stat_bld_click(int x, int y);
  void handle_stat_8_click(int x, int y);
  void handle_stat_7_click(int x, int y);
  void handle_stat_1_2_click(int x, int y);
  void handle_start_attack_click(int x, int y);
  void handle_ground_analysis_clk(int x, int y);
  void handle_sett_select_clk(int x, int y);
  void handle_sett_1_click(int x, int y);
  void handle_sett_2_click(int x, int y);
  void handle_sett_3_click(int x, int y);
  void handle_knight_level_click(int x, int y);
  void handle_sett_4_click(int x, int y);
  void handle_sett_5_6_click(int x, int y);
  void handle_quit_confirm_click(int x, int y);
  void handle_no_save_quit_confirm_click(int x, int y);
  void handle_castle_res_clk(int x, int y);
  void handle_transport_info_clk(int x, int y);
  void handle_castle_serf_clk(int x, int y);
  void handle_resdir_clk(int x, int y);
  void handle_sett_8_click(int x, int y);
  void handle_message_clk(int x, int y);
  void handle_player_faces_click(int x, int y);
  void handle_box_demolish_clk(int x, int y);
  void handle_minimap_clk(int x, int y);
  void handle_box_bld_1(int x, int y);
  void handle_box_bld_2(int x, int y);
  void handle_box_bld_3(int x, int y);
  void handle_box_bld_4(int x, int y);
};

#endif /* !_POPUP_H */
