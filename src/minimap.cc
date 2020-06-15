/*
 * minimap.cc - Minimap GUI component
 *
 * Copyright (C) 2013-2017  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "src/minimap.h"

#include <algorithm>
#include <utility>

#include "src/game.h"
#include "src/interface.h"
#include "src/viewport.h"

const int
Minimap::max_scale = 8;

Minimap::Minimap(unsigned int _width, unsigned int _height, PMap _map)
  : Control(_width, _height) {
  offset_x = 0;
  offset_y = 0;
  scale = 1;

  draw_grid = false;

  std::array<Color, 17> colors_water_0;
  colors_water_0.fill(Color(0x00, 0x00, 0xaf));

  std::array<Color, 17> colors_water_1;
  colors_water_0.fill(Color(0x07, 0x07, 0xb3));

  std::array<Color, 17> colors_water_2;
  colors_water_2.fill(Color(0x0b, 0x0b, 0xb7));

  std::array<Color, 17> colors_water_3;
  colors_water_3.fill(Color(0x13, 0x13, 0xbb));

  std::array<Color, 17> colors_grass =
                                     make_gradient<17>(Color(0x72, 0xb3, 0x42),
                                                       Color(0x1b, 0x33, 0x06));

  std::array<Color, 17> colors_desert =
                                     make_gradient<17>(Color(0xf0, 0xd1, 0xaf),
                                                       Color(0x7a, 0x4e, 0x32));

  std::array<Color, 17> colors_tundra =
                                     make_gradient<17>(Color(0xd6, 0xd2, 0x90),
                                                       Color(0x42, 0x22, 0x13));

  std::array<Color, 17> colors_snow = make_gradient<17>(Color::white,
                                                       Color(0x4d, 0x4d, 0x4d));

  palette = {
    {Map::TerrainWater0, colors_water_0},
    {Map::TerrainWater1, colors_water_1},
    {Map::TerrainWater2, colors_water_2},
    {Map::TerrainWater3, colors_water_3},
    {Map::TerrainGrass0, colors_grass},
    {Map::TerrainGrass1, colors_grass},
    {Map::TerrainGrass2, colors_grass},
    {Map::TerrainGrass3, colors_grass},
    {Map::TerrainDesert0, colors_desert},
    {Map::TerrainDesert1, colors_desert},
    {Map::TerrainDesert2, colors_desert},
    {Map::TerrainTundra0, colors_tundra},
    {Map::TerrainTundra1, colors_tundra},
    {Map::TerrainTundra2, colors_tundra},
    {Map::TerrainSnow0, colors_snow},
    {Map::TerrainSnow1, colors_snow}
  };

  set_map(_map);
}

void
Minimap::set_draw_grid(bool _draw_grid) {
  draw_grid = _draw_grid;
  invalidate();
}

template <size_t size> std::array<Color, size>
Minimap::make_gradient(Color color_start, Color color_end) {
  std::array<Color, size> result;

  double cs = color_start.get_cyan();
  double ms = color_start.get_magenta();
  double ys = color_start.get_yellow();
  double ks = color_start.get_key();

  double ce = color_end.get_cyan();
  double me = color_end.get_magenta();
  double ye = color_end.get_yellow();
  double ke = color_end.get_key();

  for (size_t i = 0; i < size; i++) {
    double n = static_cast<double>(i) / (size - 1.);
    double c = cs + ((ce - cs) * n);
    double m = ms + ((me - ms) * n);
    double y = ys + ((ye - ys) * n);
    double k = ks + ((ke - ks) * n);
    result[i] = Color(c, m, y, k, 0xff);
  }

  return result;
}

/* Initialize minimap data. */
void
Minimap::init_minimap() {
  if (map == NULL) {
    return;
  }

  minimap.clear();

  for (MapPos pos : map->geom()) {
    pos = map->move_right(pos);
    int h1 = map->get_height(pos);

    pos = map->move_left(map->move_down(pos));
    int h2 = map->get_height(pos);

    int h_off = h2 - h1 + 8;
    minimap.push_back(palette[map->type_up(pos)][h_off]);
  }
}

void
Minimap::draw_minimap_point(Frame *frame, unsigned int x, unsigned int y,
                            int col, int row,
                            const Color &color, int density) {
  int map_width = map->get_cols() * scale;
  int map_height = map->get_rows() * scale;

  if (0 == map_width || 0 == map_height) {
    return;
  }

  int mm_y = row * scale - offset_y;
  col -= (map->get_rows()/2) * static_cast<int>(mm_y / map_height);
  mm_y = mm_y % map_height;

  while (mm_y < static_cast<int>(height)) {
    if (mm_y >= 0) {
      int mm_x = col * scale - (row * scale) / 2 - offset_x;
      mm_x = mm_x % map_width;
      while (mm_x < static_cast<int>(width)) {
        if (mm_x >= 0) {
          frame->fill_rect(x + mm_x, y + mm_y, density, density, color);
        }
        mm_x += map_width;
      }
    }
    col -= map->get_rows()/2;
    mm_y += map_height;
  }
}

