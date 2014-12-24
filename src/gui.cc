/*
 * gui.c - Base functions for the GUI hierarchy
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

#include "gui.h"
#include "misc.h"

#include <cstdlib>

#include <cstdlib>

/* Get the resulting value from a click on a slider bar. */
int
gui_get_slider_click_value(int x)
{
  return 1310 * clamp(0, x - 7, 50);
}


void
gui_object_t::layout()
{
}

gui_object_t *gui_object_t::focused_object = NULL;

gui_object_t::gui_object_t()
{
  x = 0;
  y = 0;
  width = 0;
  height = 0;
  displayed = false;
  enabled = true;
  redraw = true;
  parent = NULL;
  frame = NULL;
  focused = false;
}

gui_object_t::~gui_object_t()
{
  delete_frame();
}

void
gui_object_t::delete_frame()
{
  if(frame != NULL) {
    delete frame;
    frame = NULL;
  }
}

void
gui_object_t::draw(frame_t *frame)
{
  if(!displayed) {
    return;
  }

  if(this->frame == NULL) {
    gfx_t *gfx = gfx_t::get_gfx();
    this->frame = gfx->create_frame(width, height);
  }
  if(redraw) {
    internal_draw();

    float_list_t::iterator fl = floats.begin();
    for( ; fl != floats.end() ; fl++) {
      (*fl)->draw(this->frame);
    }

    redraw = 0;
  }
  frame->draw_frame(x, y, 0, 0, this->frame, width, height);
}

bool
gui_object_t::handle_event(const event_t *event)
{
  if (!enabled || !displayed) {
    return false;
  }

  int event_x = event->x - x;
  int event_y = event->y - y;
  if (event_x < 0 || event_y < 0 ||
      event_x > width || event_y > height)
  {
    return false;
  }

  event_t internal_event;
  internal_event.type = event->type;
  internal_event.x = event_x;
  internal_event.y = event_y;
  internal_event.dx = event->dx;
  internal_event.dy = event->dy;
  internal_event.button = event->button;

  /* Find the corresponding float element if any */
  float_list_t::reverse_iterator fl = floats.rbegin();
  for( ; fl != floats.rend() ; fl++) {
    bool result = (*fl)->handle_event(&internal_event);
    if (result != 0) {
      return result;
    }
  }

  bool result = false;
  switch (event->type) {
    case EVENT_TYPE_CLICK:
      if (event->button == EVENT_BUTTON_LEFT) {
        result = handle_click_left(event_x, event_y);
      }
      break;
    case EVENT_TYPE_DRAG:
      result = handle_drag(event->dx, event->dy);
      break;
    case EVENT_TYPE_DBL_CLICK:
      result = handle_dbl_click(x, y, event->button);
      break;
    case EVENT_KEY_PRESSED:
      result = handle_key_pressed(event->dx, event->dy);
      break;
    default:
      break;
  }

  if ((result != 0) && (focused_object != NULL)) {
    if (focused_object != this) {
      focused_object->handle_focus_loose();
      focused_object = NULL;
    }
  }

  return result;
}

void
gui_object_t::move_to(int x, int y)
{
  this->x = x;
  this->y = y;
  set_redraw();
}

void
gui_object_t::get_position(int &x, int &y)
{
  x = this->x;
  y = this->y;
}

void
gui_object_t::set_size(int width, int height)
{
  delete_frame();
  this->width = width;
  this->height = height;
  layout();
  set_redraw();
}

void
gui_object_t::get_size(int &width, int &height)
{
  width = this->width;
  height = this->height;
}

void
gui_object_t::set_displayed(bool displayed)
{
  this->displayed = displayed;
  set_redraw();
}

void
gui_object_t::set_enabled(bool enabled)
{
  this->enabled = enabled;
}

void
gui_object_t::set_redraw()
{
  redraw = true;
  if (parent != NULL) {
    parent->set_redraw();
  }
}

bool
gui_object_t::point_inside(int point_x, int point_y)
{
  return (point_x >= x && point_y >= y &&
          point_x < x + width &&
          point_y < y + height);
}

void
gui_object_t::add_float(gui_object_t *obj,
                        int x, int y)
{
  obj->set_parent(this);
  floats.push_back(obj);
  obj->move_to(x, y);
  set_redraw();
}
