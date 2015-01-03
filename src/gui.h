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

#include <list>

typedef enum {
  GUI_EVENT_TYPE_CLICK,
  GUI_EVENT_TYPE_DBL_CLICK,
  GUI_EVENT_TYPE_BUTTON_UP,
  GUI_EVENT_TYPE_BUTTON_DOWN,
  GUI_EVENT_TYPE_DRAG_START,
  GUI_EVENT_TYPE_DRAG_MOVE,
  GUI_EVENT_TYPE_DRAG_END,
  GUI_EVENT_KEY_PRESSED,
} gui_event_type_t;

typedef enum {
  GUI_EVENT_BUTTON_LEFT = 1,
  GUI_EVENT_BUTTON_MIDDLE,
  GUI_EVENT_BUTTON_RIGHT
} gui_event_button_t;

typedef struct {
  gui_event_type_t type;
  int x, y;
  int dx, dy;
  gui_event_button_t button;
} gui_event_t;

class gui_object_t
{
private:
  typedef std::list<gui_object_t*> float_list_t;
  float_list_t floats;

protected:
  int x, y;
  int width, height;
  bool displayed;
  bool enabled;
  bool redraw;
  gui_object_t *parent;
  frame_t *frame;

  virtual void internal_draw() = 0;
  virtual void layout();

  virtual int handle_click_left(int x, int y) { return 0; }
  virtual int handle_dbl_click(int x, int y, gui_event_button_t button) { return 0; }
  virtual int handle_drag(int dx, int dy) { return 0; }
  virtual int handle_key_pressed(char key, int modifier) { return 0; }

  void delete_frame();

public:
  gui_object_t();
  virtual ~gui_object_t();

  void draw(frame_t *frame);
  int handle_event(const gui_event_t *event);
  void move_to(int x, int y);
  void get_position(int &x, int &y);
  void set_size(int width, int height);
  void get_size(int &width, int &height);
  void set_displayed(bool displayed);
  void set_enabled(bool enabled);
  void set_redraw();
  bool is_displayed() { return displayed; }
  gui_object_t *get_parent() { return parent; }
  void set_parent(gui_object_t *parent) { this->parent = parent; }
  bool point_inside(int point_x, int point_y);

  void add_float(gui_object_t *obj, int x, int y);
};

int gui_get_slider_click_value(int x);

#endif /* !_GUI_H */
