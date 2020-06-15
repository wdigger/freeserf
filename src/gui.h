/*
 * gui.h - Base functions for the GUI hierarchy
 *
 * Copyright (C) 2012-2017  Jon Lund Steffensen <jonlst@gmail.com>
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

#ifndef SRC_GUI_H_
#define SRC_GUI_H_

#include <list>
#include <memory>

#include "src/gfx.h"
#include "src/event_loop.h"

class GuiObject;
typedef std::shared_ptr<GuiObject> PGuiObject;

class GuiObject : public EventLoop::Handler,
                  public std::enable_shared_from_this<GuiObject> {
 private:
  typedef std::list<PGuiObject> FloatList;
  FloatList floats;

 protected:
  int x, y;
  int width, height;
  bool displayed;
  bool enabled;
  bool redraw;
  std::weak_ptr<GuiObject> parent;
  Frame *frame;
  bool initialized;

  virtual void init() {}
  virtual void internal_draw() = 0;
  virtual void layout();

  virtual bool handle_click_left(int x, int y) { return false; }
  virtual bool handle_dbl_click(int x, int y, Event::Button button) {
    return false; }
  virtual bool handle_drag(int dx, int dy) { return true; }
  virtual bool handle_key_pressed(char key, int modifier) { return false; }
  virtual void on_float_closed(PGuiObject obj) {}

  void delete_frame();

  void close_float(PGuiObject obj);

 public:
  GuiObject();
  virtual ~GuiObject();

  void draw(Frame *frame);
  void move_to(int x, int y);
  void get_position(int *x, int *y);
  void set_size(int width, int height);
  void get_size(int *width, int *height);
  void set_displayed(bool displayed);
  void set_enabled(bool enabled);
  void set_redraw();
  bool is_displayed() { return displayed; }
  PGuiObject get_parent() { return parent.lock(); }
  virtual void set_parent(std::shared_ptr<GuiObject> _parent) {
    parent = _parent;
  }
  bool point_inside(int point_x, int point_y);

  void add_float(PGuiObject obj, int x, int y);
  void del_float(PGuiObject obj);
  void close();

  virtual bool handle_event(const Event *event);

  void play_sound(int sound);
};

int gui_get_slider_click_value(int x);

#endif  // SRC_GUI_H_
