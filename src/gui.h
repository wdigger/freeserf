/*
 * gui.h - Base functions for the GUI hierarchy
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

#ifndef _GUI_H
#define _GUI_H

#ifndef _MSC_VER
extern "C" {
#endif
  #include "gfx.h"
#ifndef _MSC_VER
}
#endif

typedef enum {
  GUI_EVENT_TYPE_CLICK,
  GUI_EVENT_TYPE_DBL_CLICK,
  GUI_EVENT_TYPE_BUTTON_UP,
  GUI_EVENT_TYPE_BUTTON_DOWN,
  GUI_EVENT_TYPE_DRAG_START,
  GUI_EVENT_TYPE_DRAG_MOVE,
  GUI_EVENT_TYPE_DRAG_END
} gui_event_type_t;

typedef enum {
  GUI_EVENT_BUTTON_LEFT = 1,
  GUI_EVENT_BUTTON_MIDDLE,
  GUI_EVENT_BUTTON_RIGHT
} gui_event_button_t;

typedef struct {
  gui_event_type_t type;
  int x, y;
  int button;
} gui_event_t;

class gui_container_t;

class gui_object_t
{
public:
  int width, height;
  int displayed;
  int enabled;
  int redraw;
  gui_container_t *parent;

  gui_object_t();

  virtual void draw(frame_t *frame) = 0;
  virtual int handle_event(const gui_event_t *event) = 0;
  virtual void set_size(int width, int height);

  void gui_object_redraw(frame_t *frame);
  int gui_object_handle_event(const gui_event_t *event);
  void gui_object_set_size(int width, int height);
  void gui_object_set_displayed(int displayed);
  void gui_object_set_enabled(int enabled);
  void gui_object_set_redraw();
};

class gui_container_t
  : public gui_object_t
{
public:
  gui_container_t();

  virtual void set_redraw_child(gui_object_t *child);
  virtual int get_child_position(gui_object_t *child, int *x, int *t) = 0;

  void gui_container_set_redraw_child(gui_object_t *child);
  int gui_container_get_child_position(gui_object_t *child, int *x, int *y);
};

int gui_get_slider_click_value(int x);

#endif /* !_GUI_H */
