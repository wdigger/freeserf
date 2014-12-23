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

#include <stdlib.h>

#define MINIMAP_MAX_SCALE  8

static void
draw_minimap_point(minimap_t *minimap, int col, int row, uint8_t color,
       int density, frame_t *frame)
{
  int map_width = game.map.cols * minimap->scale;
  int map_height = game.map.rows * minimap->scale;

  if (0 == map_width || 0 == map_height) {
    return;
  }

  int mm_y = row*minimap->scale - minimap->offset_y;
  col -= (game.map.rows/2) * (int)(mm_y / map_height);
  mm_y = mm_y % map_height;

  while (mm_y < minimap->height) {
    if (mm_y >= -density) {
      int mm_x = col*minimap->scale -
        (row*minimap->scale)/2 - minimap->offset_x;
      mm_x = mm_x % map_width;
      while (mm_x < minimap->width) {
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

static void
draw_minimap_map(minimap_t *minimap, frame_t *frame)
{
  uint8_t *color_data = game.minimap;
  for (uint row = 0; row < game.map.rows; row++) {
    for (uint col = 0; col < game.map.cols; col++) {
      uint8_t color = *(color_data++);
      draw_minimap_point(minimap, col, row, color,
             minimap->scale, frame);
    }
  }
}

static void
draw_minimap_ownership(minimap_t *minimap, int density, frame_t *frame)
{
  for (uint row = 0; row < game.map.rows; row++) {
    for (uint col = 0; col < game.map.cols; col++) {
      map_pos_t pos = MAP_POS(col, row);
      if (MAP_HAS_OWNER(pos)) {
        int color = game.player[MAP_OWNER(pos)]->color;
        draw_minimap_point(minimap, col, row, color,
               density, frame);
      }
    }
  }
}

static void
draw_minimap_roads(minimap_t *minimap, frame_t *frame)
{
  for (uint row = 0; row < game.map.rows; row++) {
    for (uint col = 0; col < game.map.cols; col++) {
      int pos = MAP_POS(col, row);
      if (MAP_PATHS(pos)) {
        draw_minimap_point(minimap, col, row, 1,
               minimap->scale, frame);
      }
    }
  }
}

static void
draw_minimap_buildings(minimap_t *minimap, frame_t *frame)
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
        if (minimap->advanced > 0) {
          building_t *bld = game_get_building(MAP_OBJ_INDEX(pos));
          if (BUILDING_TYPE(bld) == building_remap[minimap->advanced]) {
            draw_minimap_point(minimap, col, row, color,
                   minimap->scale, frame);
          }
        } else {
          draw_minimap_point(minimap, col, row, color,
                 minimap->scale, frame);
        }
      }
    }
  }
}

static void
draw_minimap_traffic(minimap_t *minimap, frame_t *frame)
{
  for (uint row = 0; row < game.map.rows; row++) {
    for (uint col = 0; col < game.map.cols; col++) {
      int pos = MAP_POS(col, row);
      if (MAP_IDLE_SERF(pos)) {
        int color = game.player[MAP_OWNER(pos)]->color;
        draw_minimap_point(minimap, col, row, color,
               minimap->scale, frame);
      }
    }
  }
}

static void
draw_minimap_grid(minimap_t *minimap, frame_t *frame)
{
  for (uint y = 0; y < game.map.rows * minimap->scale; y += 2) {
    draw_minimap_point(minimap, 0, y, 47, 1, frame);
    draw_minimap_point(minimap, 0, y+1, 1, 1, frame);
  }

  for (uint x = 0; x < game.map.cols * minimap->scale; x += 2) {
    draw_minimap_point(minimap, x, 0, 47, 1, frame);
    draw_minimap_point(minimap, x+1, 0, 1, 1, frame);
  }
}

static void
draw_minimap_rect(minimap_t *minimap, frame_t *frame)
{
  int y = minimap->height/2;
  int x = minimap->width/2;
  gfx_draw_transp_sprite(x, y, 354, 1, 0, 0, frame);
}

void
minimap_t::internal_draw(frame_t *frame)
{
  if (BIT_TEST(flags, 1)) {
    gfx_fill_rect(0, 0, 128, 128, 1, frame);
    draw_minimap_ownership(this, 2, frame);
  } else {
    draw_minimap_map(this, frame);
    if (BIT_TEST(flags, 0)) {
      draw_minimap_ownership(this, 1, frame);
    }
  }

  if (BIT_TEST(flags, 2)) {
    draw_minimap_roads(this, frame);
  }

  if (BIT_TEST(flags, 3)) {
    draw_minimap_buildings(this, frame);
  }

  if (BIT_TEST(flags, 4)) {
    draw_minimap_grid(this, frame);
  }

  if (advanced > 0) {
    draw_minimap_traffic(this, frame);
  }

  draw_minimap_rect(this, frame);
}

static int
minimap_handle_event_click(minimap_t *minimap, int x, int y)
{
  map_pos_t pos = minimap->map_pos_from_screen_pix(x, y);
  minimap->interface->get_top_viewport()->move_to_map_pos(pos);

  minimap->interface->update_map_cursor_pos(pos);
  minimap->interface->close_popup();

  return 0;
}

static int
minimap_handle_scroll(minimap_t *minimap, int up)
{
  int scale = 0;
  if (up) scale = minimap->scale + 1;
  else scale = minimap->scale - 1;

  minimap->set_scale(clamp(1, scale, MINIMAP_MAX_SCALE));
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
      return minimap_handle_event_click(this, x, y);
    }
    break;
  case GUI_EVENT_TYPE_BUTTON_UP:
    if (event->button == 4 || event->button == 5) {
      return minimap_handle_scroll(this, event->button == 4);
    }
    break;
  case GUI_EVENT_TYPE_DRAG_MOVE:
    return minimap_handle_drag(this, x, y,
      (gui_event_button_t)event->button);
  case GUI_EVENT_TYPE_DRAG_START:
    interface->cursor_lock_target = this;
    return 0;
  case GUI_EVENT_TYPE_DRAG_END:
    interface->cursor_lock_target = NULL;
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
minimap_screen_pix_from_map_pix(minimap_t *minimap, int mx, int my, int *sx, int *sy)
{
  int width = game.map.cols * minimap->scale;
  int height = game.map.rows * minimap->scale;

  *sx = mx - minimap->offset_x;
  *sy = my - minimap->offset_y;

  while (*sy < 0) {
    *sx -= height/2;
    *sy += height;
  }

  while (*sy >= height) {
    *sx += height/2;
    *sy -= height;
  }

  while (*sx < 0) *sx += width;
  while (*sx >= width) *sx -= width;
}

void
minimap_map_pix_from_map_coord(minimap_t *minimap, map_pos_t pos, int *mx, int *my)
{
  int width = game.map.cols * minimap->scale;
  int height = game.map.rows * minimap->scale;

  *mx = minimap->scale*MAP_POS_COL(pos) - (minimap->scale*MAP_POS_ROW(pos))/2;
  *my = minimap->scale*MAP_POS_ROW(pos);

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
  minimap_map_pix_from_map_coord(this, pos, &mx, &my);

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
