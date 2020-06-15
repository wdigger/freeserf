/*
 * minimap.h - Minimap GUI component
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

#ifndef SRC_MINIMAP_H_
#define SRC_MINIMAP_H_

#include <vector>
#include <array>
#include <map>

#include "src/gui.h"
#include "src/map.h"
#include "src/game.h"
#include "src/dialog.h"

class Interface;

class Minimap : public Control {
 protected:
  PMap map;

  int offset_x, offset_y;
  int scale;

  bool draw_grid;

  std::map<Map::Terrain, std::array<Color, 17>> palette;

  std::vector<Color> minimap;

 public:
  explicit Minimap(unsigned int _width, unsigned int _height, PMap map);

  void set_map(PMap map);

  int get_scale() const { return scale; }
  void set_scale(int scale);
  bool get_draw_grid() const { return draw_grid; }
  void set_draw_grid(bool draw_grid);

  void move_to_map_pos(MapPos pos);
  void move_by_pixels(int x, int y);
  MapPos get_current_map_pos();

  void screen_pix_from_map_pos(MapPos pos, int *sx, int *sy);
  MapPos map_pos_from_screen_pix(int x, int y);

 protected:
  static const int max_scale;

  void init_minimap();

  void draw_minimap_point(Frame *frame, unsigned int x, unsigned int y,
                          int col, int row,
                          const Color &color, int density);
  void draw_minimap_map(Frame *frame, unsigned int x, unsigned int y);
  void draw_minimap_grid(Frame *frame, unsigned int x, unsigned int y);
  void draw_minimap_rect(Frame *frame, unsigned int x, unsigned int y);
  int handle_scroll(int up);
  void screen_pix_from_map_pix(int mx, int my, int *sx, int *sy);
  void map_pix_from_map_coord(MapPos pos, int *mx, int *my);

  virtual void draw(Frame *frame, unsigned int x, unsigned int y);
  virtual bool handle_drag(int dx, int dy);

  template <size_t size> std::array<Color, size>
                              make_gradient(Color color_start, Color color_end);
};

class MinimapGame : public Minimap {
 public:
  typedef enum OwnershipMode {
    OwnershipModeNone = 0,
    OwnershipModeMixed = 1,
    OwnershipModeSolid = 2,
    OwnershipModeLast = OwnershipModeSolid
  } OwnershipMode;

 protected:
  Interface *interface;

  PGame game;

  int advanced;
  bool draw_roads;
  bool draw_buildings;
  OwnershipMode ownership_mode;

 public:
  MinimapGame(unsigned int _width, unsigned int _height,
              Interface *interface, PGame game);

  int get_advanced() const { return advanced; }
  void set_advanced(int advanced) { this->advanced = advanced; }
  bool get_draw_roads() const { return draw_roads; }
  void set_draw_roads(bool draw_roads);
  bool get_draw_buildings() const { return draw_buildings; }
  void set_draw_buildings(bool draw_buildings);
  OwnershipMode get_ownership_mode() { return ownership_mode; }
  void set_ownership_mode(OwnershipMode _ownership_mode);

 protected:
  void draw_minimap_ownership(Frame *frame, unsigned int x, unsigned int y,
                              int density);
  void draw_minimap_roads(Frame *frame, unsigned int x, unsigned int y);
  void draw_minimap_buildings(Frame *frame, unsigned int x, unsigned int y);
  void draw_minimap_traffic(Frame *frame, unsigned int x, unsigned int y);

  virtual void draw(Frame *frame, unsigned int x, unsigned int y);
  virtual bool handle_click_left(int x, int y);
};

#endif  // SRC_MINIMAP_H_
