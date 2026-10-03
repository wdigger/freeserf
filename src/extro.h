/*
 * extro.h - End sequence after the last mission
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

#ifndef SRC_EXTRO_H_
#define SRC_EXTRO_H_

#include "src/gui.h"

class interface_t;

/* The landscape scrolls from left to right, then the closing text is
   shown (Amiga play_extro @0x1db1a). The view is 320x200 pixels in the
   middle of the screen. */
class extro_t : public gui_object_t {
 protected:
  interface_t *interface;
  frame_t *view;
  int counter;

 public:
  explicit extro_t(interface_t *interface);
  virtual ~extro_t();

  void start();
  void step();

 protected:
  virtual void internal_draw();
  virtual bool handle_click_left(int x, int y) { return true; }
  virtual bool handle_click_right(int x, int y) { return true; }
  virtual bool handle_dbl_click(int x, int y, event_button_t button) {
    return true; }
  virtual bool handle_drag(int dx, int dy) { return true; }
  virtual bool handle_key_pressed(char key, int modifier) { return true; }
};

#endif  // SRC_EXTRO_H_
