/*
 * panel.h - Panel GUI component
 *
 * Copyright (C) 2012  Jon Lund Steffensen <jonlst@gmail.com>
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

#ifndef _PANEL_H
#define _PANEL_H

#include "gui.h"

typedef enum {
  PANEL_BTN_BUILD_INACTIVE = 0,
  PANEL_BTN_BUILD_FLAG,
  PANEL_BTN_BUILD_MINE,
  PANEL_BTN_BUILD_SMALL,
  PANEL_BTN_BUILD_LARGE,
  PANEL_BTN_BUILD_CASTLE,
  PANEL_BTN_DESTROY,
  PANEL_BTN_DESTROY_INACTIVE,
  PANEL_BTN_BUILD_ROAD,
  PANEL_BTN_MAP_INACTIVE,
  PANEL_BTN_MAP,
  PANEL_BTN_STATS_INACTIVE,
  PANEL_BTN_STATS,
  PANEL_BTN_SETT_INACTIVE,
  PANEL_BTN_SETT,
  PANEL_BTN_DESTROY_ROAD,
  PANEL_BTN_GROUND_ANALYSIS,
  PANEL_BTN_BUILD_SMALL_STARRED,
  PANEL_BTN_BUILD_LARGE_STARRED,
  PANEL_BTN_MAP_STARRED,
  PANEL_BTN_STATS_STARRED,
  PANEL_BTN_SETT_STARRED,
  PANEL_BTN_GROUND_ANALYSIS_STARRED,
  PANEL_BTN_BUILD_MINE_STARRED,
  PANEL_BTN_BUILD_ROAD_STARRED
} panel_btn_t;

class interface_t;

class panel_bar_t
  : public gui_object_t
{
public:
  interface_t *interface;

  panel_bar_t(interface_t *interface);

  virtual void draw(frame_t *frame);
  virtual int handle_event(const gui_event_t *event);

  void panel_bar_activate_button(int button);
};

#endif /* !_PANEL_H */
