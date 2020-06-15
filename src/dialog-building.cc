/*
 * dialog-building.cc - Building dialog
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

#include "src/dialog-building.h"

#include <sstream>
#include <memory>
#include <cassert>

#include "src/interface.h"
#include "src/game.h"

class BuildingButton : public Button {
 public:
  BuildingButton(unsigned int _width, unsigned int _height,
                 unsigned int _sprite,
                 std::function<void(int x, int y)> _handler)
    : Button(_width, _height, _sprite, _handler) {
  }

  void draw(Frame *frame, unsigned int x, unsigned int y) {
    frame->draw_sprite(x, y, Data::AssetMapObject, sprite);
  }
};

DialogBuilding::DialogBuilding(Interface *_interface, Building *_building) {
  interface = _interface;
  building = _building;
  set_size(144, 160);
  set_background(312);
}

DialogBuilding::~DialogBuilding() {
}

void
DialogBuilding::init() {
  if (!building->is_done()) {
    add_layout(create_layout_ordered());
    return;
  }

  Building::Type type = building->get_type();

  switch (type) {
    case Building::TypeStock:
    case Building::TypeCastle: {
      create_layout_inventory();
      break;
    }
    case Building::TypeStoneMine:
    case Building::TypeCoalMine:
    case Building::TypeIronMine:
    case Building::TypeGoldMine: {
      PLayout layout = create_layout_mine();
      add_layout(layout);
      break;
    }
    case Building::TypeHut:
    case Building::TypeTower:
    case Building::TypeFortress: {
      add_layout(create_layout_military());
      break;
    }
    default: {
    }
  }
}

void
DialogBuilding::add_flip_button(PLayout layout) {
  layout->make_item<Button>(92, 128, 16, 16, 61,
                            [this](unsigned int x, unsigned int y) {
    this->next_layout();
  });
}

void
DialogBuilding::add_exit_button(PLayout layout) {
  layout->make_item<Button>(108, 128, 16, 16, 60,
                            [this](unsigned int x, unsigned int y) {
    this->close();
  });
}

// Building mine layout
PLayout
DialogBuilding::create_layout_mine() {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(8, 8);
/*
  draw_box_background(PatternPlaidAlongGreen);

  if (interface->get_player()->temp_index == 0) {
    close();
    return;
  }

  Building *building =
  interface->get_game()->get_building(interface->get_player()->temp_index);
  if (building->is_burning()) {
    close();
    return;
  }

  Building::Type type = building->get_type();

  if (type != Building::TypeStoneMine &&
      type != Building::TypeCoalMine &&
      type != Building::TypeIronMine &&
      type != Building::TypeGoldMine) {
    close();
    return;
  }

  // Draw building
  draw_popup_building(6, 60, map_building_sprite[type]);

  // Draw serf icon
  int sprite = 0xdc; // minus box
  if (building->has_serf()) sprite = 0x11; // miner

  draw_popup_icon(10, 75, sprite);

  // Draw food present at mine
  int stock = building->get_res_count_in_stock(0);
  int stock_left_col = (stock + 1) >> 1;
  int stock_right_col = stock >> 1;

  // Left column
  for (int i = 0; i < stock_left_col; i++) {
    draw_popup_icon(1, 90 - 8*stock_left_col + i*16,
                    0x24); // meat (food) sprite
  }

  // Right column
  for (int i = 0; i < stock_right_col; i++) {
    draw_popup_icon(13, 90 - 8*stock_right_col + i*16,
                    0x24); // meat (food) sprite
  }

  // Calculate output percentage (simple WMA)
  const int output_weight[] = { 10, 10, 9, 9, 8, 8, 7, 7,
    6,  6, 5, 5, 4, 3, 2, 1 };
  int output = 0;
  for (int i = 0; i < 15; i++) {
    output += !!BIT_TEST(building->get_progress(), i) * output_weight[i];
  }

  // Print output precentage
  int x = 7;
  if (output >= 100) x += 1;
  if (output >= 10) x += 1;
  draw_green_string(x, 38, "%");
  draw_green_number(6, 38, output);

  draw_green_string(1, 14, "MINING");
  draw_green_string(1, 24, "OUTPUT:");

  // Exit box
  draw_popup_icon(14, 128, 0x3c);
*/
  return layout;
}

