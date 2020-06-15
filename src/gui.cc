/*
 * gui.cc - Base functions for the GUI hierarchy
 *
 * Copyright (C) 2013-2019  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "src/gui.h"

#include <algorithm>
#include <memory>

#include "src/misc.h"
#include "src/audio.h"

// Get the resulting value from a click on a slider bar
int
gui_get_slider_click_value(int x) {
  return 1310 * clamp(0, x - 7, 50);
}

void
GuiObject::layout() {
}

GuiObject::GuiObject() {
  x = 0;
  y = 0;
  width = 0;
  height = 0;
  displayed = false;
  enabled = true;
  redraw = true;
  frame = nullptr;
  initialized = false;
}

GuiObject::~GuiObject() {
  delete_frame();
}

void
GuiObject::delete_frame() {
  if (frame != nullptr) {
    delete frame;
    frame = nullptr;
  }
}

void
GuiObject::close_float(PGuiObject obj) {
  EventLoop &event_loop = EventLoop::get_instance();
  event_loop.deferred_call([this, obj](void*){
    floats.remove(obj);
    on_float_closed(obj);
  }, nullptr);
}

void
GuiObject::draw(Frame *_frame) {
  if (!displayed) {
    return;
  }

  if (frame == nullptr) {
    frame = Graphics::get_instance().create_frame(width, height);
  }

  if (!initialized) {
    init();
    initialized = true;
  }

  if (redraw) {
    internal_draw();

    for (PGuiObject float_window : floats) {
      float_window->draw(frame);
    }

    redraw = false;
  }
  _frame->draw_frame(x, y, 0, 0, frame, width, height);
}

bool
GuiObject::handle_event(const Event *event) {
  if (!enabled || !displayed) {
    return false;
  }

  int event_x = event->x;
  int event_y = event->y;
  if (event->type == Event::TypeClick ||
      event->type == Event::TypeDoubleClick ||
      event->type == Event::TypeDrag) {
    event_x = event->x - x;
    event_y = event->y - y;
    if (event_x < 0 || event_y < 0 || event_x > width || event_y > height) {
      return false;
    }
  }

  Event internal_event;
  internal_event.type = event->type;
  internal_event.x = event_x;
  internal_event.y = event_y;
  internal_event.dx = event->dx;
  internal_event.dy = event->dy;
  internal_event.button = event->button;

  /* Find the corresponding float element if any */
  FloatList::reverse_iterator fl = floats.rbegin();
  for ( ; fl != floats.rend() ; ++fl) {
    bool result = (*fl)->handle_event(&internal_event);
    if (result != 0) {
      return result;
    }
  }

  bool result = false;
  switch (event->type) {
    case Event::TypeClick:
      if (event->button == Event::ButtonLeft) {
        result = handle_click_left(event_x, event_y);
      }
      break;
    case Event::TypeDrag:
      result = handle_drag(event->dx, event->dy);
      break;
    case Event::TypeDoubleClick:
      result = handle_dbl_click(event->x, event->y, event->button);
      break;
    case Event::TypeKeyPressed:
      result = handle_key_pressed(event->dx, event->dy);
      break;
    default:
      break;
  }

  return result;
}

void
GuiObject::move_to(int px, int py) {
  x = px;
  y = py;
  set_redraw();
}

void
GuiObject::get_position(int *px, int *py) {
  if (px != nullptr) {
    *px = x;
  }
  if (py != nullptr) {
    *py = y;
  }
}

void
GuiObject::set_size(int new_width, int new_height) {
  delete_frame();
  width = new_width;
  height = new_height;
  layout();
  set_redraw();
}

void
GuiObject::get_size(int *pwidth, int *pheight) {
  if (pwidth != nullptr) {
    *pwidth = width;
  }
  if (pheight != nullptr) {
    *pheight = height;
  }
}

void
GuiObject::set_displayed(bool _displayed) {
  displayed = _displayed;
  set_redraw();
}

void
GuiObject::set_enabled(bool enabled) {
  this->enabled = enabled;
}

void
GuiObject::set_redraw() {
  redraw = true;
  if (parent.lock()) {
    parent.lock()->set_redraw();
  }
}

bool
GuiObject::point_inside(int point_x, int point_y) {
  return (point_x >= x && point_y >= y &&
          point_x < x + width && point_y < y + height);
}

void
GuiObject::add_float(PGuiObject obj, int fx, int fy) {
  obj->move_to(fx, fy);
  obj->set_parent(shared_from_this());
  floats.push_back(obj);
  set_redraw();
}

void
GuiObject::del_float(PGuiObject obj) {
  obj->set_parent(nullptr);
  floats.remove(obj);
  set_redraw();
}

void
GuiObject::close() {
  if (parent.lock()) {
    parent.lock()->close_float(shared_from_this());
  }
}

void
GuiObject::play_sound(int sound) {
  Audio &audio = Audio::get_instance();
  Audio::PPlayer player = audio.get_sound_player();
  if (player) {
    Audio::PTrack t = player->play_track(sound);
  }
}
