/*
 * minimap.c - Minimap GUI component
 *
 * Copyright (C) 2013-2014  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "minimap.h"
#include "interface.h"
#include "viewport.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "gfx.h"
  #include "game.h"
  #include "data.h"
#ifndef _MSC_VER
}
#endif

#define MINIMAP_MAX_SCALE  8

void
minimap_t::draw_minimap_point(int col, int row, uint8_t color,
       int density, frame_t *frame)
{
  int map_width = game.map.cols * scale;
  int map_height = game.map.rows * scale;

  if (0 == map_width || 0 == map_height) {
    return;
  }

  int mm_y = row*scale - offset_y;
  col -= (game.map.rows/2) * (int)(mm_y / map_height);
  mm_y = mm_y % map_height;

  while (mm_y < height) {
    if (mm_y >= -density) {
      int mm_x = col*scale -
        (row*scale)/2 - offset_x;
      mm_x = mm_x % map_width;
      while (mm_x < width) {
        if (mm_x >= -density) {
          gfx_fill_rect(mm_x, mm_y, density,
                  density, color, frame);
        }
        mm_x += map_width;
      }
    }
    col -= game.map.rows/2;
    mm_y += map_height;
  }
}

void
minimap_t::draw_minimap_map(frame_t *frame)
{
  uint8_t *color_data = game.minimap;
  for (uint row = 0; row < game.map.rows; row++) {
    for (uint col = 0; col < game.map.cols; col++) {
      uint8_t color = *(color_data++);
      draw_minimap_point(col, row, color, scale, frame);
    }
  }
}

void
minimap_t::draw_minimap_ownership(int density, frame_t *frame)
{
  for (uint row = 0; row < game.map.rows; row++) {
    for (uint col = 0; col < game.map.cols; col++) {
      map_pos_t pos = MAP_POS(col, row);
      if (MAP_HAS_OWNER(pos)) {
        int color = game.player[MAP_OWNER(pos)]->color;
        draw_minimap_point(col, row, color, density, frame);
      }
    }
  }
}

void
minimap_t::draw_minimap_roads(frame_t *frame)
{
  for (uint row = 0; row < game.map.rows; row++) {
    for (uint col = 0; col < game.map.cols; col++) {
      int pos = MAP_POS(col, row);
      if (MAP_PATHS(pos)) {
        draw_minimap_point(col, row, 1, scale, frame);
      }
    }
  }
}

void
minimap_t::draw_minimap_buildings(frame_t *frame)
{
  const int building_remap[] = {
    BUILDING_CASTLE,
    BUILDING_STOCK, BUILDING_TOWER, BUILDING_HUT,
    BUILDING_FORTRESS, BUILDING_TOOLMAKER, BUILDING_SAWMILL,
    BUILDING_WEAPONSMITH, BUILDING_STONECUTTER, BUILDING_BOATBUILDER,
    BUILDING_FORESTER, BUILDING_LUMBERJACK, BUILDING_PIGFARM,
    BUILDING_FARM, BUILDING_FISHER, BUILDING_MILL, BUILDING_BUTCHER,
    BUILDING_BAKER, BUILDING_STONEMINE, BUILDING_COALMINE,
    BUILDING_IRONMINE, BUILDING_GOLDMINE, BUILDING_STEELSMELTER,
    BUILDING_GOLDSMELTER
  };

  for (uint row = 0; row < game.map.rows; row++) {
    for (uint col = 0; col < game.map.cols; col++) {
      int pos = MAP_POS(col, row);
      int obj = MAP_OBJ(pos);
      if (obj > MAP_OBJ_FLAG && obj <= MAP_OBJ_CASTLE) {
        int color = game.player[MAP_OWNER(pos)]->color;
        if (advanced > 0) {
          building_t *bld = game_get_building(MAP_OBJ_INDEX(pos));
          if (BUILDING_TYPE(bld) == building_remap[advanced]) {
            draw_minimap_point(col, row, color, scale, frame);
          }
        } else {
          draw_minimap_point(col, row, color, scale, frame);
        }
      }
    }
  }
}

void
minimap_t::draw_minimap_traffic(frame_t *frame)
{
  for (uint row = 0; row < game.map.rows; row++) {
    for (uint col = 0; col < game.map.cols; col++) {
      int pos = MAP_POS(col, row);
      if (MAP_IDLE_SERF(pos)) {
        int color = game.player[MAP_OWNER(pos)]->color;
        draw_minimap_point(col, row, color, scale, frame);
      }
    }
  }
}

void
minimap_t::draw_minimap_grid(frame_t *frame)
{
  for (uint y = 0; y < game.map.rows * scale; y += 2) {
    draw_minimap_point(0, y, 47, 1, frame);
    draw_minimap_point(0, y+1, 1, 1, frame);
  }

  for (uint x = 0; x < game.map.cols * scale; x += 2) {
    draw_minimap_point(x, 0, 47, 1, frame);
    draw_minimap_point(x+1, 0, 1, 1, frame);
  }
}

void
minimap_t::draw_minimap_rect(frame_t *frame)
{
  int y = height/2;
  int x = width/2;
  gfx_draw_transp_sprite(x, y, 354, 1, 0, 0, frame);
}

void
minimap_t::internal_draw()
{
  if (BIT_TEST(flags, 1)) {
    gfx_fill_rect(0, 0, 128, 128, 1, frame);
    draw_minimap_ownership(2, frame);
  } else {
    draw_minimap_map(frame);
    if (BIT_TEST(flags, 0)) {
      draw_minimap_ownership(1, frame);
    }
  }

  if (BIT_TEST(flags, 2)) {
    draw_minimap_roads(frame);
  }

  if (BIT_TEST(flags, 3)) {
    draw_minimap_buildings(frame);
  }

  if (BIT_TEST(flags, 4)) {
    draw_minimap_grid(frame);
  }

  if (advanced > 0) {
    draw_minimap_traffic(frame);
  }

  draw_minimap_rect(frame);
}

int
minimap_t::handle_event_click(int x, int y)
{
  map_pos_t pos = map_pos_from_screen_pix(x, y);
  interface->get_viewport()->move_to_map_pos(pos);

  interface->update_map_cursor_pos(pos);
  interface->close_popup();

  return 0;
}

int
minimap_t::handle_scroll(int up)
{
  int scale = 0;
  if (up) scale = scale + 1;
  else scale = scale - 1;

  scale = clamp(1, scale, MINIMAP_MAX_SCALE);
  return 0;
}

static int
minimap_handle_drag(minimap_t *minimap, int x, int y,
        gui_event_button_t button)
{
  if (button == GUI_EVENT_BUTTON_RIGHT) {
    if (x != 0 || y != 0) {
      minimap->move_by_pixels(x, y);
    }
  }

  return 0;
}

int
minimap_t::internal_handle_event(const gui_event_t *event)
{
  int x = event->x;
  int y = event->y;

  switch (event->type) {
  case GUI_EVENT_TYPE_CLICK:
    if (event->button == GUI_EVENT_BUTTON_LEFT) {
      return handle_event_click(x, y);
    }
    break;
  case GUI_EVENT_TYPE_BUTTON_UP:
    if (event->button == 4 || event->button == 5) {
      return handle_scroll(event->button == 4);
    }
    break;
  case GUI_EVENT_TYPE_DRAG_MOVE:
    return minimap_handle_drag(this, x, y,
      (gui_event_button_t)event->button);
  case GUI_EVENT_TYPE_DRAG_START:
    interface->set_cursor_lock_target(this);
    return 0;
  case GUI_EVENT_TYPE_DRAG_END:
    interface->set_cursor_lock_target(NULL);
    return 0;
  default:
    break;
  }

  return 0;
}

minimap_t::minimap_t(interface_t *interface)
  : gui_object_t()
{
  this->interface = interface;
  offset_x = 0;
  offset_y = 0;
  scale = 1;

  advanced = -1;
  flags = 8;
}

/* Set the scale of the map (zoom). Must be positive. */
void
minimap_t::set_scale(int scale)
{
  map_pos_t pos = get_current_map_pos();
  this->scale = scale;
  move_to_map_pos(pos);

  set_redraw();
}

