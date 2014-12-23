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

#ifndef _MSC_VER
extern "C" {
#endif
  #include "misc.h"
#ifndef _MSC_VER
}
#endif

#include <stdlib.h>

/* Get the resulting value from a click on a slider bar. */
int
gui_get_slider_click_value(int x)
{
  return 1310 * clamp(0, x - 7, 50);
}


void
gui_object_t::internal_set_size(int width, int height)
{
  this->width = width;
  this->height = height;
}

gui_object_t::gui_object_t()
{
  width = 0;
  height = 0;
  displayed = 0;
  enabled = 1;
  redraw = 0;
  parent = NULL;
  frame = NULL;
}

void
gui_object_t::draw(frame_t *frame, int x, int y)
{
  if(this->frame == NULL) {
    this->frame = gfx_frame_create(width, height);
  }
  internal_draw();
  gfx_draw_frame(x, y, frame, 0, 0, this->frame, width, height);
  redraw = 0;
}

int
gui_object_t::handle_event(const gui_event_t *event)
{
  if (!enabled) return 0;
  return internal_handle_event(event);
}

void
gui_object_t::set_size(int width, int height)
{
  if(frame != NULL) {
    gfx_frame_destroy(frame);
    frame = NULL;
  }
  internal_set_size(width, height);
}

void
gui_object_t::set_displayed(bool displayed)
{
  this->displayed = displayed;
  if (displayed) {
    set_redraw();
  } else if (parent != NULL) {
    parent->set_redraw();
  }
}

void
gui_object_t::set_enabled(bool enabled)
{
  this->enabled = enabled;
}

void
gui_object_t::set_redraw()
{
  redraw = 1;
  if (parent != NULL) {
    parent->set_redraw_child(this);
  }
}

bool
gui_object_t::point_inside(int x, int y, int point_x, int point_y)
{
  return (point_x >= x && point_y >= y &&
          point_x < x + width &&
          point_y < y + height);
}

void
gui_container_t::internal_set_redraw_child(gui_object_t *child)
{
  set_redraw();
}

gui_container_t::gui_container_t()
  : gui_object_t()
{
}

void
gui_container_t::set_redraw_child(gui_object_t *child)
{
  internal_set_redraw_child(child);
}

int
gui_container_t::get_child_position(gui_object_t *child,
         int *x, int *y)
{
  return internal_get_child_position(child, x, y);
}
