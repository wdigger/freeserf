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

gui_object_t::gui_object_t()
{
  width = 0;
  height = 0;
  displayed = false;
  enabled = true;
  redraw = true;
  parent = NULL;
  frame = NULL;
}

gui_object_t::~gui_object_t()
{
  delete_frame();
}

void
gui_object_t::delete_frame()
{
  if(frame != NULL) {
    gfx_frame_destroy(frame);
    frame = NULL;
  }
}

void
gui_object_t::draw(frame_t *frame, int x, int y)
{
  if(!displayed) {
    return;
  }

  if(this->frame == NULL) {
    this->frame = gfx_frame_create(width, height);
  }
  if(redraw) {
    internal_draw();
    redraw = 0;
  }
  gfx_draw_frame(x, y, frame, 0, 0, this->frame, width, height);
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
  delete_frame();
  this->width = width;
  this->height = height;
  layout();
  set_redraw();
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
}

bool
gui_object_t::point_inside(int x, int y, int point_x, int point_y)
{
  return (point_x >= x && point_y >= y &&
          point_x < x + width &&
          point_y < y + height);
}

gui_container_t::gui_container_t()
  : gui_object_t()
{
}

int
gui_container_t::get_child_position(gui_object_t *child,
         int *x, int *y)
{
  return internal_get_child_position(child, x, y);
}
