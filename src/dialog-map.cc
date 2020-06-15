/*
 * dialog-map.cc - Map dialog
 *
 * Copyright (C) 2017  Wicked_Digger <wicked_digger@mail.ru>
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

#include "src/dialog-map.h"

#include <memory>

#include "src/interface.h"
#include "src/minimap.h"
#include "src/viewport.h"

DialogMap::DialogMap(Interface *_interface) {
  interface = _interface;
  set_size(144, 160);
}

DialogMap::~DialogMap() {
}

void
DialogMap::init() {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(8, 9);

  minimap = std::make_shared<MinimapGame>(128, 128, interface,
                                          interface->get_game());
  layout->add_item(0, 0, minimap);

  // Synchronize minimap window with viewport.
  std::shared_ptr<Viewport> viewport = interface->get_viewport();
  MapPos pos = viewport->get_current_map_pos();
  minimap->move_to_map_pos(pos);

  // Mode
  PButton button = layout->make_item<Button>(0, 128, 32, 16, 0,
                                        [this](unsigned int x, unsigned int y) {
    int mode = minimap->get_ownership_mode() + 1;
    if (mode > MinimapGame::OwnershipModeLast) {
      mode = MinimapGame::OwnershipModeNone;
    }
    minimap->set_ownership_mode((MinimapGame::OwnershipMode)mode);
  });
  button->set_delegate([this]()->unsigned int {
    return minimap->get_ownership_mode();
  });

  // Roads
  button = layout->make_item<Button>(32, 128, 32, 16, 3,
                                     [this](unsigned int x, unsigned int y) {
    minimap->set_draw_roads(!minimap->get_draw_roads());
  });
  button->set_delegate([this]()->unsigned int {
    return minimap->get_draw_roads() ? 3 : 4;
  });

  // Building
  button = layout->make_item<Button>(64, 128, 32, 16, 306,
                                     [this](unsigned int x, unsigned int y) {
    if (minimap->get_advanced() >= 0) {
      minimap->set_advanced(-1);
      minimap->set_draw_buildings(true);
    } else {
      minimap->set_draw_buildings(!minimap->get_draw_buildings());
    }
  });
  button->set_delegate([this]()->unsigned int {
    if (minimap->get_advanced() >= 0) {  // Unknown mode
      return (minimap->get_advanced() == 0) ? 306 : 305;
    } else {  // Buildings
      return minimap->get_draw_buildings() ? 5 : 6;
    }
  });

  // Grid
  button = layout->make_item<Button>(96, 128, 16, 16, 7,
                                     [this](unsigned int x, unsigned int y) {
    minimap->set_draw_grid(!minimap->get_draw_grid());
  });
  button->set_delegate([this]()->unsigned int {
    return minimap->get_draw_grid() ? 7 : 8;
  });

  // Scale
  button = layout->make_item<Button>(112, 128, 16, 16, 91,
                                     [this](unsigned int x, unsigned int y) {
    minimap->set_scale(minimap->get_scale() == 1 ? 2 : 1);
  });
  button->set_delegate([this]()->unsigned int {
    return (minimap->get_scale() == 1) ? 91 : 92;
  });

  add_layout(layout);
}