PLayout
DialogBuilding::create_layout_ordered() {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(8, 8);
/*
  draw_box_background(PatternPlaidAlongGreen);

  if (interface->get_player()->temp_index == 0) {
    close();
    return;
  }

  Building *building =
  interface->get_game()->get_building(interface->get_player()->temp_index);
  if (building->is_burning()) {
    close();
    return;
  }

  Building::Type type = building->get_type();

  int sprite = map_building_sprite[type];
  int x = 6;
  if (sprite == 0xc0 || sprite < 0x9e) x = 4; // stock, tower
  draw_popup_building(x, 40, sprite);

  draw_green_string(2, 4, "Ordered");
  draw_green_string(2, 14, "Building");

  if (building->has_serf()) {
    if (building->get_progress() == 0) { // Digger
      draw_popup_icon(2, 100, 0xb);
    } else { // Builder
      draw_popup_icon(2, 100, 0xc);
    }
  } else {
    draw_popup_icon(2, 100, 0xdc); // Minus box
  }

  draw_popup_icon(14, 128, 0x3c); // Exit box
*/
  return layout;
}

PLayout
DialogBuilding::create_layout_military() {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(8, 8);
/*
  draw_box_background(PatternPlaidAlongGreen);

  if (interface->get_player()->temp_index == 0) {
    close();
    return;
  }

  Building *building =
  interface->get_game()->get_building(interface->get_player()->temp_index);
  if (building->is_burning()) {
    close();
    return;
  }

  if (building->get_owner() != interface->get_player()->get_index()) {
    close();
    return;
  }

  if (building->get_type() != Building::TypeHut &&
      building->get_type() != Building::TypeTower &&
      building->get_type() != Building::TypeFortress) {
    close();
    return;
  }

  // Draw building sprite
  int sprite = map_building_sprite[building->get_type()];
  int x = 0, y = 0;
  switch (building->get_type()) {
    case Building::TypeHut: x = 6; y = 20; break;
    case Building::TypeTower: x = 4; y = 6; break;
    case Building::TypeFortress: x = 4; y = 1; break;
    default: NOT_REACHED(); break;
  }

  draw_popup_building(x, y, sprite);

  // Draw gold stock
  if (building->get_res_count_in_stock(1) > 0) {
    int left = (building->get_res_count_in_stock(1) + 1) / 2;
    for (int i = 0; i < left; i++) {
      draw_popup_icon(1, 32 - 8*left + 16*i, 0x30);
    }

    int right = building->get_res_count_in_stock(1) / 2;
    for (int i = 0; i < right; i++) {
      draw_popup_icon(13, 32 - 8*right + 16*i, 0x30);
    }
  }

  // Draw heading string
  draw_green_string(3, 62, "Defenders:");

  // Draw knights
  int next_knight = building->get_first_knight();
  for (int i = 0; next_knight != 0; i++) {
    Serf *serf = interface->get_game()->get_serf(next_knight);
    draw_popup_icon(3 + 4*(i%3), 72 + 16*(i/3), 7 + serf->get_type());
    next_knight = serf->get_next();
  }

  draw_green_string(0, 128, "State:");
  draw_green_number(7, 128, static_cast<int>(building->get_threat_level()));

  draw_popup_icon(14, 128, 0x3c); // Exit box
*/
  return layout;
}

// Building inventory layout

/* Draw generic popup box of resources. */
void
DialogBuilding::create_layout_inventory() {
  Inventory *inventory = building->get_inventory();
  add_layout(create_layout_resources(inventory));
  add_layout(create_layout_serfs(inventory));
  add_layout(create_layout_control(inventory));
}

