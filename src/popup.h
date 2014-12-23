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
class minimap_t;

class popup_box_t
  : public gui_container_t
{
public:
  interface_t *interface;
  minimap_t *minimap;

  box_t box;

  popup_box_t(interface_t *interface);

  virtual void draw(frame_t *frame);
  virtual int handle_event(const gui_event_t *event);
  virtual int get_child_position(gui_object_t *child, int *x, int *t);
};

#endif /* !_POPUP_H */