void
Minimap::draw_minimap_map(Frame *frame, unsigned int x, unsigned int y) {
  for (unsigned int row = 0; row < map->get_rows(); row++) {
    for (unsigned int col = 0; col < map->get_cols(); col++) {
      Color color = minimap[row * map->get_cols() + col];
      draw_minimap_point(frame, x, y, col, row, color, scale);
    }
  }
}

void
Minimap::draw_minimap_grid(Frame *frame, unsigned int _x, unsigned int _y) {
  for (unsigned int y = 0; y < map->get_rows() * scale; y += 2) {
    draw_minimap_point(frame, _x, _y, 0, y, Color::white, 1);
  }

  for (unsigned int x = 0; x < map->get_cols() * scale; x += 2) {
    draw_minimap_point(frame, _x, _y, x, 0, Color::white, 1);
  }
}

void
Minimap::draw_minimap_rect(Frame *frame, unsigned int _x, unsigned int _y) {
  int y = height/2;
  int x = width/2;
  frame->draw_sprite(_x + x, _y + y, Data::AssetGameObject, 33, true);
}

void
Minimap::draw(Frame *frame, unsigned int x, unsigned int y) {
  if (map == nullptr) {
    frame->fill_rect(x, y, width, height, Color::black);
    return;
  }

  draw_minimap_map(frame, x, y);

  if (draw_grid) {
    draw_minimap_grid(frame, x, y);
  }
}

int
Minimap::handle_scroll(int up) {
  int scale_ = 0;

  if (up) {
    scale_ = scale + 1;
  } else {
    scale_ = scale - 1;
  }

  set_scale(clamp(1, scale_, max_scale));

  return 0;
}

bool
Minimap::handle_drag(int dx, int dy) {
  if (dx != 0 || dy != 0) {
    move_by_pixels(dx, dy);
  }

  return true;
}

void
Minimap::set_map(PMap _map) {
  map = std::move(_map);
  init_minimap();
  invalidate();
}

/* Set the scale of the map (zoom). Must be positive. */
void
Minimap::set_scale(int scale) {
  MapPos pos = get_current_map_pos();
  this->scale = scale;
  move_to_map_pos(pos);

  invalidate();
}

void
Minimap::screen_pix_from_map_pix(int mx, int my, int *sx, int *sy) {
  int pwidth = map->get_cols() * scale;
  int pheight = map->get_rows() * scale;

  *sx = mx - offset_x;
  *sy = my - offset_y;

  while (*sy < 0) {
    *sx -= pheight / 2;
    *sy += pheight;
  }

  while (*sy >= pheight) {
    *sx += pheight / 2;
    *sy -= pheight;
  }

  while (*sx < 0) *sx += pwidth;
  while (*sx >= pwidth) *sx -= pwidth;
}

void
Minimap::map_pix_from_map_coord(MapPos pos, int *mx, int *my) {
  int pwidth = map->get_cols() * scale;
  int pheight = map->get_rows() * scale;

  *mx = scale * map->pos_col(pos) - (scale * map->pos_row(pos)) / 2;
  *my = scale * map->pos_row(pos);

  if (*my < 0) {
    *mx -= pheight / 2;
    *my += pheight;
  }

  if (*mx < 0) *mx += pwidth;
  else if (*mx >= pwidth) *mx -= pwidth;
}

MapPos
Minimap::map_pos_from_screen_pix(int sx, int sy) {
  int mx = sx + offset_x;
  int my = sy + offset_y;

  int col = ((my/2 + mx)/scale) & map->get_col_mask();
  int row = (my/scale) & map->get_row_mask();

  return map->pos(col, row);
}

MapPos
Minimap::get_current_map_pos() {
  return map_pos_from_screen_pix(width/2, height/2);
}

void
Minimap::move_to_map_pos(MapPos pos) {
  int mx, my;
  map_pix_from_map_coord(pos, &mx, &my);

  int map_width = map->get_cols()*scale;
  int map_height = map->get_rows()*scale;

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

  invalidate();
}

void
Minimap::move_by_pixels(int dx, int dy) {
  int pwidth = map->get_cols() * scale;
  int pheight = map->get_rows() * scale;

  offset_x += dx;
  offset_y += dy;

  if (offset_y < 0) {
    offset_y += pheight;
    offset_x -= pheight / 2;
  } else if (offset_y >= pheight) {
    offset_y -= pheight;
    offset_x += pheight / 2;
  }

  if (offset_x >= pwidth) offset_x -= pwidth;
  else if (offset_x < 0) offset_x += pwidth;

  invalidate();
}