void
minimap_t::set_advanced(int advanced)
{
  this->advanced = advanced;

  set_redraw();
}

void
minimap_t::set_flags(int flags)
{
  this->flags |= flags;

  set_redraw();
}

void
minimap_t::clear_flags(int flags)
{
  this->flags &= ~flags;

  set_redraw();
}

void
minimap_t::invert_flags(int flags)
{
  BIT_INVERT(this->flags, flags);
}

void
minimap_t::map_pix_from_map_coord(map_pos_t pos, int *mx, int *my)
{
  int width = game.map.cols * scale;
  int height = game.map.rows * scale;

  *mx = scale*MAP_POS_COL(pos) - (scale*MAP_POS_ROW(pos))/2;
  *my = scale*MAP_POS_ROW(pos);

  if (*my < 0) {
    *mx -= height/2;
    *my += height;
  }

  if (*mx < 0) *mx += width;
  else if (*mx >= width) *mx -= width;
}

map_pos_t
minimap_t::map_pos_from_screen_pix(int x, int y)
{
  int mx = x + offset_x;
  int my = y + offset_y;

  int col = ((my/2 + mx)/scale) & game.map.col_mask;
  int row = (my/scale) & game.map.row_mask;

  return MAP_POS(col, row);
}

map_pos_t
minimap_t::get_current_map_pos()
{
  return map_pos_from_screen_pix(width/2, height/2);
}

void
minimap_t::move_to_map_pos(map_pos_t pos)
{
  int mx, my;
  map_pix_from_map_coord(pos, &mx, &my);

  int map_width = game.map.cols*scale;
  int map_height = game.map.rows*scale;

  /* Center view */
  mx -= width/2;
  my -= height/2;

  if (my < 0) {
    mx -= map_height/2;
    my += map_height;
  }

  if (mx < 0) mx += map_width;
  else if (mx >= map_width) mx -= map_width;

  offset_x = mx;
  offset_y = my;

  set_redraw();
}

void
minimap_t::move_by_pixels(int dx, int dy)
{
  int width = game.map.cols * scale;
  int height = game.map.rows * scale;

  offset_x += dx;
  offset_y += dy;

  if (offset_y < 0) {
    offset_y += height;
    offset_x -= height/2;
  } else if (offset_y >= height) {
    offset_y -= height;
    offset_x += height/2;
  }

  if (offset_x >= width) offset_x -= width;
  else if (offset_x < 0) offset_x += width;

  set_redraw();
}