PLayout
DialogBuilding::create_layout_resources(Inventory *inventory) {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(12, 8);

  add_flip_button(layout);
  add_exit_button(layout);

  ResourceMap resources = inventory->get_all_resources();
  const int layout_res[] = {
    0x28,  0,   0, Resource::TypeLumber,
    0x29,  0,  16, Resource::TypePlank,
    0x2a,  0,  32, Resource::TypeBoat,
    0x2b,  0,  48, Resource::TypeStone,
    0x2e,  0,  64, Resource::TypeCoal,
    0x2c,  0,  80, Resource::TypeIronOre,
    0x2d,  0,  96, Resource::TypeSteel,
    0x2f,  0, 112, Resource::TypeGoldOre,
    0x30,  0, 128, Resource::TypeGoldBar,
    0x31, 40,   0, Resource::TypeShovel,
    0x32, 40,  16, Resource::TypeHammer,
    0x36, 40,  32, Resource::TypeAxe,
    0x37, 40,  48, Resource::TypeSaw,
    0x35, 40,  64, Resource::TypeScythe,
    0x38, 40,  80, Resource::TypePick,
    0x39, 40,  96, Resource::TypePincer,
    0x34, 40, 112, Resource::TypeCleaver,
    0x33, 40, 128, Resource::TypeRod,
    0x3a, 80,   0, Resource::TypeSword,
    0x3b, 80,  16, Resource::TypeShield,
    0x22, 80,  32, Resource::TypeFish,
    0x23, 80,  48, Resource::TypePig,
    0x24, 80,  64, Resource::TypeMeat,
    0x25, 80,  80, Resource::TypeWheat,
    0x26, 80,  96, Resource::TypeFlour,
    0x27, 80, 112, Resource::TypeBread
    -1
  };

  for (size_t i = 0; i < sizeof(layout_res)/sizeof(layout_res[0])/4; i++) {
    Resource::Type res_type = (Resource::Type)layout_res[i*4+3];
    ResourceMap::const_iterator it = resources.find(res_type);
    int value = 0;
    if (it != resources.end()) {
      value = static_cast<int>(it->second);
    }
    PLabel label = std::make_shared<Label>(40, 16, layout_res[i*4]);
    std::stringstream stream;
    stream << value;
    label->set_text(stream.str());
    layout->add_item(layout_res[i*4+1], layout_res[i*4+2], label);
  }

  return layout;
}

PLayout
DialogBuilding::create_layout_serfs(Inventory *inventory) {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(12, 8);

  add_flip_button(layout);
  add_exit_button(layout);

  int serfs[27] = {0};
  for (Serf *serf : interface->get_game()->get_serfs_in_inventory(inventory)) {
    serfs[serf->get_type()] += 1;
  }

  const int layout_serf[] = {
    0x09,  0,   0, Serf::TypeTransporter,
    0x0a,  0,  16, Serf::TypeSailor,
    0x0b,  0,  32, Serf::TypeDigger,
    0x0c,  0,  48, Serf::TypeBuilder,
    0x21,  0,  64, Serf::TypeKnight4,
    0x20,  0,  80, Serf::TypeKnight3,
    0x1f,  0,  96, Serf::TypeKnight2,
    0x1e,  0, 112, Serf::TypeKnight1,
    0x1d,  0, 128, Serf::TypeKnight0,
    0x0d, 40,   0, Serf::TypeLumberjack,
    0x0e, 40,  16, Serf::TypeSawmiller,
    0x12, 40,  32, Serf::TypeSmelter,
    0x0f, 40,  48, Serf::TypeStonecutter,
    0x10, 40,  64, Serf::TypeForester,
    0x11, 40,  80, Serf::TypeMiner,
    0x19, 40,  96, Serf::TypeBoatBuilder,
    0x1a, 40, 112, Serf::TypeToolmaker,
    0x1b, 40, 128, Serf::TypeWeaponSmith,
    0x13, 80,   0, Serf::TypeFisher,
    0x14, 80,  16, Serf::TypePigFarmer,
    0x15, 80,  32, Serf::TypeButcher,
    0x16, 80,  48, Serf::TypeFarmer,
    0x17, 80,  64, Serf::TypeMiller,
    0x18, 80,  80, Serf::TypeBaker,
    0x1c, 80,  96, Serf::TypeGeologist,
    0x82, 80, 112, Serf::TypeGeneric,
    -1
  };

  for (size_t i = 0; i < sizeof(layout_serf)/sizeof(layout_serf[0])/4; i++) {
    Serf::Type type = (Serf::Type)layout_serf[i*4+3];
    int value = serfs[type];
    PLabel label = std::make_shared<Label>(40, 16, layout_serf[i*4]);
    std::stringstream stream;
    stream << value;
    label->set_text(stream.str());
    layout->add_item(layout_serf[i*4+1], layout_serf[i*4+2], label);
  }

  return layout;
}