MinimapGame::MinimapGame(unsigned int _width, unsigned int _height,
                         Interface *_interface, PGame _game)
  : Minimap(_width, _height, _game->get_map()) {
  interface = _interface;
  draw_roads = false;
  draw_buildings = true;
  ownership_mode = OwnershipModeNone;
  advanced = -1;
}

void
MinimapGame::set_ownership_mode(OwnershipMode _ownership_mode) {
  ownership_mode = _ownership_mode;
  invalidate();
}

void
MinimapGame::set_draw_roads(bool _draw_roads) {
  draw_roads = _draw_roads;
  invalidate();
}

void
MinimapGame::set_draw_buildings(bool _draw_buildings) {
  draw_buildings = _draw_buildings;
  invalidate();
}

void
MinimapGame::draw(Frame *frame, unsigned int x, unsigned int y) {
  switch (ownership_mode) {
    case OwnershipModeNone:
      draw_minimap_map(frame, x, y);
      break;
    case OwnershipModeMixed:
      draw_minimap_map(frame, x, y);
      draw_minimap_ownership(frame, x, y, 2);
      break;
    case OwnershipModeSolid:
      frame->fill_rect(x, y, width, height, Color::black);
      draw_minimap_ownership(frame, x, y, 1);
      break;
  }

  if (draw_roads) {
    draw_minimap_roads(frame, x, y);
  }

  if (draw_buildings) {
    draw_minimap_buildings(frame, x, y);
  }

  if (draw_grid) {
    draw_minimap_grid(frame, x, y);
  }

  if (advanced > 0) {
    draw_minimap_traffic(frame, x, y);
  }

  draw_minimap_rect(frame, x, y);
}

void
MinimapGame::draw_minimap_ownership(Frame *frame,
                                    unsigned int x, unsigned int y,
                                    int density) {
  for (unsigned int row = 0; row < map->get_rows(); row += density) {
    for (unsigned int col = 0; col < map->get_cols(); col += density) {
      MapPos pos = map->pos(col, row);
      if (map->has_owner(pos)) {
        Color color = interface->get_player_color(map->get_owner(pos));
        draw_minimap_point(frame, x, y, col, row, color, scale);
      }
    }
  }
}

void
MinimapGame::draw_minimap_roads(Frame *frame, unsigned int x, unsigned int y) {
  for (unsigned int row = 0; row < map->get_rows(); row++) {
    for (unsigned int col = 0; col < map->get_cols(); col++) {
      int pos = map->pos(col, row);
      if (map->paths(pos)) {
        draw_minimap_point(frame, x, y, col, row, Color::black, scale);
      }
    }
  }
}

void
MinimapGame::draw_minimap_buildings(Frame *frame,
                                    unsigned int x, unsigned int y) {
  const int building_remap[] = {
    Building::TypeCastle,
    Building::TypeStock, Building::TypeTower, Building::TypeHut,
    Building::TypeFortress, Building::TypeToolMaker, Building::TypeSawmill,
    Building::TypeWeaponSmith, Building::TypeStonecutter,
    Building::TypeBoatbuilder, Building::TypeForester, Building::TypeLumberjack,
    Building::TypePigFarm, Building::TypeFarm, Building::TypeFisher,
    Building::TypeMill, Building::TypeButcher, Building::TypeBaker,
    Building::TypeStoneMine, Building::TypeCoalMine, Building::TypeIronMine,
    Building::TypeGoldMine, Building::TypeSteelSmelter,
    Building::TypeGoldSmelter
  };

  for (unsigned int row = 0; row < map->get_rows(); row++) {
    for (unsigned int col = 0; col < map->get_cols(); col++) {
      int pos = map->pos(col, row);
      int obj = map->get_obj(pos);
      if (obj > Map::ObjectFlag && obj <= Map::ObjectCastle) {
        Color color = interface->get_player_color(map->get_owner(pos));
        if (advanced > 0) {
          Building *bld = interface->get_game()->get_building_at_pos(pos);
          if (bld->get_type() == building_remap[advanced]) {
            draw_minimap_point(frame, x, y, col, row, color, scale);
          }
        } else {
          draw_minimap_point(frame, x, y, col, row, color, scale);
        }
      }
    }
  }
}

void
MinimapGame::draw_minimap_traffic(Frame *frame,
                                  unsigned int x, unsigned int y) {
  for (unsigned int row = 0; row < map->get_rows(); row++) {
    for (unsigned int col = 0; col < map->get_cols(); col++) {
      int pos = map->pos(col, row);
      if (map->get_idle_serf(pos)) {
        Color color = interface->get_player_color(map->get_owner(pos));
        draw_minimap_point(frame, x, y, col, row, color, scale);
      }
    }
  }
}

bool
MinimapGame::handle_click_left(int cx, int cy) {
  MapPos pos = map_pos_from_screen_pix(cx, cy);
  interface->get_viewport()->move_to_map_pos(pos);

  interface->update_map_cursor_pos(pos);
  if (parent.lock()) {
    parent.lock()->close();
  }

  return true;
}
