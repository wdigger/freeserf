/*
 * dialog-build.cc - Build dialog
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

#include "src/dialog-build.h"

#include <memory>

#include "src/interface.h"
#include "src/game.h"

class BuildingButton : public Button {
 protected:
  Color color;

 public:
  BuildingButton(unsigned int _width, unsigned int _height,
                 unsigned int _sprite, Player::Color _color,
                 std::function<void(int x, int y)> _handler)
    : Button(_width, _height, _sprite, _handler) {
    color = Color(_color.red, _color.green, _color.blue, 0xFF);
  }

  void draw(Frame *frame, unsigned int x, unsigned int y) {
    frame->draw_sprite(x, y, Data::AssetMapObject, sprite, false, color);
  }
};

class LayoutBuild : public Layout {
 protected:
  Interface *interface;

 public:
  explicit LayoutBuild(Interface *_interface)
    : Layout(144, 144) {
    set_indents(8, 9);
    interface = _interface;
  }

  void make_bld_item(unsigned int x, unsigned int y,
                     unsigned int width, unsigned int height,
                     unsigned int sprite, Building::Type type) {
    Player::Color color = {0, 0, 0};
    Layout::make_item<BuildingButton>(x, y, width, height, sprite, color,
                                      [this, type](unsigned int x,
                                                   unsigned int y) {
      this->interface->build_building(type);
      if (this->parent.lock()) {
        this->parent.lock()->close();
      }
    });
  }
};

typedef std::shared_ptr<LayoutBuild> PLayoutBuild;

DialogBuild::DialogBuild(Interface *_interface) {
  interface = _interface;
  set_size(144, 160);
  set_background(131);
}

DialogBuild::~DialogBuild() {
}

void
DialogBuild::init() {
  Interface::BuildPossibility build_possibility =
                                             interface->get_build_possibility();

  if (build_possibility == Interface::BuildPossibilityMine) {
    add_layout(create_layout_mine());
  } else {
    PLayout layout = create_layout_small();
    add_layout(layout);

    if (build_possibility == Interface::BuildPossibilityLarge) {
      add_flip_button(layout);
      layout = create_layout_medium();
      add_flip_button(layout);
      add_layout(layout);
      layout = create_layout_large();
      add_flip_button(layout);
      add_layout(layout);
    }
  }
}

void
DialogBuild::add_flip_button(PLayout layout) {
  layout->make_item<Button>(0, 128, 16, 16, 61,
                            [this](unsigned int x, unsigned int y) {
    this->next_layout();
  });
}

// Building mines layout
PLayout
DialogBuild::create_layout_mine() {
  PLayoutBuild layout = std::make_shared<LayoutBuild>(interface);

  layout->make_bld_item(16, 8, 33, 65, 163, Building::TypeStoneMine);
  layout->make_bld_item(64, 8, 33, 65, 164, Building::TypeCoalMine);
  layout->make_bld_item(32, 77, 33, 65, 165, Building::TypeIronMine);
  layout->make_bld_item(80, 77, 33, 65, 166, Building::TypeGoldMine);

  // Flag
  if (interface->get_game()->can_build_flag(interface->get_map_cursor_pos(),
                                            interface->get_player())) {
    layout->make_item<BuildingButton>(8, 112, 17, 21,
                                     0x80, interface->get_player()->get_color(),
                                      [this](int x, int y) {
      this->interface->build_flag();
      this->close();
    });
  }

  return layout;
}

// Building small buildings layout
PLayout
DialogBuild::create_layout_small() {
  PLayoutBuild layout = std::make_shared<LayoutBuild>(interface);

  if (interface->get_game()->can_build_military(
                                             interface->get_map_cursor_pos())) {
    layout->make_bld_item(80, 13, 33, 27, 171, Building::TypeHut);
  }
  layout->make_bld_item(16, 13, 33, 29, 169, Building::TypeStonecutter);
  layout->make_bld_item(0, 58, 33, 24, 168, Building::TypeLumberjack);
  layout->make_bld_item(48, 56, 33, 26, 170, Building::TypeForester);
  layout->make_bld_item(96, 55, 33, 30, 167, Building::TypeFisher);
  layout->make_bld_item(16, 85, 33, 58, 188, Building::TypeMill);
  layout->make_bld_item(80, 87, 33, 53, 174, Building::TypeBoatbuilder);

  // Flag
  if (interface->get_game()->can_build_flag(interface->get_map_cursor_pos(),
                                            interface->get_player())) {
    layout->make_item<BuildingButton>(64, 108, 17, 21,
                                     0x80, interface->get_player()->get_color(),
                                      [this](unsigned int x, unsigned int y) {
      this->interface->build_flag();
      this->close();
    });
  }

  return layout;
}

PLayout
DialogBuild::create_layout_medium() {
  PLayoutBuild layout = std::make_shared<LayoutBuild>(interface);

  layout->make_bld_item(0, 15, 65, 26, 156, Building::TypeButcher);
  layout->make_bld_item(64, 15, 65, 26, 157, Building::TypeWeaponSmith);
  layout->make_bld_item(0, 50, 49, 39, 161, Building::TypeSteelSmelter);
  layout->make_bld_item(64, 50, 49, 41, 160, Building::TypeSawmill);
  layout->make_bld_item(16, 100, 49, 33, 162, Building::TypeBaker);
  layout->make_bld_item(80, 96, 49, 40, 159, Building::TypeGoldSmelter);

  return layout;
}

PLayout
DialogBuild::create_layout_large() {
  PLayoutBuild layout = std::make_shared<LayoutBuild>(interface);

  if (interface->get_game()->can_build_military(
                                             interface->get_map_cursor_pos())) {
    layout->make_bld_item(64, 87, 64, 56, 152, Building::TypeFortress);
    layout->make_bld_item(16, 99, 48, 43, 158, Building::TypeTower);
  }

  layout->make_bld_item(0, 1, 64, 48, 153, Building::TypeToolMaker);
  layout->make_bld_item(0, 50, 64, 56, 192, Building::TypeStock);
  layout->make_bld_item(64, 1, 64, 42, 154, Building::TypeFarm);
  layout->make_bld_item(64, 45, 64, 41, 155, Building::TypePigFarm);

  return layout;
}