PLayout
DialogBuilding::create_layout_control(Inventory *inventory) {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(12, 8);

  add_flip_button(layout);
  add_exit_button(layout);

  layout->add_item(24, 16, std::make_shared<Label>(32, 48, 296));
  layout->add_item(24, 80, std::make_shared<Label>(32, 48, 296));

  // Resource mode checkbox
  PButton button = std::make_shared<Button>(16, 16, 220,
                                            [this, inventory](unsigned int x,
                                                              unsigned int y) {
    inventory->set_res_mode(Inventory::ModeIn);
    this->set_redraw();
  });
  button->set_delegate([inventory]()->int{
    return (inventory->get_res_mode() == Inventory::ModeIn) ? 288 : 220;
  });
  layout->add_item(62, 16, button);
  button = std::make_shared<Button>(16, 16, 220,
                                    [this, inventory](unsigned int x,
                                                      unsigned int y) {
    inventory->set_res_mode(Inventory::ModeStop);
    this->set_redraw();
  });
  button->set_delegate([inventory]()->int{
    return (inventory->get_res_mode() == Inventory::ModeStop) ? 288 : 220;
  });
  layout->add_item(62, 32, button);
  button = std::make_shared<Button>(16, 16, 220,
                                    [this, inventory](unsigned int x,
                                                      unsigned int y) {
    inventory->set_res_mode(Inventory::ModeOut);
    this->set_redraw();
  });
  button->set_delegate([inventory]()->int{
    return (inventory->get_res_mode() == Inventory::ModeOut) ? 288 : 220;
  });
  layout->add_item(62, 48, button);

  // Serf mode checkbox
  button = std::make_shared<Button>(16, 16, 220,
                                            [this, inventory](unsigned int x,
                                                              unsigned int y) {
    inventory->set_serf_mode(Inventory::ModeIn);
    this->set_redraw();
  });
  button->set_delegate([inventory]()->int{
    return (inventory->get_serf_mode() == Inventory::ModeIn) ? 288 : 220;
  });
  layout->add_item(62, 80, button);
  button = std::make_shared<Button>(16, 16, 220,
                                    [this, inventory](unsigned int x,
                                                      unsigned int y) {
    inventory->set_serf_mode(Inventory::ModeStop);
    this->set_redraw();
  });
  button->set_delegate([inventory]()->int{
    return (inventory->get_serf_mode() == Inventory::ModeStop) ? 288 : 220;
  });
  layout->add_item(62, 96, button);
  button = std::make_shared<Button>(16, 16, 220,
                                    [this, inventory](unsigned int x,
                                                      unsigned int y) {
    inventory->set_serf_mode(Inventory::ModeOut);
    this->set_redraw();
                                    });
  button->set_delegate([inventory]()->int{
    return (inventory->get_serf_mode() == Inventory::ModeOut) ? 288 : 220;
  });
  layout->add_item(62, 112, button);

  if (building->get_type() == Building::TypeCastle) {
    int knights[5] = { 0 };

    // Follow linked list of knights on duty
    int serf_index = building->get_first_knight();
    while (serf_index != 0) {
      Serf *serf = interface->get_game()->get_serf(serf_index);
      Serf::Type serf_type = serf->get_type();
      assert(serf_type >= Serf::TypeKnight0 && serf_type <= Serf::TypeKnight4);
      knights[serf_type-Serf::TypeKnight0] += 1;
      serf_index = serf->get_next();
    }

    for (int i = 0; i < 5; i++) {
      PLabel label = std::make_shared<Label>(32, 16, 33-i);
      layout->add_item(84, 16 + i*20, label);
      std::stringstream stream;
      stream << knights[4-i];
      label->set_text(stream.str());
    }
  }

  return layout;
}
