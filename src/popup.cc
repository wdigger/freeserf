/*
 * popup.cc - Popup GUI component
 *
 * Copyright (C) 2013-2018  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "src/popup.h"

#include <algorithm>
#include <sstream>

#include "src/misc.h"
#include "src/game.h"
#include "src/debug.h"
#include "src/data.h"
#include "src/audio.h"
#include "src/gfx.h"
#include "src/interface.h"
#include "src/event_loop.h"
#include "src/minimap.h"
#include "src/viewport.h"
#include "src/inventory.h"
#include "src/list.h"
#include "src/text-input.h"

/* Action types that can be fired from
   clicks in the popup window. */
typedef enum Action {
  ACTION_SHOW_STAT_1,
  ACTION_SHOW_STAT_2,
  ACTION_SHOW_STAT_8,
  ACTION_SHOW_STAT_BLD,
  ACTION_SHOW_STAT_6,
  ACTION_SHOW_STAT_7,
  ACTION_SHOW_STAT_4,
  ACTION_SHOW_STAT_3,
  ACTION_SHOW_STAT_SELECT,
  ACTION_STAT_BLD_FLIP,
  ACTION_CLOSE_BOX,
  ACTION_SETT_8_SET_ASPECT_ALL,
  ACTION_SETT_8_SET_ASPECT_LAND,
  ACTION_SETT_8_SET_ASPECT_BUILDINGS,
  ACTION_SETT_8_SET_ASPECT_MILITARY,
  ACTION_SETT_8_SET_SCALE_30_MIN,
  ACTION_SETT_8_SET_SCALE_60_MIN,
  ACTION_SETT_8_SET_SCALE_600_MIN,
  ACTION_SETT_8_SET_SCALE_3000_MIN,
  ACTION_STAT_7_SELECT_FISH,
  ACTION_STAT_7_SELECT_PIG,
  ACTION_STAT_7_SELECT_MEAT,
  ACTION_STAT_7_SELECT_WHEAT,
  ACTION_STAT_7_SELECT_FLOUR,
  ACTION_STAT_7_SELECT_BREAD,
  ACTION_STAT_7_SELECT_LUMBER,
  ACTION_STAT_7_SELECT_PLANK,
  ACTION_STAT_7_SELECT_BOAT,
  ACTION_STAT_7_SELECT_STONE,
  ACTION_STAT_7_SELECT_IRONORE,
  ACTION_STAT_7_SELECT_STEEL,
  ACTION_STAT_7_SELECT_COAL,
  ACTION_STAT_7_SELECT_GOLDORE,
  ACTION_STAT_7_SELECT_GOLDBAR,
  ACTION_STAT_7_SELECT_SHOVEL,
  ACTION_STAT_7_SELECT_HAMMER,
  ACTION_STAT_7_SELECT_ROD,
  ACTION_STAT_7_SELECT_CLEAVER,
  ACTION_STAT_7_SELECT_SCYTHE,
  ACTION_STAT_7_SELECT_AXE,
  ACTION_STAT_7_SELECT_SAW,
  ACTION_STAT_7_SELECT_PICK,
  ACTION_STAT_7_SELECT_PINCER,
  ACTION_STAT_7_SELECT_SWORD,
  ACTION_STAT_7_SELECT_SHIELD,
  ACTION_ATTACKING_KNIGHTS_DEC,
  ACTION_ATTACKING_KNIGHTS_INC,
  ACTION_START_ATTACK,
  ACTION_CLOSE_ATTACK_BOX,
  /* ... 78 - 91 ... */
  ACTION_CLOSE_SETT_BOX = 92,
  ACTION_SHOW_SETT_1,
  ACTION_SHOW_SETT_2,
  ACTION_SHOW_SETT_3,
  ACTION_SHOW_SETT_7,
  ACTION_SHOW_SETT_4,
  ACTION_SHOW_SETT_5,
  ACTION_SHOW_SETT_SELECT,
  ACTION_SETT_1_ADJUST_STONEMINE,
  ACTION_SETT_1_ADJUST_COALMINE,
  ACTION_SETT_1_ADJUST_IRONMINE,
  ACTION_SETT_1_ADJUST_GOLDMINE,
  ACTION_SETT_2_ADJUST_CONSTRUCTION,
  ACTION_SETT_2_ADJUST_BOATBUILDER,
  ACTION_SETT_2_ADJUST_TOOLMAKER_PLANKS,
  ACTION_SETT_2_ADJUST_TOOLMAKER_STEEL,
  ACTION_SETT_2_ADJUST_WEAPONSMITH,
  ACTION_SETT_3_ADJUST_STEELSMELTER,
  ACTION_SETT_3_ADJUST_GOLDSMELTER,
  ACTION_SETT_3_ADJUST_WEAPONSMITH,
  ACTION_SETT_3_ADJUST_PIGFARM,
  ACTION_SETT_3_ADJUST_MILL,
  ACTION_KNIGHT_LEVEL_CLOSEST_MIN_DEC,
  ACTION_KNIGHT_LEVEL_CLOSEST_MIN_INC,
  ACTION_KNIGHT_LEVEL_CLOSEST_MAX_DEC,
  ACTION_KNIGHT_LEVEL_CLOSEST_MAX_INC,
  ACTION_KNIGHT_LEVEL_CLOSE_MIN_DEC,
  ACTION_KNIGHT_LEVEL_CLOSE_MIN_INC,
  ACTION_KNIGHT_LEVEL_CLOSE_MAX_DEC,
  ACTION_KNIGHT_LEVEL_CLOSE_MAX_INC,
  ACTION_KNIGHT_LEVEL_FAR_MIN_DEC,
  ACTION_KNIGHT_LEVEL_FAR_MIN_INC,
  ACTION_KNIGHT_LEVEL_FAR_MAX_DEC,
  ACTION_KNIGHT_LEVEL_FAR_MAX_INC,
  ACTION_KNIGHT_LEVEL_FARTHEST_MIN_DEC,
  ACTION_KNIGHT_LEVEL_FARTHEST_MIN_INC,
  ACTION_KNIGHT_LEVEL_FARTHEST_MAX_DEC,
  ACTION_KNIGHT_LEVEL_FARTHEST_MAX_INC,
  ACTION_SETT_4_ADJUST_SHOVEL,
  ACTION_SETT_4_ADJUST_HAMMER,
  ACTION_SETT_4_ADJUST_AXE,
  ACTION_SETT_4_ADJUST_SAW,
  ACTION_SETT_4_ADJUST_SCYTHE,
  ACTION_SETT_4_ADJUST_PICK,
  ACTION_SETT_4_ADJUST_PINCER,
  ACTION_SETT_4_ADJUST_CLEAVER,
  ACTION_SETT_4_ADJUST_ROD,
  ACTION_SETT_5_6_ITEM_1,
  ACTION_SETT_5_6_ITEM_2,
  ACTION_SETT_5_6_ITEM_3,
  ACTION_SETT_5_6_ITEM_4,
  ACTION_SETT_5_6_ITEM_5,
  ACTION_SETT_5_6_ITEM_6,
  ACTION_SETT_5_6_ITEM_7,
  ACTION_SETT_5_6_ITEM_8,
  ACTION_SETT_5_6_ITEM_9,
  ACTION_SETT_5_6_ITEM_10,
  ACTION_SETT_5_6_ITEM_11,
  ACTION_SETT_5_6_ITEM_12,
  ACTION_SETT_5_6_ITEM_13,
  ACTION_SETT_5_6_ITEM_14,
  ACTION_SETT_5_6_ITEM_15,
  ACTION_SETT_5_6_ITEM_16,
  ACTION_SETT_5_6_ITEM_17,
  ACTION_SETT_5_6_ITEM_18,
  ACTION_SETT_5_6_ITEM_19,
  ACTION_SETT_5_6_ITEM_20,
  ACTION_SETT_5_6_ITEM_21,
  ACTION_SETT_5_6_ITEM_22,
  ACTION_SETT_5_6_ITEM_23,
  ACTION_SETT_5_6_ITEM_24,
  ACTION_SETT_5_6_ITEM_25,
  ACTION_SETT_5_6_ITEM_26,
  ACTION_SETT_5_6_TOP,
  ACTION_SETT_5_6_UP,
  ACTION_SETT_5_6_DOWN,
  ACTION_SETT_5_6_BOTTOM,
  ActionShowOptions,
  ACTION_SHOW_SAVE,
  ACTION_SETT_8_CYCLE,
  ACTION_CLOSE_OPTIONS,
  ACTION_OPTIONS_PATHWAY_SCROLLING_1,
  ACTION_OPTIONS_PATHWAY_SCROLLING_2,
  ACTION_OPTIONS_FAST_MAP_CLICK_1,
  ACTION_OPTIONS_FAST_MAP_CLICK_2,
  ACTION_OPTIONS_FAST_BUILDING_1,
  ACTION_OPTIONS_FAST_BUILDING_2,
  ACTION_OPTIONS_MESSAGE_COUNT_1,
  ACTION_OPTIONS_MESSAGE_COUNT_2,
  ACTION_SHOW_SETT_SELECT_FILE, /* UNUSED */
  ACTION_SHOW_STAT_SELECT_FILE, /* UNUSED */
  ACTION_DEFAULT_SETT_1,
  ACTION_DEFAULT_SETT_2,
  ACTION_DEFAULT_SETT_5_6,
  ACTION_BUILD_STOCK,
  ACTION_SEND_GEOLOGIST,
  ACTION_RES_MODE_IN,
  ACTION_RES_MODE_STOP,
  ACTION_RES_MODE_OUT,
  ACTION_SERF_MODE_IN,
  ACTION_SERF_MODE_STOP,
  ACTION_SERF_MODE_OUT,
  ACTION_SHOW_SETT_8,
  ACTION_SHOW_SETT_6,
  ACTION_SETT_8_ADJUST_RATE,
  ACTION_SETT_8_TRAIN_1,
  ACTION_SETT_8_TRAIN_5,
  ACTION_SETT_8_TRAIN_20,
  ACTION_SETT_8_TRAIN_100,
  ACTION_DEFAULT_SETT_3,
  ACTION_SETT_8_SET_COMBAT_MODE_WEAK,
  ACTION_SETT_8_SET_COMBAT_MODE_STRONG,
  ACTION_ATTACKING_SELECT_ALL_1,
  ACTION_ATTACKING_SELECT_ALL_2,
  ACTION_ATTACKING_SELECT_ALL_3,
  ACTION_ATTACKING_SELECT_ALL_4,
  ACTION_MINIMAP_BLD_1,
  ACTION_MINIMAP_BLD_2,
  ACTION_MINIMAP_BLD_3,
  ACTION_MINIMAP_BLD_4,
  ACTION_MINIMAP_BLD_5,
  ACTION_MINIMAP_BLD_6,
  ACTION_MINIMAP_BLD_7,
  ACTION_MINIMAP_BLD_8,
  ACTION_MINIMAP_BLD_9,
  ACTION_MINIMAP_BLD_10,
  ACTION_MINIMAP_BLD_11,
  ACTION_MINIMAP_BLD_12,
  ACTION_MINIMAP_BLD_13,
  ACTION_MINIMAP_BLD_14,
  ACTION_MINIMAP_BLD_15,
  ACTION_MINIMAP_BLD_16,
  ACTION_MINIMAP_BLD_17,
  ACTION_MINIMAP_BLD_18,
  ACTION_MINIMAP_BLD_19,
  ACTION_MINIMAP_BLD_20,
  ACTION_MINIMAP_BLD_21,
  ACTION_MINIMAP_BLD_22,
  ACTION_MINIMAP_BLD_23,
  ACTION_MINIMAP_BLD_FLAG,
  ACTION_MINIMAP_BLD_NEXT,
  ACTION_MINIMAP_BLD_EXIT,
  ACTION_CLOSE_MESSAGE,
  ACTION_DEFAULT_SETT_4,
  ACTION_SHOW_PLAYER_FACES,
  ACTION_OPTIONS_RIGHT_SIDE,
  ACTION_UNKNOWN_TP_INFO_FLAG,
  ACTION_SETT_8_CASTLE_DEF_DEC,
  ACTION_SETT_8_CASTLE_DEF_INC,
  ACTION_OPTIONS_MUSIC,
  ACTION_OPTIONS_FULLSCREEN,
  ACTION_OPTIONS_VOLUME_MINUS,
  ACTION_OPTIONS_VOLUME_PLUS,
  ACTION_DEMOLISH,
  ACTION_OPTIONS_SFX,
  ACTION_SAVE,
  ACTION_NEW_NAME
} Action;

PopupBox::PopupBox(Interface *_interface)
  : file_field(new TextInput(100, 100))
  , box(TypeNone) {
  interface = _interface;

  current_sett_5_item = 8;
  current_sett_6_item = 15;
  current_stat_7_item = 7;
  current_stat_8_mode = 0;
}

PopupBox::~PopupBox() {
}

void
PopupBox::init() {
/*
  file_list = std::make_shared<ListSavedFiles>();
  file_list->set_size(120, 100);
  file_list->set_displayed(false);
  file_list->set_selection_handler([this](const std::string &item) {
    size_t p = item.find_last_of("/\\");
    std::string file_name = item.substr(p+1, item.size());
    this->file_field->set_text(file_name);
  });
  add_float(file_list, 12, 22);

  file_field = std::make_shared<TextInput>();
  file_field->set_size(120, 10);
  file_field->set_displayed(false);
  add_float(file_field, 12, 124);
*/
}

/* Draw the frame around the popup box. */
void
PopupBox::draw_popup_box_frame() {
  frame->draw_sprite(0, 0, Data::AssetFramePopup, 0);
  frame->draw_sprite(0, 153, Data::AssetFramePopup, 1);
  frame->draw_sprite(0, 9, Data::AssetFramePopup, 2);
  frame->draw_sprite(136, 9, Data::AssetFramePopup, 3);
}

/* Draw icon in a popup frame. */
void
PopupBox::draw_popup_icon(int x_, int y_, int sprite) {
  frame->draw_sprite(8 * x_ + 8, y_ + 9, Data::AssetIcon, sprite);
}

/* Draw building in a popup frame. */
void
PopupBox::draw_popup_building(int x_, int y_, int sprite) {
  Player *player = interface->get_player();
  Color color = interface->get_player_color(player->get_index());
  frame->draw_sprite(8 * x_ + 8, y_ + 9,
                     Data::AssetMapObject, sprite, false, color);
}

/* Fill the background of a popup frame. */
void
PopupBox::draw_box_background(BackgroundPattern sprite) {
  for (int iy = 0; iy < 144; iy += 16) {
    for (int ix = 0; ix < 16; ix += 2) {
      draw_popup_icon(ix, iy, sprite);
    }
  }
}

/* Fill one row of a popup frame. */
void
PopupBox::draw_box_row(int sprite, int iy) {
  for (int ix = 0; ix < 16; ix += 2) {
    draw_popup_icon(ix, iy, sprite);
  }
}

/* Draw a green string in a popup frame. */
void
PopupBox::draw_green_string(int sx, int sy, const std::string &str) {
  frame->draw_string(8 * sx + 8, sy + 9, str, Color::green);
}

/* Draw a green number in a popup frame.
   n must be non-negative. If > 999 simply draw ">999" (three characters). */
void
PopupBox::draw_green_number(int sx, int sy, int n) {
  if (n >= 1000) {
    draw_popup_icon(sx, sy, 0xd5); /* Draw >999 */
    draw_popup_icon(sx+1, sy, 0xd6);
    draw_popup_icon(sx+2, sy, 0xd7);
  } else {
    frame->draw_number(8 * sx + 8, 9 + sy, n, Color::green);
  }
}

/* Draw a green number in a popup frame.
   No limits on n. */
void
PopupBox::draw_green_large_number(int sx, int sy, int n) {
  frame->draw_number(8 * sx + 8, 9 + sy, n, Color::green);
}

/* Draw small green number. */
void
PopupBox::draw_additional_number(int ix, int iy, int n) {
  if (n > 0) {
    draw_popup_icon(ix, iy, 240 + std::min(n, 10));
  }
}

/* Get the sprite number for a face. */
unsigned int
PopupBox::get_player_face_sprite(size_t face) {
  if (face != 0) {
    return static_cast<unsigned int>(0x10b + face);
  }
  return 0x119; /* sprite_face_none */
}

/* Draw player face in popup frame. */
void
PopupBox::draw_player_face(int ix, int iy, int player) {
  Color color;
  size_t face = 0;
  Player *p = interface->get_game()->get_player(player);
  if (p != nullptr) {
    color = interface->get_player_color(player);
    face = p->get_face();
  }

  frame->fill_rect(8 * ix, iy + 5, 48, 72, color);
  draw_popup_icon(ix, iy, get_player_face_sprite(face));
}

/* Draw a layout of buildings in a popup box. */
void
PopupBox::draw_custom_bld_box(const int sprites[]) {
  while (sprites[0] > 0) {
    int sx = sprites[1];
    int sy = sprites[2];
    frame->draw_sprite(8 * sx + 8, sy + 9,
                       Data::AssetMapObject, sprites[0], false);
    sprites += 3;
  }
}

/* Draw a layout of icons in a popup box. */
void
PopupBox::draw_custom_icon_box(const int sprites[]) {
  while (sprites[0] > 0) {
    draw_popup_icon(sprites[1], sprites[2], sprites[0]);
    sprites += 3;
  }
}

/* Draw generic popup box of resources. */
void
PopupBox::draw_resources_box(const ResourceMap &resources) {
  const int layout[] = {
    0x28, 1, 0, /* resources */
    0x29, 1, 16,
    0x2a, 1, 32,
    0x2b, 1, 48,
    0x2e, 1, 64,
    0x2c, 1, 80,
    0x2d, 1, 96,
    0x2f, 1, 112,
    0x30, 1, 128,
    0x31, 6, 0,
    0x32, 6, 16,
    0x36, 6, 32,
    0x37, 6, 48,
    0x35, 6, 64,
    0x38, 6, 80,
    0x39, 6, 96,
    0x34, 6, 112,
    0x33, 6, 128,
    0x3a, 11, 0,
    0x3b, 11, 16,
    0x22, 11, 32,
    0x23, 11, 48,
    0x24, 11, 64,
    0x25, 11, 80,
    0x26, 11, 96,
    0x27, 11, 112,
    -1
  };

  draw_custom_icon_box(layout);

  const int layout_res[] = {
     3,   4, Resource::TypeLumber,
     3,  20, Resource::TypePlank,
     3,  36, Resource::TypeBoat,
     3,  52, Resource::TypeStone,
     3,  68, Resource::TypeCoal,
     3,  84, Resource::TypeIronOre,
     3, 100, Resource::TypeSteel,
     3, 116, Resource::TypeGoldOre,
     3, 132, Resource::TypeGoldBar,
     8,   4, Resource::TypeShovel,
     8,  20, Resource::TypeHammer,
     8,  36, Resource::TypeAxe,
     8,  52, Resource::TypeSaw,
     8,  68, Resource::TypeScythe,
     8,  84, Resource::TypePick,
     8, 100, Resource::TypePincer,
     8, 116, Resource::TypeCleaver,
     8, 132, Resource::TypeRod,
    13,   4, Resource::TypeSword,
    13,  20, Resource::TypeShield,
    13,  36, Resource::TypeFish,
    13,  52, Resource::TypePig,
    13,  68, Resource::TypeMeat,
    13,  84, Resource::TypeWheat,
    13, 100, Resource::TypeFlour,
    13, 116, Resource::TypeBread
  };

  for (size_t i = 0; i < sizeof(layout_res)/sizeof(layout_res[0])/3; i++) {
    Resource::Type res_type = (Resource::Type)layout_res[i*3+2];
    ResourceMap::const_iterator it = resources.find(res_type);
    int value = 0;
    if (it != resources.end()) {
      value = static_cast<int>(it->second);
    }
    draw_green_number(layout_res[i*3], layout_res[i*3+1], value);
  }
}

/* Draw generic popup box of serfs. */
void
PopupBox::draw_serfs_box(const int *serfs, int total) {
  const int layout[] = {
    0x9, 1, 0, /* serfs */
    0xa, 1, 16,
    0xb, 1, 32,
    0xc, 1, 48,
    0x21, 1, 64,
    0x20, 1, 80,
    0x1f, 1, 96,
    0x1e, 1, 112,
    0x1d, 1, 128,
    0xd, 6, 0,
    0xe, 6, 16,
    0x12, 6, 32,
    0xf, 6, 48,
    0x10, 6, 64,
    0x11, 6, 80,
    0x19, 6, 96,
    0x1a, 6, 112,
    0x1b, 6, 128,
    0x13, 11, 0,
    0x14, 11, 16,
    0x15, 11, 32,
    0x16, 11, 48,
    0x17, 11, 64,
    0x18, 11, 80,
    0x1c, 11, 96,
    0x82, 11, 112,
    -1
  };

  draw_custom_icon_box(layout);

  /* First column */
  draw_green_number(3, 4, serfs[Serf::TypeTransporter]);
  draw_green_number(3, 20, serfs[Serf::TypeSailor]);
  draw_green_number(3, 36, serfs[Serf::TypeDigger]);
  draw_green_number(3, 52, serfs[Serf::TypeBuilder]);
  draw_green_number(3, 68, serfs[Serf::TypeKnight4]);
  draw_green_number(3, 84, serfs[Serf::TypeKnight3]);
  draw_green_number(3, 100, serfs[Serf::TypeKnight2]);
  draw_green_number(3, 116, serfs[Serf::TypeKnight1]);
  draw_green_number(3, 132, serfs[Serf::TypeKnight0]);

  /* Second column */
  draw_green_number(8, 4, serfs[Serf::TypeLumberjack]);
  draw_green_number(8, 20, serfs[Serf::TypeSawmiller]);
  draw_green_number(8, 36, serfs[Serf::TypeSmelter]);
  draw_green_number(8, 52, serfs[Serf::TypeStonecutter]);
  draw_green_number(8, 68, serfs[Serf::TypeForester]);
  draw_green_number(8, 84, serfs[Serf::TypeMiner]);
  draw_green_number(8, 100, serfs[Serf::TypeBoatBuilder]);
  draw_green_number(8, 116, serfs[Serf::TypeToolmaker]);
  draw_green_number(8, 132, serfs[Serf::TypeWeaponSmith]);

  /* Third column */
  draw_green_number(13, 4, serfs[Serf::TypeFisher]);
  draw_green_number(13, 20, serfs[Serf::TypePigFarmer]);
  draw_green_number(13, 36, serfs[Serf::TypeButcher]);
  draw_green_number(13, 52, serfs[Serf::TypeFarmer]);
  draw_green_number(13, 68, serfs[Serf::TypeMiller]);
  draw_green_number(13, 84, serfs[Serf::TypeBaker]);
  draw_green_number(13, 100, serfs[Serf::TypeGeologist]);
  draw_green_number(13, 116, serfs[Serf::TypeGeneric]);

  if (total >= 0) {
    draw_green_large_number(11, 132, total);
  }
}

void
PopupBox::draw_stat_select_box() {
  const int layout[] = {
    72, 1, 12,
    73, 6, 12,
    77, 11, 12,
    74, 1, 56,
    76, 6, 56,
    75, 11, 56,
    71, 1, 100,
    70, 6, 100,
    61, 12, 104, /* Flip */
    60, 14, 128, /* Exit */
    -1
  };

  draw_box_background(PatternStripedGreen);
  draw_custom_icon_box(layout);
}

void
PopupBox::draw_stat_4_box() {
  draw_box_background(PatternStripedGreen);

  /* Sum up resources of all inventories. */
  ResourceMap resources = interface->get_player()->get_stats_resources();

  draw_resources_box(resources);

  draw_popup_icon(14, 128, 60); /* exit */
}

void
PopupBox::draw_building_count(int x_, int y_, int type) {
  Player *player = interface->get_player();
  draw_green_number(x_, y_,
             player->get_completed_building_count((Building::Type)type));
  draw_additional_number(x_+1, y_,
             player->get_incomplete_building_count((Building::Type)type));
}

void
PopupBox::draw_stat_bld_1_box() {
  const int bld_layout[] = {
    192, 0, 5,
    171, 2, 77,
    158, 8, 7,
    152, 6, 69,
    -1
  };

  draw_box_background(PatternStripedGreen);

  draw_custom_bld_box(bld_layout);

  draw_building_count(2, 105, Building::TypeHut);
  draw_building_count(10, 53, Building::TypeTower);
  draw_building_count(9, 130, Building::TypeFortress);
  draw_building_count(4, 61, Building::TypeStock);

  draw_popup_icon(0, 128, 61); /* flip */
  draw_popup_icon(14, 128, 60); /* exit */
}

void
PopupBox::draw_stat_bld_2_box() {
  const int bld_layout[] = {
    153, 0, 4,
    160, 8, 6,
    157, 0, 68,
    169, 8, 65,
    174, 12, 57,
    170, 4, 105,
    168, 8, 107,
    -1
  };

  draw_box_background(PatternStripedGreen);

  draw_custom_bld_box(bld_layout);

  draw_building_count(3, 54, Building::TypeToolMaker);
  draw_building_count(10, 48, Building::TypeSawmill);
  draw_building_count(3, 95, Building::TypeWeaponSmith);
  draw_building_count(8, 95, Building::TypeStonecutter);
  draw_building_count(12, 95, Building::TypeBoatbuilder);
  draw_building_count(5, 132, Building::TypeForester);
  draw_building_count(9, 132, Building::TypeLumberjack);

  draw_popup_icon(0, 128, 61); /* flip */
  draw_popup_icon(14, 128, 60); /* exit */
}

void
PopupBox::draw_stat_bld_3_box() {
  const int bld_layout[] = {
    155, 0, 2,
    154, 8, 3,
    167, 0, 61,
    156, 8, 60,
    188, 4, 75,
    162, 8, 100,
    -1
  };

  draw_box_background(PatternStripedGreen);

  draw_custom_bld_box(bld_layout);

  draw_building_count(3, 48, Building::TypePigFarm);
  draw_building_count(11, 48, Building::TypeFarm);
  draw_building_count(0, 92, Building::TypeFisher);
  draw_building_count(11, 87, Building::TypeButcher);
  draw_building_count(5, 134, Building::TypeMill);
  draw_building_count(10, 134, Building::TypeBaker);

  draw_popup_icon(0, 128, 61); /* flip */
  draw_popup_icon(14, 128, 60); /* exit */
}

void
PopupBox::draw_stat_bld_4_box() {
  const int bld_layout[] = {
    163, 0, 4,
    164, 4, 4,
    165, 8, 4,
    166, 12, 4,
    161, 2, 90,
    159, 8, 90,
    -1
  };

  draw_box_background(PatternStripedGreen);

  draw_custom_bld_box(bld_layout);

  draw_building_count(0, 71, Building::TypeStoneMine);
  draw_building_count(4, 71, Building::TypeCoalMine);
  draw_building_count(8, 71, Building::TypeIronMine);
  draw_building_count(12, 71, Building::TypeGoldMine);
  draw_building_count(4, 130, Building::TypeSteelSmelter);
  draw_building_count(9, 130, Building::TypeGoldSmelter);

  draw_popup_icon(0, 128, 61); /* flip */
  draw_popup_icon(14, 128, 60); /* exit */
}

void
PopupBox::draw_player_stat_chart(const int *data, int index,
                                 const Color &color) {
  int lx = 8;
  int ly = 9;
  int lw = 112;
  int lh = 100;

  int prev_value = data[index];

  for (int i = 0; i < lw; i++) {
    int value = data[index];
    index = index > 0 ? index-1 : (lw-1);

    if (value > 0 || prev_value > 0) {
      if (value > prev_value) {
        int diff = value - prev_value;
        int h = diff/2;
        frame->fill_rect(lx + lw - i, ly + lh - h - prev_value, 1, h,
                         color);
        diff -= h;
        frame->fill_rect(lx + lw - i - 1, ly + lh - value, 1, diff, color);
      } else if (value == prev_value) {
        frame->fill_rect(lx + lw - i - 1, ly + lh - value, 2, 1, color);
      } else {
        int diff = prev_value - value;
        int h = diff/2;
        frame->fill_rect(lx + lw - i, ly + lh - prev_value, 1, h, color);
        diff -= h;
        frame->fill_rect(lx + lw - i - 1, ly + lh - value - diff, 1, diff,
                         color);
      }
    }

    prev_value = value;
  }
}

void
PopupBox::draw_stat_8_box() {
  const int layout[] = {
    0x58, 14, 0,
    0x59, 0, 100,
    0x41, 8, 112,
    0x42, 10, 112,
    0x43, 8, 128,
    0x44, 10, 128,
    0x45, 2, 112,
    0x40, 4, 112,
    0x3e, 2, 128,
    0x3f, 4, 128,
    0x133, 14, 112,

    0x3c, 14, 128, /* exit */
    -1
  };

  int mode = interface->get_current_stat_8_mode();
  int aspect = (mode >> 2) & 3;
  int scale = mode & 3;

  /* Draw background */
  draw_box_row(132+aspect, 0);
  draw_box_row(132+aspect, 16);
  draw_box_row(132+aspect, 32);
  draw_box_row(132+aspect, 48);
  draw_box_row(132+aspect, 64);
  draw_box_row(132+aspect, 80);
  draw_box_row(132+aspect, 96);

  draw_box_row(136, 108);
  draw_box_row(129, 116);
  draw_box_row(137, 132);

  draw_custom_icon_box(layout);

  /* Draw checkmarks to indicate current settings. */
  draw_popup_icon(!BIT_TEST(aspect, 0) ? 1 : 6,
                  !BIT_TEST(aspect, 1) ? 116 : 132, 106); /* checkmark */

  draw_popup_icon(!BIT_TEST(scale, 0) ? 7 : 12,
                  !BIT_TEST(scale, 1) ? 116 : 132, 106); /* checkmark */

  /* Correct numbers on time scale. */
  draw_popup_icon(2, 103, 94 + 3*scale + 0);
  draw_popup_icon(6, 103, 94 + 3*scale + 1);
  draw_popup_icon(10, 103, 94 + 3*scale + 2);

  /* Draw chart */
  PGame game = interface->get_game();
  int index = game->get_player_history_index(scale);
  for (int i = 0; i < GAME_MAX_PLAYER_COUNT; i++) {
    if (game->get_player(GAME_MAX_PLAYER_COUNT-i-1) != nullptr) {
      Player *player = game->get_player(GAME_MAX_PLAYER_COUNT-i-1);
      Color color = interface->get_player_color(GAME_MAX_PLAYER_COUNT-i-1);
      draw_player_stat_chart(player->get_player_stat_history(mode), index,
                             color);
    }
  }
}

void
PopupBox::draw_stat_7_box() {
  const int layout[] = {
    0x81, 6, 80,
    0x81, 8, 80,
    0x81, 6, 96,
    0x81, 8, 96,

    0x59, 0, 64,
    0x5a, 14, 0,

    0x28, 0, 75, /* lumber */
    0x29, 2, 75, /* plank */
    0x2b, 4, 75, /* stone */
    0x2e, 0, 91, /* coal */
    0x2c, 2, 91, /* ironore */
    0x2f, 4, 91, /* goldore */
    0x2a, 0, 107, /* boat */
    0x2d, 2, 107, /* iron */
    0x30, 4, 107, /* goldbar */
    0x3a, 7, 83, /* sword */
    0x3b, 7, 99, /* shield */
    0x31, 10, 75, /* shovel */
    0x32, 12, 75, /* hammer */
    0x36, 14, 75, /* axe */
    0x37, 10, 91, /* saw */
    0x38, 12, 91, /* pick */
    0x35, 14, 91, /* scythe */
    0x34, 10, 107, /* cleaver */
    0x39, 12, 107, /* pincer */
    0x33, 14, 107, /* rod */
    0x22, 1, 125, /* fish */
    0x23, 3, 125, /* pig */
    0x24, 5, 125, /* meat */
    0x25, 7, 125, /* wheat */
    0x26, 9, 125, /* flour */
    0x27, 11, 125, /* bread */

    0x3c, 14, 128, /* exitbox */
    -1
  };

  draw_box_row(129, 64);
  draw_box_row(129, 112);
  draw_box_row(129, 128);

  draw_custom_icon_box(layout);

  Resource::Type item = (Resource::Type)(current_stat_7_item-1);

  /* Draw background of chart */
  for (int iy = 0; iy < 64; iy += 16) {
    for (int ix = 0; ix < 14; ix += 2) {
      draw_popup_icon(ix, iy, 138 + item);
    }
  }

  const int sample_weights[] = { 4, 6, 8, 9, 10, 9, 8, 6, 4 };

  /* Create array of historical counts */
  int historical_data[112];
  int max_val = 0;
  int index = interface->get_game()->get_resource_history_index();

  for (int i = 0; i < 112; i++) {
    historical_data[i] = 0;
    int j = index;
    for (int k = 0; k < 9; k++) {
      historical_data[i] += sample_weights[k]*interface->
                              get_player()->get_resource_count_history(item)[j];
      j = j > 0 ? j-1 : 119;
    }

    if (historical_data[i] > max_val) {
      max_val = historical_data[i];
    }

    index = index > 0 ? index-1 : 119;
  }

  const int axis_icons_1[] = { 110, 109, 108, 107 };
  const int axis_icons_2[] = { 112, 111, 110, 108 };
  const int axis_icons_3[] = { 114, 113, 112, 110 };
  const int axis_icons_4[] = { 117, 116, 114, 112 };
  const int axis_icons_5[] = { 120, 119, 118, 115 };
  const int axis_icons_6[] = { 122, 121, 120, 118 };
  const int axis_icons_7[] = { 125, 124, 122, 120 };
  const int axis_icons_8[] = { 128, 127, 126, 123 };

  const int *axis_icons = nullptr;
  int multiplier = 0;

  /* TODO chart background pattern */

  if (max_val <= 64) {
    axis_icons = axis_icons_1;
    multiplier = 0x8000;
  } else if (max_val <= 128) {
    axis_icons = axis_icons_2;
    multiplier = 0x4000;
  } else if (max_val <= 256) {
    axis_icons = axis_icons_3;
    multiplier = 0x2000;
  } else if (max_val <= 512) {
    axis_icons = axis_icons_4;
    multiplier = 0x1000;
  } else if (max_val <= 1280) {
    axis_icons = axis_icons_5;
    multiplier = 0x666;
  } else if (max_val <= 2560) {
    axis_icons = axis_icons_6;
    multiplier = 0x333;
  } else if (max_val <= 5120) {
    axis_icons = axis_icons_7;
    multiplier = 0x199;
  } else {
    axis_icons = axis_icons_8;
    multiplier = 0xa3;
  }

  /* Draw axis icons */
  for (int i = 0; i < 4; i++) {
    draw_popup_icon(14, i*16, axis_icons[i]);
  }

  /* Draw chart */
  for (int i = 0; i < 112; i++) {
    int value = std::min((historical_data[i]*multiplier) >> 16, 64);
    if (value > 0) {
      frame->fill_rect(119 - i, 73 - value, 1, value, Color(0xcf, 0x63, 0x63));
    }
  }
}

void
PopupBox::draw_gauge_balance(int lx, int ly, unsigned int value,
                             unsigned int count) {
  int sprite = -1;
  if (count > 0) {
    unsigned int v = (16  *value) / count;
    if (v >= 230) {
      sprite = 0xd2;
    } else if (v >= 207) {
      sprite = 0xd1;
    } else if (v >= 184) {
      sprite = 0xd0;
    } else if (v >= 161) {
      sprite = 0xcf;
    } else if (v >= 138) {
      sprite = 0xce;
    } else if (v >= 115) {
      sprite = 0xcd;
    } else if (v >= 92) {
      sprite = 0xcc;
    } else if (v >= 69) {
      sprite = 0xcb;
    } else if (v >= 46) {
      sprite = 0xca;
    } else if (v >= 23) {
      sprite = 0xc9;
    } else {
      sprite = 0xc8;
    }
  }

  draw_popup_icon(lx, ly, sprite);
}

void
PopupBox::draw_gauge_full(int lx, int ly, unsigned int value,
                          unsigned int count) {
  unsigned int sprite = 0xc7;
  if (count > 0) {
    unsigned int v = (16 * value) / count;
    if (v >= 230) {
      sprite = 0xc6;
    } else if (v >= 207) {
      sprite = 0xc5;
    } else if (v >= 184) {
      sprite = 0xc4;
    } else if (v >= 161) {
      sprite = 0xc3;
    } else if (v >= 138) {
      sprite = 0xc2;
    } else if (v >= 115) {
      sprite = 0xc1;
    } else if (v >= 92) {
      sprite = 0xc0;
    } else if (v >= 69) {
      sprite = 0xbf;
    } else if (v >= 46) {
      sprite = 0xbe;
    } else if (v >= 23) {
      sprite = 0xbd;
    } else {
      sprite = 0xbc;
    }
  }

  draw_popup_icon(lx, ly, sprite);
}

static void
calculate_gauge_values(Player *player,
                       unsigned int values[24][Building::kMaxStock][2]) {
  for (Building *building : player->get_game()->get_player_buildings(player)) {
    if (building->is_burning() || !building->has_serf()) {
      continue;
    }

    int type = building->get_type();
    if (!building->is_done()) type = 0;

    for (int i = 0; i < Building::kMaxStock; i++) {
      if (building->get_maximum_in_stock(i) > 0) {
        int v = 2*building->get_res_count_in_stock(i) +
          building->get_requested_in_stock(i);
        values[type][i][0] += (16*v)/(2*building->get_maximum_in_stock(i));
        values[type][i][1] += 1;
      }
    }
  }
}

void
PopupBox::draw_stat_1_box() {
  const int layout[] = {
    0x18, 0, 0, /* baker */
    0xb4, 0, 16,
    0xb3, 0, 24,
    0xb2, 0, 32,
    0xb3, 0, 40,
    0xb2, 0, 48,
    0xb3, 0, 56,
    0xb2, 0, 64,
    0xb3, 0, 72,
    0xb2, 0, 80,
    0xb3, 0, 88,
    0xd4, 0, 96,
    0xb1, 0, 112,
    0x13, 0, 120, /* fisher */
    0x15, 2, 48, /* butcher */
    0xb4, 2, 64,
    0xb3, 2, 72,
    0xd4, 2, 80,
    0xa4, 2, 96,
    0xa4, 2, 112,
    0xae, 4, 4,
    0xae, 4, 36,
    0xa6, 4, 80,
    0xa6, 4, 96,
    0xa6, 4, 112,
    0x26, 6, 0, /* flour */
    0x23, 6, 32, /* pig */
    0xb5, 6, 64,
    0x24, 6, 76, /* meat */
    0x27, 6, 92, /* bread */
    0x22, 6, 108, /* fish */
    0xb6, 6, 124,
    0x17, 8, 0, /* miller */
    0x14, 8, 32, /* pigfarmer */
    0xa6, 8, 64,
    0xab, 8, 88,
    0xab, 8, 104,
    0xa6, 8, 128,
    0xba, 12, 8,
    0x11, 12, 56, /* miner */
    0x11, 12, 80, /* miner */
    0x11, 12, 104, /* miner */
    0x11, 12, 128, /* miner */
    0x16, 14, 0, /* farmer */
    0x25, 14, 16, /* wheat */
    0x2f, 14, 56, /* goldore */
    0x2e, 14, 80, /* coal */
    0x2c, 14, 104, /* ironore */
    0x2b, 14, 128, /* stone */
    -1
  };

  draw_box_background(PatternStripedGreen);

  draw_custom_icon_box(layout);

  unsigned int values[24][Building::kMaxStock][2] = {{{0}}};
  calculate_gauge_values(interface->get_player(), values);

  draw_gauge_balance(10, 0, values[Building::TypeMill][0][0],
                     values[Building::TypeMill][0][1]);
  draw_gauge_balance(2, 0, values[Building::TypeBaker][0][0],
                     values[Building::TypeBaker][0][1]);
  draw_gauge_full(10, 32, values[Building::TypePigFarm][0][0],
                  values[Building::TypePigFarm][0][1]);
  draw_gauge_balance(2, 32, values[Building::TypeButcher][0][0],
                     values[Building::TypeButcher][0][1]);
  draw_gauge_full(10, 56, values[Building::TypeGoldMine][0][0],
                  values[Building::TypeGoldMine][0][1]);
  draw_gauge_full(10, 80, values[Building::TypeCoalMine][0][0],
                  values[Building::TypeCoalMine][0][1]);
  draw_gauge_full(10, 104, values[Building::TypeIronMine][0][0],
                  values[Building::TypeIronMine][0][1]);
  draw_gauge_full(10, 128, values[Building::TypeStoneMine][0][0],
                  values[Building::TypeStoneMine][0][1]);
}

void
PopupBox::draw_stat_2_box() {
  const int layout[] = {
    0x11, 0, 0, /* miner */
    0x11, 0, 24, /* miner */
    0x11, 0, 56, /* miner */
    0xd, 0, 80, /* lumberjack */
    0x11, 0, 104, /* miner */
    0xf, 0, 128, /* stonecutter */
    0x2f, 2, 0, /* goldore */
    0x2e, 2, 24, /* coal */
    0xb0, 2, 40,
    0x2c, 2, 56, /* ironore */
    0x28, 2, 80, /* lumber */
    0x2b, 2, 104, /* stone */
    0x2b, 2, 128, /* stone */
    0xaa, 4, 4,
    0xab, 4, 24,
    0xad, 4, 32,
    0xa8, 4, 40,
    0xac, 4, 60,
    0xaa, 4, 84,
    0xbb, 4, 108,
    0xa4, 6, 32,
    0xe, 6, 96, /* sawmiller */
    0xa5, 6, 132,
    0x30, 8, 0, /* gold */
    0x12, 8, 16, /* smelter */
    0xa4, 8, 32,
    0x2d, 8, 40, /* steel */
    0x12, 8, 56, /* smelter */
    0xb8, 8, 80,
    0x29, 8, 96, /* planks */
    0xaf, 8, 112,
    0xa5, 8, 132,
    0xaa, 10, 4,
    0xb9, 10, 24,
    0xab, 10, 40,
    0xb7, 10, 48,
    0xa6, 10, 80,
    0xa9, 10, 96,
    0xa6, 10, 112,
    0xa7, 10, 132,
    0x21, 14, 0, /* knight 4 */
    0x1b, 14, 28, /* weaponsmith */
    0x1a, 14, 64, /* toolmaker */
    0x19, 14, 92, /* boatbuilder */
    0xc, 14, 120, /* builder */
    -1
  };

  draw_box_background(PatternStripedGreen);

  draw_custom_icon_box(layout);

  unsigned int values[24][Building::kMaxStock][2] = {{{0}}};
  calculate_gauge_values(interface->get_player(), values);

  draw_gauge_balance(6, 0, values[Building::TypeGoldSmelter][1][0],
                     values[Building::TypeGoldSmelter][1][1]);
  draw_gauge_balance(6, 16, values[Building::TypeGoldSmelter][0][0],
                     values[Building::TypeGoldSmelter][0][1]);
  draw_gauge_balance(6, 40, values[Building::TypeSteelSmelter][0][0],
                     values[Building::TypeSteelSmelter][0][1]);
  draw_gauge_balance(6, 56, values[Building::TypeSteelSmelter][1][0],
                     values[Building::TypeSteelSmelter][1][1]);

  draw_gauge_balance(6, 80, values[Building::TypeSawmill][1][0],
                     values[Building::TypeSawmill][1][1]);

  unsigned int gold_value = values[Building::TypeHut][1][0] +
                            values[Building::TypeTower][1][0] +
                            values[Building::TypeFortress][1][0];
  unsigned int gold_count = values[Building::TypeHut][1][1] +
                            values[Building::TypeTower][1][1] +
                            values[Building::TypeFortress][1][1];
  draw_gauge_full(12, 0, gold_value, gold_count);

  draw_gauge_balance(12, 20, values[Building::TypeWeaponSmith][0][0],
                     values[Building::TypeWeaponSmith][0][1]);
  draw_gauge_balance(12, 36, values[Building::TypeWeaponSmith][1][0],
                     values[Building::TypeWeaponSmith][1][1]);

  draw_gauge_balance(12, 56, values[Building::TypeToolMaker][1][0],
                     values[Building::TypeToolMaker][1][1]);
  draw_gauge_balance(12, 72, values[Building::TypeToolMaker][0][0],
                     values[Building::TypeToolMaker][0][1]);

  draw_gauge_balance(12, 92, values[Building::TypeBoatbuilder][0][0],
                     values[Building::TypeBoatbuilder][0][1]);

  draw_gauge_full(12, 112, values[0][0][0], values[0][0][1]);
  draw_gauge_full(12, 128, values[0][1][0], values[0][1][1]);
}

void
PopupBox::draw_stat_6_box() {
  draw_box_background(PatternStripedGreen);

  int total = 0;
  for (int i = 0; i < 27; i++) {
    if (i != Serf::TypeTransporterInventory) {
      total += interface->get_player()->get_serf_count(i);
    }
  }

  draw_serfs_box(interface->get_player()->get_serfs(), total);

  draw_popup_icon(14, 128, 60); /* exit */
}

void
PopupBox::draw_stat_3_meter(int lx, int ly, int value) {
  unsigned int sprite = 0xc6;
  if (value < 1) {
    sprite = 0xbc;
  } else if (value < 2) {
    sprite = 0xbe;
  } else if (value < 3) {
    sprite = 0xc0;
  } else if (value < 4) {
    sprite = 0xc1;
  } else if (value < 5) {
    sprite = 0xc2;
  } else if (value < 7) {
    sprite = 0xc3;
  } else if (value < 10) {
    sprite = 0xc4;
  } else if (value < 20) {
    sprite = 0xc5;
  }
  draw_popup_icon(lx, ly, sprite);
}

void
PopupBox::draw_stat_3_box() {
  draw_box_background(PatternStripedGreen);

  Serf::SerfMap serfs = interface->get_player()->get_stats_serfs_idle();
  Serf::SerfMap serfs_potential =
                           interface->get_player()->get_stats_serfs_potential();
  for (int i = 0; i < 27; ++i) {
    serfs[(Serf::Type)i] += serfs_potential[(Serf::Type)i];
  }

  const int layout[] = {
    0x9, 1, 0, /* serfs */
    0xa, 1, 16,
    0xb, 1, 32,
    0xc, 1, 48,
    0x21, 1, 64,
    0x20, 1, 80,
    0x1f, 1, 96,
    0x1e, 1, 112,
    0x1d, 1, 128,
    0xd, 6, 0,
    0xe, 6, 16,
    0x12, 6, 32,
    0xf, 6, 48,
    0x10, 6, 64,
    0x11, 6, 80,
    0x19, 6, 96,
    0x1a, 6, 112,
    0x1b, 6, 128,
    0x13, 11, 0,
    0x14, 11, 16,
    0x15, 11, 32,
    0x16, 11, 48,
    0x17, 11, 64,
    0x18, 11, 80,
    0x1c, 11, 96,
    0x82, 11, 112,
    -1
  };

  draw_custom_icon_box(layout);

  /* First column */
  draw_stat_3_meter(3, 0, serfs[Serf::TypeTransporter]);
  draw_stat_3_meter(3, 16, serfs[Serf::TypeSailor]);
  draw_stat_3_meter(3, 32, serfs[Serf::TypeDigger]);
  draw_stat_3_meter(3, 48, serfs[Serf::TypeBuilder]);
  draw_stat_3_meter(3, 64, serfs[Serf::TypeKnight4]);
  draw_stat_3_meter(3, 80, serfs[Serf::TypeKnight3]);
  draw_stat_3_meter(3, 96, serfs[Serf::TypeKnight2]);
  draw_stat_3_meter(3, 112, serfs[Serf::TypeKnight1]);
  draw_stat_3_meter(3, 128, serfs[Serf::TypeKnight0]);

  /* Second column */
  draw_stat_3_meter(8, 0, serfs[Serf::TypeLumberjack]);
  draw_stat_3_meter(8, 16, serfs[Serf::TypeSawmiller]);
  draw_stat_3_meter(8, 32, serfs[Serf::TypeSmelter]);
  draw_stat_3_meter(8, 48, serfs[Serf::TypeStonecutter]);
  draw_stat_3_meter(8, 64, serfs[Serf::TypeForester]);
  draw_stat_3_meter(8, 80, serfs[Serf::TypeMiner]);
  draw_stat_3_meter(8, 96, serfs[Serf::TypeBoatBuilder]);
  draw_stat_3_meter(8, 112, serfs[Serf::TypeToolmaker]);
  draw_stat_3_meter(8, 128, serfs[Serf::TypeWeaponSmith]);

  /* Third column */
  draw_stat_3_meter(13, 0, serfs[Serf::TypeFisher]);
  draw_stat_3_meter(13, 16, serfs[Serf::TypePigFarmer]);
  draw_stat_3_meter(13, 32, serfs[Serf::TypeButcher]);
  draw_stat_3_meter(13, 48, serfs[Serf::TypeFarmer]);
  draw_stat_3_meter(13, 64, serfs[Serf::TypeMiller]);
  draw_stat_3_meter(13, 80, serfs[Serf::TypeBaker]);
  draw_stat_3_meter(13, 96, serfs[Serf::TypeGeologist]);
  draw_stat_3_meter(13, 112, serfs[Serf::TypeGeneric]);

  draw_popup_icon(14, 128, 60); /* exit */
}

void
PopupBox::draw_start_attack_redraw_box() {
  /* TODO Should overwrite the previously drawn number.
     Just use fill_rect(), perhaps? */
  draw_green_string(6, 116, "    ");
  draw_green_number(7, 116, interface->get_player()->knights_attacking);
}

void
PopupBox::draw_start_attack_box() {
  const int building_layout[] = {
    0x0, 2, 33,
    0xa, 6, 30,
    0x7, 10, 33,
    0xc, 14, 30,
    0xe, 2, 36,
    0x2, 6, 39,
    0xb, 10, 36,
    0x4, 12, 39,
    0x8, 8, 42,
    0xf, 12, 42,
    -1
  };

  const int icon_layout[] = {
    216, 1, 80,
    217, 5, 80,
    218, 9, 80,
    219, 13, 80,
    220, 4, 112,
    221, 10, 112,
    222, 0, 128,
    60, 14, 128,
    -1
  };

  draw_box_background(PatternConstruction);

  for (int i = 0; building_layout[i] >= 0; i += 3) {
    draw_popup_building(building_layout[i+1], building_layout[i+2],
                        building_layout[i]);
  }

  Building *building = interface->get_game()->get_building(
                                    interface->get_player()->building_attacked);
  int ly = 0;

  switch (building->get_type()) {
    case Building::TypeHut: ly = 50; break;
    case Building::TypeTower: ly = 32; break;
    case Building::TypeFortress: ly = 17; break;
    case Building::TypeCastle: ly = 0; break;
    default: NOT_REACHED(); break;
  }

  draw_popup_building(0, ly, map_building_sprite[building->get_type()]);
  draw_custom_icon_box(icon_layout);

  /* Draw number of knight at each distance. */
  for (int i = 0; i < 4; i++) {
    draw_green_number(1+4*i, 96, interface->get_player()->attacking_knights[i]);
  }

  draw_start_attack_redraw_box();
}

void
PopupBox::draw_sett_select_box() {
  const int layout[] = {
    230, 1, 8,
    231, 6, 8,
    232, 11, 8,
    234, 1, 48,
    235, 6, 48,
    299, 11, 48,
    233, 1, 88,
    298, 6, 88,
    61, 12, 104,
    60, 14, 128,

    285, 4, 128,
    286, 0, 128,
    224, 8, 128,
    -1
  };

  draw_box_background(PatternCheckerdDiagonalBrown);
  draw_custom_icon_box(layout);
}

/* Draw slide bar in a popup box. */
void
PopupBox::draw_slide_bar(int lx, int ly, int value) {
  draw_popup_icon(lx, ly, 236);

  int lwidth = value / 1310;
  if (lwidth > 0) {
    frame->fill_rect(8*lx+15, ly+11, lwidth, 4, Color(0x6b, 0xab, 0x3b));
  }
}

void
PopupBox::draw_sett_1_box() {
  const int bld_layout[] = {
    163, 12, 21,
    164, 8, 41,
    165, 4, 61,
    166, 0, 81,
    -1
  };

  const int layout[] = {
    34, 4, 1,
    36, 7, 1,
    39, 10, 1,
    60, 14, 128,
    295, 1, 8,
    -1
  };

  draw_box_background(PatternCheckerdDiagonalBrown);
  draw_custom_bld_box(bld_layout);
  draw_custom_icon_box(layout);

  Player *player = interface->get_player();

  unsigned int prio_layout[] = {
    4,  21, Building::TypeStoneMine,
    0,  41, Building::TypeCoalMine,
    8, 114, Building::TypeIronMine,
    4, 133, Building::TypeGoldMine
  };

  for (int i = 0; i < 4; i++) {
    draw_slide_bar(prio_layout[i * 3], prio_layout[i * 3 + 1],
                   player->get_food_for_building(prio_layout[i * 3 + 2]));
  }
}

void
PopupBox::draw_sett_2_box() {
  const int bld_layout[] = {
    186, 2, 0,
    174, 2, 41,
    153, 8, 54,
    157, 0, 102,
    -1
  };

  const int layout[] = {
    41, 9, 25,
    45, 9, 119,
    60, 14, 128,
    295, 13, 8,
    -1
  };

  draw_box_background(PatternCheckerdDiagonalBrown);
  draw_custom_bld_box(bld_layout);
  draw_custom_icon_box(layout);

  Player *player = interface->get_player();

  draw_slide_bar(0, 26, player->get_planks_construction());
  draw_slide_bar(0, 36, player->get_planks_boatbuilder());
  draw_slide_bar(8, 44, player->get_planks_toolmaker());
  draw_slide_bar(8, 103, player->get_steel_toolmaker());
  draw_slide_bar(0, 130, player->get_steel_weaponsmith());
}

void
PopupBox::draw_sett_3_box() {
  const int bld_layout[] = {
    161, 0, 1,
    159, 10, 0,
    157, 4, 56,
    188, 12, 61,
    155, 0, 101,
    -1
  };

  const int layout[] = {
    46, 7, 19,
    37, 8, 101,
    60, 14, 128,
    295, 1, 60,
    -1
  };

  draw_box_background(PatternCheckerdDiagonalBrown);
  draw_custom_bld_box(bld_layout);
  draw_custom_icon_box(layout);

  Player *player = interface->get_player();

  draw_slide_bar(0, 39, player->get_coal_steelsmelter());
  draw_slide_bar(8, 39, player->get_coal_goldsmelter());
  draw_slide_bar(4, 47, player->get_coal_weaponsmith());
  draw_slide_bar(0, 92, player->get_wheat_pigfarm());
  draw_slide_bar(8, 118, player->get_wheat_mill());
}

void
PopupBox::draw_knight_level_box() {
  const char *level_str[] = {
    "Minimum", "Weak", "Medium", "Good", "Full", "ERROR", "ERROR", "ERROR",
  };

  const int layout[] = {
    226, 0, 2,
    227, 0, 36,
    228, 0, 70,
    229, 0, 104,

    220, 4, 2,  /* minus */
    221, 6, 2,  /* plus */
    220, 4, 18, /* ... */
    221, 6, 18,
    220, 4, 36,
    221, 6, 36,
    220, 4, 52,
    221, 6, 52,
    220, 4, 70,
    221, 6, 70,
    220, 4, 86,
    221, 6, 86,
    220, 4, 104,
    221, 6, 104,
    220, 4, 120,
    221, 6, 120,

    60, 14, 128, /* exit */
    -1
  };

  draw_box_background(PatternCheckerdDiagonalBrown);

  Player *player = interface->get_player();

  for (int i = 0; i < 4; i++) {
    int ly = 8 + (34*i);
    draw_green_string(8, ly,
                      level_str[(player->get_knight_occupation(3-i) >> 4) &
                                0x7]);
    draw_green_string(8, ly + 11,
                      level_str[player->get_knight_occupation(3-i) & 0x7]);
  }

  draw_custom_icon_box(layout);
}

void
PopupBox::draw_sett_4_box() {
  const int layout[] = {
    49, 1, 0, /* shovel */
    50, 1, 16, /* hammer */
    54, 1, 32, /* axe */
    55, 1, 48, /* saw */
    53, 1, 64, /* scythe */
    56, 1, 80, /* pick */
    57, 1, 96, /* pincer */
    52, 1, 112, /* cleaver */
    51, 1, 128, /* rod */

    60, 14, 128, /* exit*/
    295, 13, 8, /* default */
    -1
  };

  draw_box_background(PatternCheckerdDiagonalBrown);
  draw_custom_icon_box(layout);

  Player *player = interface->get_player();
  draw_slide_bar(4, 4, player->get_tool_prio(0)); /* shovel */
  draw_slide_bar(4, 20, player->get_tool_prio(1)); /* hammer */
  draw_slide_bar(4, 36, player->get_tool_prio(5)); /* axe */
  draw_slide_bar(4, 52, player->get_tool_prio(6)); /* saw */
  draw_slide_bar(4, 68, player->get_tool_prio(4)); /* scythe */
  draw_slide_bar(4, 84, player->get_tool_prio(7)); /* pick */
  draw_slide_bar(4, 100, player->get_tool_prio(8)); /* pincer */
  draw_slide_bar(4, 116, player->get_tool_prio(3)); /* cleaver */
  draw_slide_bar(4, 132, player->get_tool_prio(2)); /* rod */
}

/* Draw generic popup box of resource stairs. */
void
PopupBox::draw_popup_resource_stairs(int order[]) {
  const int spiral_layout[] = {
    5, 4,
    7, 6,
    9, 8,
    11, 10,
    13, 12,
    13, 28,
    11, 30,
    9, 32,
    7, 34,
    5, 36,
    3, 38,
    1, 40,
    1, 56,
    3, 58,
    5, 60,
    7, 62,
    9, 64,
    11, 66,
    13, 68,
    13, 84,
    11, 86,
    9, 88,
    7, 90,
    5, 92,
    3, 94,
    1, 96
  };

  for (int i = 0; i < 26; i++) {
    int pos = 26 - order[i];
    draw_popup_icon(spiral_layout[2*pos], spiral_layout[2*pos+1], 34+i);
  }
}

void
PopupBox::draw_sett_5_box() {
  const int layout[] = {
    237, 1, 120, /* up */
    238, 3, 120, /* smallup */
    239, 9, 120, /* smalldown */
    240, 11, 120, /* down */
    295, 1, 4, /* default*/
    60, 14, 128, /* exit */
    -1
  };

  draw_box_background(PatternCheckerdDiagonalBrown);
  draw_custom_icon_box(layout);
  draw_popup_resource_stairs(interface->get_player()->get_flag_prio());

  draw_popup_icon(6, 120, 33+current_sett_5_item);
}

void
PopupBox::draw_no_save_quit_confirm_box() {
  draw_green_string(0, 70, "The game has not");
  draw_green_string(0, 80, "   been saved");
  draw_green_string(0, 90, "   recently.");
  draw_green_string(0, 100, "    Are you");
  draw_green_string(0, 110, "     sure?");
  draw_green_string(0, 125, "  Yes       No");
}

void
PopupBox::draw_options_box() {
  draw_box_background(PatternDiagonalGreen);

  draw_green_string(1, 14, "Music");
  draw_green_string(1, 30, "Sound");
  draw_green_string(1, 39, "effects");
  draw_green_string(1, 54, "Volume");

  Audio &audio = Audio::get_instance();
  Audio::PPlayer player = audio.get_music_player();
  // Music
  draw_popup_icon(13, 10, ((player) && player->is_enabled()) ? 288 : 220);
  player = audio.get_sound_player();
  // Sfx
  draw_popup_icon(13, 30, ((player) && player->is_enabled()) ? 288 :220);
  draw_popup_icon(11, 50, 220); /* Volume minus */
  draw_popup_icon(13, 50, 221); /* Volume plus */

  float volume = 0.f;
  Audio::VolumeController *volume_controller = audio.get_volume_controller();
  if (volume_controller != nullptr) {
    volume = 99.f * volume_controller->get_volume();
  }
  std::stringstream str;
  str << static_cast<int>(volume);
  draw_green_string(8, 54, str.str());

  draw_green_string(1, 70, "Fullscreen");
  draw_green_string(1, 79, "video");

  draw_popup_icon(13, 70,   /* Fullscreen mode */
                  Graphics::get_instance().is_fullscreen() ? 288 : 220);

  const char *value = "All";
  if (!interface->get_config(3)) {
    value = "Most";
    if (!interface->get_config(4)) {
      value = "Few";
      if (!interface->get_config(5)) {
        value = "None";
      }
    }
  }
  draw_green_string(1, 94, "Messages");
  draw_green_string(11, 94, value);

  draw_popup_icon(14, 128, 60); /* exit */
}

void
PopupBox::draw_transport_info_box() {
  const int layout[] = {
    9, 24,
    5, 24,
    3, 44,
    5, 64,
    9, 64,
    11, 44
  };

  draw_box_background(PatternPlaidAlongGreen);

  /* TODO show path merge button. */
  /* if (r == 0) draw_popup_icon(7, 51, 0x135); */

  if (interface->get_player()->temp_index == 0) {
    close();
    return;
  }

  Flag *flag =
           interface->get_game()->get_flag(interface->get_player()->temp_index);

#if 1
  /* Draw viewport of flag */
  Viewport flag_view(interface, interface->get_game()->get_map());
  flag_view.switch_layer(Viewport::LayerLandscape);
  flag_view.switch_layer(Viewport::LayerSerfs);
  flag_view.switch_layer(Viewport::LayerCursor);
  flag_view.set_displayed(true);

  flag_view.set_parent(shared_from_this());
  flag_view.set_size(128, 64);
  flag_view.move_to_map_pos(flag->get_position());
  flag_view.move_by_pixels(0, -10);

  flag_view.move_to(8, 24);
  flag_view.draw(frame);
#else
  /* Static flag */
  draw_popup_building(8, 40, 0x80 + 4*popup->interface->player->player_num);
#endif

  for (Direction d : cycle_directions_cw()) {
    int index = 5 - d;
    int lx = layout[2*index];
    int ly = layout[2*index + 1];
    if (flag->has_path(d)) {
      int sprite = 0xdc; /* Minus box */
      if (flag->has_transporter(d)) {
        sprite = 0x120; /* Check box */
      }
      draw_popup_icon(lx, ly, sprite);
    }
  }

  draw_green_string(0, 4, "Transport Info:");
  draw_popup_icon(2, 96, 0x1c); /* Geologist */
  draw_popup_icon(14, 128, 0x3c); /* Exit box */

  /* Draw list of resources */
  for (int i = 0; i < FLAG_MAX_RES_COUNT; i++) {
    if (flag->get_resource_at_slot(i) != Resource::TypeNone) {
      draw_popup_icon(7 + 2*(i&3), 88 + 16*(i>>2),
                      0x22 + flag->get_resource_at_slot(i));
    }
  }

  draw_green_string(0, 128, "Index:");
  draw_green_number(7, 128, static_cast<int>(flag->get_index()));
}

void

PopupBox::draw_sett_8_box() {
  const int layout[] = {
    9, 2, 8,
    29, 12, 8,
    300, 2, 28,
    59, 7, 44,
    130, 8, 28,
    58, 9, 44,
    304, 3, 64,
    303, 11, 64,
    302, 2, 84,
    220, 6, 84,
    220, 6, 100,
    301, 10, 84,
    220, 3, 120,
    221, 9, 120,
    60, 14, 128,
    -1
  };

  draw_box_background(PatternCheckerdDiagonalBrown);
  draw_custom_icon_box(layout);

  Player *player = interface->get_player();

  draw_slide_bar(4, 12, player->get_serf_to_knight_rate());
  draw_green_string(8, 63, "%");
  draw_green_number(6, 63, (100*player->get_knight_morale())/0x1000);

  draw_green_large_number(6, 73, player->get_gold_deposited());

  draw_green_number(6, 119, player->get_castle_knights_wanted());
  draw_green_number(6, 129, player->get_castle_knights());

  if (!player->send_strongest()) {
    draw_popup_icon(6, 84, 288); /* checkbox */
  } else {
    draw_popup_icon(6, 100, 288); /* checkbox */
  }

  size_t convertible_to_knights = 0;
  for (Inventory *inv : interface->get_game()->get_player_inventories(player)) {
    size_t c = std::min(inv->get_count_of(Resource::TypeSword),
                        inv->get_count_of(Resource::TypeShield));
    convertible_to_knights += std::max((size_t)0,
                                       std::min(c, inv->free_serf_count()));
  }

  draw_green_number(12, 40, static_cast<int>(convertible_to_knights));
}

void
PopupBox::draw_sett_6_box() {
  const int layout[] = {
    237, 1, 120,
    238, 3, 120,
    239, 9, 120,
    240, 11, 120,

    295, 1, 4, /* default */
    60, 14, 128, /* exit */
    -1
  };

  draw_box_background(PatternCheckerdDiagonalBrown);
  draw_custom_icon_box(layout);
  draw_popup_resource_stairs(interface->get_player()->get_inventory_prio());

  draw_popup_icon(6, 120, 33+current_sett_6_item);
}

void
PopupBox::draw_bld_1_box() {
  const int layout[] = {
    0xc0, 0, 5, /* stock */
    0xab, 2, 77, /* hut */
    0x9e, 8, 7, /* tower */
    0x98, 6, 69, /* fortress */
    -1
  };

  draw_box_background(PatternStaresGreen);

  draw_popup_building(4, 112, 0x80 + 4*interface->get_player()->get_index());
  draw_custom_bld_box(layout);

  draw_popup_icon(0, 128, 0x3d); /* flipbox */
  draw_popup_icon(14, 128, 0x3c); /* exit */
}

void
PopupBox::draw_bld_2_box() {
  const int layout[] = {
    153, 0, 4,
    160, 8, 6,
    157, 0, 68,
    169, 8, 65,
    174, 12, 57,
    170, 4, 105,
    168, 8, 107,
    -1
  };

  draw_box_background(PatternStaresGreen);

  draw_custom_bld_box(layout);

  draw_popup_icon(0, 128, 0x3d); /* flipbox */
  draw_popup_icon(14, 128, 0x3c); /* exit */
}

void
PopupBox::draw_bld_3_box() {
  const int layout[] = {
    155, 0, 2,
    154, 8, 3,
    167, 0, 61,
    156, 8, 60,
    188, 4, 75,
    162, 8, 100,
    -1
  };

  draw_box_background(PatternStaresGreen);

  draw_custom_bld_box(layout);

  draw_popup_icon(0, 128, 0x3d); /* flipbox */
  draw_popup_icon(14, 128, 0x3c); /* exit */
}

void
PopupBox::draw_bld_4_box() {
  const int layout[] = {
    163, 0, 4,
    164, 4, 4,
    165, 8, 4,
    166, 12, 4,
    161, 2, 90,
    159, 8, 90,
    -1
  };

  draw_box_background(PatternStaresGreen);

  draw_custom_bld_box(layout);

  draw_popup_icon(0, 128, 0x3d); /* flipbox */
  draw_popup_icon(14, 128, 0x3c); /* exit */
}

void
PopupBox::draw_building_stock_box() {
  draw_box_background(PatternPlaidAlongGreen);

  if (interface->get_player()->temp_index == 0) {
    close();
    return;
  }

  Building *building = interface->get_game()->get_building(
                                           interface->get_player()->temp_index);
  if (building->is_burning()) {
    close();
    return;
  }

  /* Draw list of resources */
  for (unsigned int j = 0; j < Building::kMaxStock; j++) {
    if (building->is_stock_active(j)) {
      int stock = building->get_res_count_in_stock(j);
      if (stock > 0) {
        int sprite = 34 + building->get_res_type_in_stock(j);
        for (int i = 0; i < stock; i++) {
          draw_popup_icon(8-stock+2*i, 110 - j*20, sprite);
        }
      } else {
        draw_popup_icon(7, 110 - j*20, 0xdc); /* minus box */
      }
    }
  }

  const int map_building_serf_sprite[] = {
    -1, 0x13, 0xd, 0x19,
    0xf, -1, -1, -1,
    -1, 0x10, -1, -1,
    0x16, 0x15, 0x14, 0x17,
    0x18, 0xe, 0x12, 0x1a,
    0x1b, -1, -1, 0x12,
    -1
  };

  /* Draw picture of serf present */
  int serf_sprite = 0xdc; /* minus box */
  if (building->has_serf()) {
    serf_sprite = map_building_serf_sprite[building->get_type()];
  }

  draw_popup_icon(1, 36, serf_sprite);

  /* Draw building */
  int bld_sprite = map_building_sprite[building->get_type()];
  int lx = 6;
  if (bld_sprite == 0xc0 /*stock*/ || bld_sprite < 0x9e /*tower*/) lx = 4;
  draw_popup_building(lx, 30, bld_sprite);

  draw_green_string(1, 4, "Stock of");
  draw_green_string(1, 14, "this building:");

  draw_popup_icon(14, 128, 0x3c); /* exit box */
}

void
PopupBox::draw_player_faces_box() {
  draw_box_background(PatternStripedGreen);

  draw_player_face(2, 4, 0);
  draw_player_face(10, 4, 1);
  draw_player_face(2, 76, 2);
  draw_player_face(10, 76, 3);
}

void
PopupBox::draw_demolish_box() {
  draw_box_background(PatternSquaresGreen);

  draw_popup_icon(14, 128, 60); /* Exit */
  draw_popup_icon(7, 45, 288); /* Checkbox */

  draw_green_string(0, 10, "    Demolish:");
  draw_green_string(0, 30, "   Click here");
  draw_green_string(0, 68, "   if you are");
  draw_green_string(0, 86, "      sure");
}

void
PopupBox::draw_save_box() {
  const int layout[] = {
    224, 0, 128,
    -1
  };

  draw_box_background(PatternDiagonalGreen);
  draw_custom_icon_box(layout);

  draw_green_string(3, 2, "Save  Game");

  draw_popup_icon(14, 128, 60); /* Exit */
}

void
PopupBox::internal_draw() {
  draw_popup_box_frame();

  /* Dispatch to one of the popup box functions above. */
  switch (box) {
  case TypeStatSelect:
    draw_stat_select_box();
    break;
  case TypeStat4:
    draw_stat_4_box();
    break;
  case TypeStatBld1:
    draw_stat_bld_1_box();
    break;
  case TypeStatBld2:
    draw_stat_bld_2_box();
    break;
  case TypeStatBld3:
    draw_stat_bld_3_box();
    break;
  case TypeStatBld4:
    draw_stat_bld_4_box();
    break;
  case TypeStat8:
    draw_stat_8_box();
    break;
  case TypeStat7:
    draw_stat_7_box();
    break;
  case TypeStat1:
    draw_stat_1_box();
    break;
  case TypeStat2:
    draw_stat_2_box();
    break;
  case TypeStat6:
    draw_stat_6_box();
    break;
  case TypeStat3:
    draw_stat_3_box();
    break;
  case TypeStartAttack:
    draw_start_attack_box();
    break;
  case TypeStartAttackRedraw:
    draw_start_attack_redraw_box();
    break;
  case TypeSettSelect:
    draw_sett_select_box();
    break;
  case TypeSett1:
    draw_sett_1_box();
    break;
  case TypeSett2:
    draw_sett_2_box();
    break;
  case TypeSett3:
    draw_sett_3_box();
    break;
  case TypeKnightLevel:
    draw_knight_level_box();
    break;
  case TypeSett4:
    draw_sett_4_box();
    break;
  case TypeSett5:
    draw_sett_5_box();
    break;
  case TypeNoSaveQuitConfirm:
    draw_no_save_quit_confirm_box();
    break;
  case TypeOptions:
    draw_options_box();
    break;
  case TypeTransportInfo:
    draw_transport_info_box();
    break;
  case TypeSett8:
    draw_sett_8_box();
    break;
  case TypeSett6:
    draw_sett_6_box();
    break;
  case TypeBld1:
    draw_bld_1_box();
    break;
  case TypeBld2:
    draw_bld_2_box();
    break;
  case TypeBld3:
    draw_bld_3_box();
    break;
  case TypeBld4:
    draw_bld_4_box();
    break;
  case TypeBldStock:
    draw_building_stock_box();
    break;
  case TypePlayerFaces:
    draw_player_faces_box();
    break;
  case TypeDemolish:
    draw_demolish_box();
    break;
  case TypeLoadSave:
    draw_save_box();
    break;
  default:
    break;
  }
}

void
PopupBox::activate_sett_5_6_item(int index) {
  if (box == TypeSett5) {
    int i;
    for (i = 0; i < 26; i++) {
      if (interface->get_player()->get_flag_prio(i) == index) break;
    }
    current_sett_5_item = i+1;
  } else {
    int i;
    for (i = 0; i < 26; i++) {
      if (interface->get_player()->get_inventory_prio(i) == index) break;
    }
    current_sett_6_item = i+1;
  }
}

void
PopupBox::move_sett_5_6_item(int up, int to_end) {
  int *prio = nullptr;
  int cur = -1;

  if (interface->get_popup_box()->get_box() == TypeSett5) {
    prio = interface->get_player()->get_flag_prio();
    cur = current_sett_5_item-1;
  } else {
    prio = interface->get_player()->get_inventory_prio();
    cur = current_sett_6_item-1;
  }

  int cur_value = prio[cur];
  int next_value = -1;
  if (up) {
    if (to_end) {
      next_value = 26;
    } else {
      next_value = cur_value + 1;
    }
  } else {
    if (to_end) {
      next_value = 1;
    } else {
      next_value = cur_value - 1;
    }
  }

  if (next_value >= 1 && next_value < 27) {
    int delta = next_value > cur_value ? -1 : 1;
    int min = next_value > cur_value ? cur_value+1 : next_value;
    int max = next_value > cur_value ? next_value : cur_value-1;
    for (int i = 0; i < 26; i++) {
      if (prio[i] >= min && prio[i] <= max) prio[i] += delta;
    }
    prio[cur] = next_value;
  }
}

void
PopupBox::handle_send_geologist() {
  MapPos pos = interface->get_map_cursor_pos();
  Flag *flag = interface->get_game()->get_flag_at_pos(pos);

  if (!interface->get_game()->send_geologist(flag)) {
    play_sound(Audio::TypeSfxNotAccepted);
  } else {
    play_sound(Audio::TypeSfxAccepted);
    close();
  }
}

void
PopupBox::sett_8_train(int number) {
  int r = interface->get_player()->promote_serfs_to_knights(number);

  if (r == 0) {
    play_sound(Audio::TypeSfxNotAccepted);
  } else {
    play_sound(Audio::TypeSfxAccepted);
  }
}

void
PopupBox::set_inventory_resource_mode(int mode) {
  Building *building = interface->get_game()->get_building(
                                           interface->get_player()->temp_index);
  Inventory *inventory = building->get_inventory();
  interface->get_game()->set_inventory_resource_mode(inventory, mode);
}

void
PopupBox::set_inventory_serf_mode(int mode) {
  Building *building = interface->get_game()->get_building(
                                           interface->get_player()->temp_index);
  Inventory *inventory = building->get_inventory();
  interface->get_game()->set_inventory_serf_mode(inventory, mode);
}

void
PopupBox::handle_action(int action, int x_, int /*y_*/) {
  set_redraw();

  Player *player = interface->get_player();

  switch (action) {
  case ACTION_SHOW_STAT_1:
    set_box(TypeStat1);
    break;
  case ACTION_SHOW_STAT_2:
    set_box(TypeStat2);
    break;
  case ACTION_SHOW_STAT_8:
    set_box(TypeStat8);
    break;
  case ACTION_SHOW_STAT_BLD:
    set_box(TypeStatBld1);
    break;
  case ACTION_SHOW_STAT_6:
    set_box(TypeStat6);
    break;
  case ACTION_SHOW_STAT_7:
    set_box(TypeStat7);
    break;
  case ACTION_SHOW_STAT_4:
    set_box(TypeStat4);
    break;
  case ACTION_SHOW_STAT_3:
    set_box(TypeStat3);
    break;
  case ACTION_SHOW_STAT_SELECT:
    set_box(TypeStatSelect);
    break;
  case ACTION_STAT_BLD_FLIP:
    set_box((box + 1 <= TypeStatBld4) ? (Type)(box + 1) : TypeStatBld1);
    break;
  case ACTION_CLOSE_BOX:
  case ACTION_CLOSE_SETT_BOX:
    close();
    break;
  case ACTION_SETT_8_SET_ASPECT_ALL:
    interface->set_current_stat_8_mode((0 << 2) |
                                    (interface->get_current_stat_8_mode() & 3));
    break;
  case ACTION_SETT_8_SET_ASPECT_LAND:
    interface->set_current_stat_8_mode((1 << 2) |
                                    (interface->get_current_stat_8_mode() & 3));
    break;
  case ACTION_SETT_8_SET_ASPECT_BUILDINGS:
    interface->set_current_stat_8_mode((2 << 2) |
                                    (interface->get_current_stat_8_mode() & 3));
    break;
  case ACTION_SETT_8_SET_ASPECT_MILITARY:
    interface->set_current_stat_8_mode((3 << 2) |
                                    (interface->get_current_stat_8_mode() & 3));
    break;
  case ACTION_SETT_8_SET_SCALE_30_MIN:
    interface->set_current_stat_8_mode(
                              (interface->get_current_stat_8_mode() & 0xc) | 0);
    break;
  case ACTION_SETT_8_SET_SCALE_60_MIN:
    interface->set_current_stat_8_mode(
                              (interface->get_current_stat_8_mode() & 0xc) | 1);
    break;
  case ACTION_SETT_8_SET_SCALE_600_MIN:
    interface->set_current_stat_8_mode(
                              (interface->get_current_stat_8_mode() & 0xc) | 2);
    break;
  case ACTION_SETT_8_SET_SCALE_3000_MIN:
    interface->set_current_stat_8_mode(
                              (interface->get_current_stat_8_mode() & 0xc) | 3);
    break;
  case ACTION_STAT_7_SELECT_FISH:
  case ACTION_STAT_7_SELECT_PIG:
  case ACTION_STAT_7_SELECT_MEAT:
  case ACTION_STAT_7_SELECT_WHEAT:
  case ACTION_STAT_7_SELECT_FLOUR:
  case ACTION_STAT_7_SELECT_BREAD:
  case ACTION_STAT_7_SELECT_LUMBER:
  case ACTION_STAT_7_SELECT_PLANK:
  case ACTION_STAT_7_SELECT_BOAT:
  case ACTION_STAT_7_SELECT_STONE:
  case ACTION_STAT_7_SELECT_IRONORE:
  case ACTION_STAT_7_SELECT_STEEL:
  case ACTION_STAT_7_SELECT_COAL:
  case ACTION_STAT_7_SELECT_GOLDORE:
  case ACTION_STAT_7_SELECT_GOLDBAR:
  case ACTION_STAT_7_SELECT_SHOVEL:
  case ACTION_STAT_7_SELECT_HAMMER:
  case ACTION_STAT_7_SELECT_ROD:
  case ACTION_STAT_7_SELECT_CLEAVER:
  case ACTION_STAT_7_SELECT_SCYTHE:
  case ACTION_STAT_7_SELECT_AXE:
  case ACTION_STAT_7_SELECT_SAW:
  case ACTION_STAT_7_SELECT_PICK:
  case ACTION_STAT_7_SELECT_PINCER:
  case ACTION_STAT_7_SELECT_SWORD:
  case ACTION_STAT_7_SELECT_SHIELD:
    interface->set_current_stat_7_item(action - ACTION_STAT_7_SELECT_FISH + 1);
    break;
  case ACTION_ATTACKING_KNIGHTS_DEC:
    player->knights_attacking = std::max(player->knights_attacking-1, 0);
    break;
  case ACTION_ATTACKING_KNIGHTS_INC:
    player->knights_attacking = std::min(player->knights_attacking + 1,
                                std::min(player->total_attacking_knights, 100));
    break;
  case ACTION_START_ATTACK:
    if (player->knights_attacking > 0) {
      if (player->attacking_building_count > 0) {
        play_sound(Audio::TypeSfxAccepted);
        player->start_attack();
      }
      close();
    } else {
      play_sound(Audio::TypeSfxNotAccepted);
    }
    break;
  case ACTION_CLOSE_ATTACK_BOX:
    close();
    break;
    /* TODO */
  case ACTION_SHOW_SETT_1:
    set_box(TypeSett1);
    break;
  case ACTION_SHOW_SETT_2:
    set_box(TypeSett2);
    break;
  case ACTION_SHOW_SETT_3:
    set_box(TypeSett3);
    break;
  case ACTION_SHOW_SETT_7:
    set_box(TypeKnightLevel);
    break;
  case ACTION_SHOW_SETT_4:
    set_box(TypeSett4);
    break;
  case ACTION_SHOW_SETT_5:
    set_box(TypeSett5);
    break;
  case ACTION_SHOW_SETT_SELECT:
    set_box(TypeSettSelect);
    break;
  case ACTION_SETT_1_ADJUST_STONEMINE:
    interface->open_popup(TypeSett1);
    player->set_food_stonemine(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_1_ADJUST_COALMINE:
    interface->open_popup(TypeSett1);
    player->set_food_coalmine(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_1_ADJUST_IRONMINE:
    interface->open_popup(TypeSett1);
    player->set_food_ironmine(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_1_ADJUST_GOLDMINE:
    interface->open_popup(TypeSett1);
    player->set_food_goldmine(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_2_ADJUST_CONSTRUCTION:
    interface->open_popup(TypeSett2);
    player->set_planks_construction(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_2_ADJUST_BOATBUILDER:
    interface->open_popup(TypeSett2);
    player->set_planks_boatbuilder(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_2_ADJUST_TOOLMAKER_PLANKS:
    interface->open_popup(TypeSett2);
    player->set_planks_toolmaker(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_2_ADJUST_TOOLMAKER_STEEL:
    interface->open_popup(TypeSett2);
    player->set_steel_toolmaker(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_2_ADJUST_WEAPONSMITH:
    interface->open_popup(TypeSett2);
    player->set_steel_weaponsmith(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_3_ADJUST_STEELSMELTER:
    interface->open_popup(TypeSett3);
    player->set_coal_steelsmelter(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_3_ADJUST_GOLDSMELTER:
    interface->open_popup(TypeSett3);
    player->set_coal_goldsmelter(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_3_ADJUST_WEAPONSMITH:
    interface->open_popup(TypeSett3);
    player->set_coal_weaponsmith(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_3_ADJUST_PIGFARM:
    interface->open_popup(TypeSett3);
    player->set_wheat_pigfarm(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_3_ADJUST_MILL:
    interface->open_popup(TypeSett3);
    player->set_wheat_mill(gui_get_slider_click_value(x_));
    break;
  case ACTION_KNIGHT_LEVEL_CLOSEST_MIN_DEC:
    player->change_knight_occupation(3, 0, -1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_CLOSEST_MIN_INC:
    player->change_knight_occupation(3, 0, 1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_CLOSEST_MAX_DEC:
    player->change_knight_occupation(3, 1, -1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_CLOSEST_MAX_INC:
    player->change_knight_occupation(3, 1, 1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_CLOSE_MIN_DEC:
    player->change_knight_occupation(2, 0, -1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_CLOSE_MIN_INC:
    player->change_knight_occupation(2, 0, 1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_CLOSE_MAX_DEC:
    player->change_knight_occupation(2, 1, -1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_CLOSE_MAX_INC:
    player->change_knight_occupation(2, 1, 1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_FAR_MIN_DEC:
    player->change_knight_occupation(1, 0, -1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_FAR_MIN_INC:
    player->change_knight_occupation(1, 0, 1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_FAR_MAX_DEC:
    player->change_knight_occupation(1, 1, -1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_FAR_MAX_INC:
    player->change_knight_occupation(1, 1, 1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_FARTHEST_MIN_DEC:
    player->change_knight_occupation(0, 0, -1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_FARTHEST_MIN_INC:
    player->change_knight_occupation(0, 0, 1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_FARTHEST_MAX_DEC:
    player->change_knight_occupation(0, 1, -1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_KNIGHT_LEVEL_FARTHEST_MAX_INC:
    player->change_knight_occupation(0, 1, 1);
    interface->open_popup(TypeKnightLevel);
    break;
  case ACTION_SETT_4_ADJUST_SHOVEL:
    interface->open_popup(TypeSett4);
    player->set_tool_prio(0, gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_4_ADJUST_HAMMER:
    interface->open_popup(TypeSett4);
    player->set_tool_prio(1, gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_4_ADJUST_AXE:
    interface->open_popup(TypeSett4);
    player->set_tool_prio(5, gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_4_ADJUST_SAW:
    interface->open_popup(TypeSett4);
    player->set_tool_prio(6, gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_4_ADJUST_SCYTHE:
    interface->open_popup(TypeSett4);
    player->set_tool_prio(4, gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_4_ADJUST_PICK:
    interface->open_popup(TypeSett4);
    player->set_tool_prio(7, gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_4_ADJUST_PINCER:
    interface->open_popup(TypeSett4);
    player->set_tool_prio(8, gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_4_ADJUST_CLEAVER:
    interface->open_popup(TypeSett4);
    player->set_tool_prio(3, gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_4_ADJUST_ROD:
    interface->open_popup(TypeSett4);
    player->set_tool_prio(2, gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_5_6_ITEM_1:
  case ACTION_SETT_5_6_ITEM_2:
  case ACTION_SETT_5_6_ITEM_3:
  case ACTION_SETT_5_6_ITEM_4:
  case ACTION_SETT_5_6_ITEM_5:
  case ACTION_SETT_5_6_ITEM_6:
  case ACTION_SETT_5_6_ITEM_7:
  case ACTION_SETT_5_6_ITEM_8:
  case ACTION_SETT_5_6_ITEM_9:
  case ACTION_SETT_5_6_ITEM_10:
  case ACTION_SETT_5_6_ITEM_11:
  case ACTION_SETT_5_6_ITEM_12:
  case ACTION_SETT_5_6_ITEM_13:
  case ACTION_SETT_5_6_ITEM_14:
  case ACTION_SETT_5_6_ITEM_15:
  case ACTION_SETT_5_6_ITEM_16:
  case ACTION_SETT_5_6_ITEM_17:
  case ACTION_SETT_5_6_ITEM_18:
  case ACTION_SETT_5_6_ITEM_19:
  case ACTION_SETT_5_6_ITEM_20:
  case ACTION_SETT_5_6_ITEM_21:
  case ACTION_SETT_5_6_ITEM_22:
  case ACTION_SETT_5_6_ITEM_23:
  case ACTION_SETT_5_6_ITEM_24:
  case ACTION_SETT_5_6_ITEM_25:
  case ACTION_SETT_5_6_ITEM_26:
    activate_sett_5_6_item(26-(action-ACTION_SETT_5_6_ITEM_1));
    break;
  case ACTION_SETT_5_6_TOP:
    move_sett_5_6_item(1, 1);
    break;
  case ACTION_SETT_5_6_UP:
    move_sett_5_6_item(1, 0);
    break;
  case ACTION_SETT_5_6_DOWN:
    move_sett_5_6_item(0, 0);
    break;
  case ACTION_SETT_5_6_BOTTOM:
    move_sett_5_6_item(0, 1);
    break;
  case ActionShowOptions:
    interface->open_popup(TypeOptions);
    break;
    /* TODO */
  case ACTION_SETT_8_CYCLE:
    player->cycle_knights();
    play_sound(Audio::TypeSfxAccepted);
    break;
  case ACTION_CLOSE_OPTIONS:
    close();
    break;
  case ACTION_OPTIONS_MESSAGE_COUNT_1:
    if (interface->get_config(3)) {
      interface->switch_config(3);
      interface->set_config(4);
    } else if (interface->get_config(4)) {
      interface->switch_config(4);
      interface->set_config(5);
    } else if (interface->get_config(5)) {
      interface->switch_config(5);
    } else {
      interface->set_config(3);
      interface->set_config(4);
      interface->set_config(5);
    }
    break;
  case ACTION_DEFAULT_SETT_1:
    interface->open_popup(TypeSett1);
    player->reset_food_priority();
    break;
  case ACTION_DEFAULT_SETT_2:
    interface->open_popup(TypeSett2);
    player->reset_planks_priority();
    player->reset_steel_priority();
    break;
  case ACTION_DEFAULT_SETT_5_6:
    switch (box) {
      case TypeSett5:
        player->reset_flag_priority();
        break;
      case TypeSett6:
        player->reset_inventory_priority();
        break;
      default:
        NOT_REACHED();
        break;
    }
    break;
  case ACTION_BUILD_STOCK:
    interface->build_building(Building::TypeStock);
    break;
  case ACTION_SEND_GEOLOGIST:
    handle_send_geologist();
    break;
  case ACTION_RES_MODE_IN:
  case ACTION_RES_MODE_STOP:
  case ACTION_RES_MODE_OUT:
    set_inventory_resource_mode(action - ACTION_RES_MODE_IN);
    break;
  case ACTION_SERF_MODE_IN:
  case ACTION_SERF_MODE_STOP:
  case ACTION_SERF_MODE_OUT:
    set_inventory_serf_mode(action - ACTION_SERF_MODE_IN);
    break;
  case ACTION_SHOW_SETT_8:
    set_box(TypeSett8);
    break;
  case ACTION_SHOW_SETT_6:
    set_box(TypeSett6);
    break;
  case ACTION_SETT_8_ADJUST_RATE:
    player->set_serf_to_knight_rate(gui_get_slider_click_value(x_));
    break;
  case ACTION_SETT_8_TRAIN_1:
    sett_8_train(1);
    break;
  case ACTION_SETT_8_TRAIN_5:
    sett_8_train(5);
    break;
  case ACTION_SETT_8_TRAIN_20:
    sett_8_train(20);
    break;
  case ACTION_SETT_8_TRAIN_100:
    sett_8_train(100);
    break;
  case ACTION_DEFAULT_SETT_3:
    interface->open_popup(TypeSett3);
    player->reset_coal_priority();
    player->reset_wheat_priority();
    break;
  case ACTION_SETT_8_SET_COMBAT_MODE_WEAK:
    player->drop_send_strongest();
    play_sound(Audio::TypeSfxAccepted);
    break;
  case ACTION_SETT_8_SET_COMBAT_MODE_STRONG:
    player->set_send_strongest();
    play_sound(Audio::TypeSfxAccepted);
    break;
  case ACTION_ATTACKING_SELECT_ALL_1:
    player->knights_attacking = player->attacking_knights[0];
    break;
  case ACTION_ATTACKING_SELECT_ALL_2:
    player->knights_attacking = player->attacking_knights[0]
                                + player->attacking_knights[1];
    break;
  case ACTION_ATTACKING_SELECT_ALL_3:
    player->knights_attacking = player->attacking_knights[0]
                                + player->attacking_knights[1]
                                + player->attacking_knights[2];
    break;
  case ACTION_ATTACKING_SELECT_ALL_4:
    player->knights_attacking = player->attacking_knights[0]
                                + player->attacking_knights[1]
                                + player->attacking_knights[2]
                                + player->attacking_knights[3];
    break;
  case ACTION_MINIMAP_BLD_1:
  case ACTION_MINIMAP_BLD_2:
  case ACTION_MINIMAP_BLD_3:
  case ACTION_MINIMAP_BLD_4:
  case ACTION_MINIMAP_BLD_5:
  case ACTION_MINIMAP_BLD_6:
  case ACTION_MINIMAP_BLD_7:
  case ACTION_MINIMAP_BLD_8:
  case ACTION_MINIMAP_BLD_9:
  case ACTION_MINIMAP_BLD_10:
  case ACTION_MINIMAP_BLD_11:
  case ACTION_MINIMAP_BLD_12:
  case ACTION_MINIMAP_BLD_13:
  case ACTION_MINIMAP_BLD_14:
  case ACTION_MINIMAP_BLD_15:
  case ACTION_MINIMAP_BLD_16:
  case ACTION_MINIMAP_BLD_17:
  case ACTION_MINIMAP_BLD_18:
  case ACTION_MINIMAP_BLD_19:
  case ACTION_MINIMAP_BLD_20:
  case ACTION_MINIMAP_BLD_21:
  case ACTION_MINIMAP_BLD_22:
  case ACTION_MINIMAP_BLD_23:
//    minimap->set_advanced(action - ACTION_MINIMAP_BLD_1 + 1);
//    minimap->set_draw_buildings(true);
    set_box(TypeMap);
    break;
  case ACTION_MINIMAP_BLD_FLAG:
//    minimap->set_advanced(0);
    set_box(TypeMap);
    break;
  case ACTION_MINIMAP_BLD_NEXT:
    set_box((Type)(box + 1));
    if (box > TypeBld4) {
      set_box(TypeBld1);
    }
    break;
  case ACTION_MINIMAP_BLD_EXIT:
    set_box(TypeMap);
    break;
  case ACTION_DEFAULT_SETT_4:
    interface->open_popup(TypeSett4);
    player->reset_tool_priority();
    break;
  case ACTION_SHOW_PLAYER_FACES:
    set_box(TypePlayerFaces);
    break;
  case ACTION_SETT_8_CASTLE_DEF_DEC:
    player->decrease_castle_knights_wanted();
    break;
  case ACTION_SETT_8_CASTLE_DEF_INC:
    player->increase_castle_knights_wanted();
    break;
  case ACTION_OPTIONS_MUSIC: {
    Audio &audio = Audio::get_instance();
    Audio::PPlayer player = audio.get_music_player();
    if (player) {
      player->enable(!player->is_enabled());
    }
    play_sound(Audio::TypeSfxClick);
    break;
  }
  case ACTION_OPTIONS_SFX: {
    Audio &audio = Audio::get_instance();
    Audio::PPlayer player = audio.get_sound_player();
    if (player) {
      player->enable(!player->is_enabled());
    }
    play_sound(Audio::TypeSfxClick);
    break;
  }
  case ACTION_OPTIONS_FULLSCREEN:
    Graphics::get_instance().set_fullscreen(
                                    !Graphics::get_instance().is_fullscreen());
    play_sound(Audio::TypeSfxClick);
    break;
  case ACTION_OPTIONS_VOLUME_MINUS: {
    Audio &audio = Audio::get_instance();
    Audio::VolumeController *volume_controller = audio.get_volume_controller();
    if (volume_controller != nullptr) {
      volume_controller->volume_down();
    }
    play_sound(Audio::TypeSfxClick);
    break;
  }
  case ACTION_OPTIONS_VOLUME_PLUS: {
    Audio &audio = Audio::get_instance();
    Audio::VolumeController *volume_controller = audio.get_volume_controller();
    if (volume_controller != nullptr) {
      volume_controller->volume_up();
    }
    play_sound(Audio::TypeSfxClick);
    break;
  }
  case ACTION_DEMOLISH:
    interface->demolish_object();
    close();
    break;
  case ACTION_SHOW_SAVE:
//    file_list->set_displayed(true);
//    file_field->set_displayed(true);
    set_box(TypeLoadSave);
    break;
  case ACTION_SAVE: {
    std::string file_name = file_field->get_text();
    size_t p = file_name.find_last_of('.');
    std::string file_ext;
    if (p != std::string::npos) {
      file_ext = file_name.substr(p+1, file_name.size());
      if (file_ext != "save") {
        file_ext.clear();
      }
    }
    if (file_ext.empty()) {
      file_name += ".save";
    }
    std::string file_path = file_list->get_folder_path() + "/" + file_name;
    if (GameStore::get_instance().save(file_path,
                                       interface->get_game().get())) {
      close();
    }
    break;
  }
  default:
    Log::Warn["popup"] << "unhandled action " << action;
    break;
  }
}  // NOLINT(readability/fn_size)

/* Generic handler for clicks in popup boxes. */
int
PopupBox::handle_clickmap(int x_, int y_, const int clkmap[]) {
  while (clkmap[0] >= 0) {
    if (clkmap[1] <= x_ && x_ < clkmap[1] + clkmap[3] &&
        clkmap[2] <= y_ && y_ < clkmap[2] + clkmap[4]) {
      play_sound(Audio::TypeSfxClick);

      Action action = (Action)clkmap[0];
      handle_action(action, x_-clkmap[1], y_-clkmap[2]);
      return 0;
    }
    clkmap += 5;
  }

  return -1;
}

void
PopupBox::handle_box_close_clk(int cx, int cy) {
  const int clkmap[] = {
    ACTION_CLOSE_BOX, 112, 128, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_box_options_clk(int cx, int cy) {
  const int clkmap[] = {
    ACTION_OPTIONS_MUSIC, 106, 10, 16, 16,
    ACTION_OPTIONS_SFX, 106, 30, 16, 16,
    ACTION_OPTIONS_VOLUME_MINUS, 90, 50, 16, 16,
    ACTION_OPTIONS_VOLUME_PLUS, 106, 50, 16, 16,
    ACTION_OPTIONS_FULLSCREEN, 106, 70, 16, 16,
    ACTION_OPTIONS_MESSAGE_COUNT_1, 90, 90, 32, 16,
    ACTION_CLOSE_OPTIONS, 112, 126, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_stat_select_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SHOW_STAT_1, 8, 12, 32, 32,
    ACTION_SHOW_STAT_2, 48, 12, 32, 32,
    ACTION_SHOW_STAT_3, 88, 12, 32, 32,
    ACTION_SHOW_STAT_4, 8, 56, 32, 32,
    ACTION_SHOW_STAT_BLD, 48, 56, 32, 32,
    ACTION_SHOW_STAT_6, 88, 56, 32, 32,
    ACTION_SHOW_STAT_7, 8, 100, 32, 32,
    ACTION_SHOW_STAT_8, 48, 100, 32, 32,
    ACTION_CLOSE_BOX, 112, 128, 16, 16,
    ACTION_SHOW_SETT_SELECT, 96, 104, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_stat_1_2_3_4_6_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SHOW_STAT_SELECT, 0, 0, 128, 144,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_stat_bld_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SHOW_STAT_SELECT, 112, 128, 16, 16,
    ACTION_STAT_BLD_FLIP, 0, 128, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_stat_8_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SETT_8_SET_ASPECT_ALL, 16, 112, 16, 16,
    ACTION_SETT_8_SET_ASPECT_LAND, 32, 112, 16, 16,
    ACTION_SETT_8_SET_ASPECT_BUILDINGS, 16, 128, 16, 16,
    ACTION_SETT_8_SET_ASPECT_MILITARY, 32, 128, 16, 16,

    ACTION_SETT_8_SET_SCALE_30_MIN, 64, 112, 16, 16,
    ACTION_SETT_8_SET_SCALE_60_MIN, 80, 112, 16, 16,
    ACTION_SETT_8_SET_SCALE_600_MIN, 64, 128, 16, 16,
    ACTION_SETT_8_SET_SCALE_3000_MIN, 80, 128, 16, 16,

    ACTION_SHOW_PLAYER_FACES, 112, 112, 16, 14,
    ACTION_SHOW_STAT_SELECT, 112, 128, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_stat_7_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_STAT_7_SELECT_LUMBER, 0, 75, 16, 16,
    ACTION_STAT_7_SELECT_PLANK, 16, 75, 16, 16,
    ACTION_STAT_7_SELECT_STONE, 32, 75, 16, 16,
    ACTION_STAT_7_SELECT_COAL, 0, 91, 16, 16,
    ACTION_STAT_7_SELECT_IRONORE, 16, 91, 16, 16,
    ACTION_STAT_7_SELECT_GOLDORE, 32, 91, 16, 16,
    ACTION_STAT_7_SELECT_BOAT, 0, 107, 16, 16,
    ACTION_STAT_7_SELECT_STEEL, 16, 107, 16, 16,
    ACTION_STAT_7_SELECT_GOLDBAR, 32, 107, 16, 16,

    ACTION_STAT_7_SELECT_SWORD, 56, 83, 16, 16,
    ACTION_STAT_7_SELECT_SHIELD, 56, 99, 16, 16,

    ACTION_STAT_7_SELECT_SHOVEL, 80, 75, 16, 16,
    ACTION_STAT_7_SELECT_HAMMER, 96, 75, 16, 16,
    ACTION_STAT_7_SELECT_AXE, 112, 75, 16, 16,
    ACTION_STAT_7_SELECT_SAW, 80, 91, 16, 16,
    ACTION_STAT_7_SELECT_PICK, 96, 91, 16, 16,
    ACTION_STAT_7_SELECT_SCYTHE, 112, 91, 16, 16,
    ACTION_STAT_7_SELECT_CLEAVER, 80, 107, 16, 16,
    ACTION_STAT_7_SELECT_PINCER, 96, 107, 16, 16,
    ACTION_STAT_7_SELECT_ROD, 112, 107, 16, 16,

    ACTION_STAT_7_SELECT_FISH, 8, 125, 16, 16,
    ACTION_STAT_7_SELECT_PIG, 24, 125, 16, 16,
    ACTION_STAT_7_SELECT_MEAT, 40, 125, 16, 16,
    ACTION_STAT_7_SELECT_WHEAT, 56, 125, 16, 16,
    ACTION_STAT_7_SELECT_FLOUR, 72, 125, 16, 16,
    ACTION_STAT_7_SELECT_BREAD, 88, 125, 16, 16,

    ACTION_SHOW_STAT_SELECT, 112, 128, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_start_attack_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_ATTACKING_KNIGHTS_DEC, 32, 112, 16, 16,
    ACTION_ATTACKING_KNIGHTS_INC, 80, 112, 16, 16,
    ACTION_START_ATTACK, 0, 128, 32, 16,
    ACTION_CLOSE_ATTACK_BOX, 112, 128, 16, 16,
    ACTION_ATTACKING_SELECT_ALL_1, 8, 80, 16, 24,
    ACTION_ATTACKING_SELECT_ALL_2, 40, 80, 16, 24,
    ACTION_ATTACKING_SELECT_ALL_3, 72, 80, 16, 24,
    ACTION_ATTACKING_SELECT_ALL_4, 104, 80, 16, 24,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_sett_select_clk(int cx, int cy) {
  const int clkmap[] = {
//    ACTION_SHOW_QUIT, 0, 128, 32, 16,
    ActionShowOptions, 32, 128, 32, 16,
    ACTION_SHOW_SAVE, 64, 128, 32, 16,

    ACTION_SHOW_SETT_1, 8, 8, 32, 32,
    ACTION_SHOW_SETT_2, 48, 8, 32, 32,
    ACTION_SHOW_SETT_3, 88, 8, 32, 32,
    ACTION_SHOW_SETT_4, 8, 48, 32, 32,
    ACTION_SHOW_SETT_5, 48, 48, 32, 32,
    ACTION_SHOW_SETT_6, 88, 48, 32, 32,
    ACTION_SHOW_SETT_7, 8, 88, 32, 32,
    ACTION_SHOW_SETT_8, 48, 88, 32, 32,

    ACTION_CLOSE_SETT_BOX, 112, 128, 16, 16,
    ACTION_SHOW_STAT_SELECT, 96, 104, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_sett_1_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SETT_1_ADJUST_STONEMINE, 32, 22, 64, 6,
    ACTION_SETT_1_ADJUST_COALMINE, 0, 42, 64, 6,
    ACTION_SETT_1_ADJUST_IRONMINE, 64, 115, 64, 6,
    ACTION_SETT_1_ADJUST_GOLDMINE, 32, 134, 64, 6,

    ACTION_SHOW_SETT_SELECT, 112, 128, 16, 16,
    ACTION_DEFAULT_SETT_1, 8, 8, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_sett_2_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SETT_2_ADJUST_CONSTRUCTION, 0, 27, 64, 6,
    ACTION_SETT_2_ADJUST_BOATBUILDER, 0, 37, 64, 6,
    ACTION_SETT_2_ADJUST_TOOLMAKER_PLANKS, 64, 45, 64, 6,
    ACTION_SETT_2_ADJUST_TOOLMAKER_STEEL, 64, 104, 64, 6,
    ACTION_SETT_2_ADJUST_WEAPONSMITH, 0, 131, 64, 6,

    ACTION_SHOW_SETT_SELECT, 112, 128, 16, 16,
    ACTION_DEFAULT_SETT_2, 104, 8, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_sett_3_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SETT_3_ADJUST_STEELSMELTER, 0, 40, 64, 6,
    ACTION_SETT_3_ADJUST_GOLDSMELTER, 64, 40, 64, 6,
    ACTION_SETT_3_ADJUST_WEAPONSMITH, 32, 48, 64, 6,
    ACTION_SETT_3_ADJUST_PIGFARM, 0, 93, 64, 6,
    ACTION_SETT_3_ADJUST_MILL, 64, 119, 64, 6,

    ACTION_SHOW_SETT_SELECT, 112, 128, 16, 16,
    ACTION_DEFAULT_SETT_3, 8, 60, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_knight_level_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_KNIGHT_LEVEL_CLOSEST_MAX_DEC, 32, 2, 16, 16,
    ACTION_KNIGHT_LEVEL_CLOSEST_MAX_INC, 48, 2, 16, 16,
    ACTION_KNIGHT_LEVEL_CLOSEST_MIN_DEC, 32, 18, 16, 16,
    ACTION_KNIGHT_LEVEL_CLOSEST_MIN_INC, 48, 18, 16, 16,
    ACTION_KNIGHT_LEVEL_CLOSE_MAX_DEC, 32, 36, 16, 16,
    ACTION_KNIGHT_LEVEL_CLOSE_MAX_INC, 48, 36, 16, 16,
    ACTION_KNIGHT_LEVEL_CLOSE_MIN_DEC, 32, 52, 16, 16,
    ACTION_KNIGHT_LEVEL_CLOSE_MIN_INC, 48, 52, 16, 16,
    ACTION_KNIGHT_LEVEL_FAR_MAX_DEC, 32, 70, 16, 16,
    ACTION_KNIGHT_LEVEL_FAR_MAX_INC, 48, 70, 16, 16,
    ACTION_KNIGHT_LEVEL_FAR_MIN_DEC, 32, 86, 16, 16,
    ACTION_KNIGHT_LEVEL_FAR_MIN_INC, 48, 86, 16, 16,
    ACTION_KNIGHT_LEVEL_FARTHEST_MAX_DEC, 32, 104, 16, 16,
    ACTION_KNIGHT_LEVEL_FARTHEST_MAX_INC, 48, 104, 16, 16,
    ACTION_KNIGHT_LEVEL_FARTHEST_MIN_DEC, 32, 120, 16, 16,
    ACTION_KNIGHT_LEVEL_FARTHEST_MIN_INC, 48, 120, 16, 16,

    ACTION_SHOW_SETT_SELECT, 112, 128, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_sett_4_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SETT_4_ADJUST_SHOVEL, 32, 4, 64, 8,
    ACTION_SETT_4_ADJUST_HAMMER, 32, 20, 64, 8,
    ACTION_SETT_4_ADJUST_AXE, 32, 36, 64, 8,
    ACTION_SETT_4_ADJUST_SAW, 32, 52, 64, 8,
    ACTION_SETT_4_ADJUST_SCYTHE, 32, 68, 64, 8,
    ACTION_SETT_4_ADJUST_PICK, 32, 84, 64, 8,
    ACTION_SETT_4_ADJUST_PINCER, 32, 100, 64, 8,
    ACTION_SETT_4_ADJUST_CLEAVER, 32, 116, 64, 8,
    ACTION_SETT_4_ADJUST_ROD, 32, 132, 64, 8,

    ACTION_SHOW_SETT_SELECT, 112, 128, 16, 16,
    ACTION_DEFAULT_SETT_4, 104, 8, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_sett_5_6_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SETT_5_6_ITEM_1, 40, 4, 16, 16,
    ACTION_SETT_5_6_ITEM_2, 56, 6, 16, 16,
    ACTION_SETT_5_6_ITEM_3, 72, 8, 16, 16,
    ACTION_SETT_5_6_ITEM_4, 88, 10, 16, 16,
    ACTION_SETT_5_6_ITEM_5, 104, 12, 16, 16,
    ACTION_SETT_5_6_ITEM_6, 104, 28, 16, 16,
    ACTION_SETT_5_6_ITEM_7, 88, 30, 16, 16,
    ACTION_SETT_5_6_ITEM_8, 72, 32, 16, 16,
    ACTION_SETT_5_6_ITEM_9, 56, 34, 16, 16,
    ACTION_SETT_5_6_ITEM_10, 40, 36, 16, 16,
    ACTION_SETT_5_6_ITEM_11, 24, 38, 16, 16,
    ACTION_SETT_5_6_ITEM_12, 8, 40, 16, 16,
    ACTION_SETT_5_6_ITEM_13, 8, 56, 16, 16,
    ACTION_SETT_5_6_ITEM_14, 24, 58, 16, 16,
    ACTION_SETT_5_6_ITEM_15, 40, 60, 16, 16,
    ACTION_SETT_5_6_ITEM_16, 56, 62, 16, 16,
    ACTION_SETT_5_6_ITEM_17, 72, 64, 16, 16,
    ACTION_SETT_5_6_ITEM_18, 88, 66, 16, 16,
    ACTION_SETT_5_6_ITEM_19, 104, 68, 16, 16,
    ACTION_SETT_5_6_ITEM_20, 104, 84, 16, 16,
    ACTION_SETT_5_6_ITEM_21, 88, 86, 16, 16,
    ACTION_SETT_5_6_ITEM_22, 72, 88, 16, 16,
    ACTION_SETT_5_6_ITEM_23, 56, 90, 16, 16,
    ACTION_SETT_5_6_ITEM_24, 40, 92, 16, 16,
    ACTION_SETT_5_6_ITEM_25, 24, 94, 16, 16,
    ACTION_SETT_5_6_ITEM_26, 8, 96, 16, 16,

    ACTION_SETT_5_6_TOP, 8, 120, 16, 16,
    ACTION_SETT_5_6_UP, 24, 120, 16, 16,
    ACTION_SETT_5_6_DOWN, 72, 120, 16, 16,
    ACTION_SETT_5_6_BOTTOM, 88, 120, 16, 16,

    ACTION_SHOW_SETT_SELECT, 112, 128, 16, 16,
    ACTION_DEFAULT_SETT_5_6, 8, 4, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_transport_info_clk(int cx, int cy) {
  const int clkmap[] = {
    ACTION_UNKNOWN_TP_INFO_FLAG, 56, 51, 16, 15,
    ACTION_SEND_GEOLOGIST, 16, 96, 16, 16,
    ACTION_CLOSE_BOX, 112, 128, 16, 16,
    -1
  };

  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_sett_8_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SETT_8_ADJUST_RATE, 32, 12, 64, 8,

    ACTION_SETT_8_TRAIN_1, 16, 28, 16, 16,
    ACTION_SETT_8_TRAIN_5, 32, 28, 16, 16,
    ACTION_SETT_8_TRAIN_20, 16, 44, 16, 16,
    ACTION_SETT_8_TRAIN_100, 32, 44, 16, 16,

    ACTION_SETT_8_SET_COMBAT_MODE_WEAK, 48, 84, 16, 16,
    ACTION_SETT_8_SET_COMBAT_MODE_STRONG, 48, 100, 16, 16,

    ACTION_SETT_8_CYCLE, 80, 84, 32, 32,

    ACTION_SETT_8_CASTLE_DEF_DEC, 24, 120, 16, 16,
    ACTION_SETT_8_CASTLE_DEF_INC, 72, 120, 16, 16,

    ACTION_SHOW_SETT_SELECT, 112, 128, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_message_clk(int cx, int cy) {
  const int clkmap[] = {
    ACTION_CLOSE_MESSAGE, 112, 128, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_player_faces_click(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SHOW_STAT_8, 0, 0, 128, 144,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_box_demolish_clk(int cx, int cy) {
  const int clkmap[] = {
    ACTION_CLOSE_BOX, 112, 128, 16, 16,
    ACTION_DEMOLISH, 56, 45, 16, 16,
    -1
  };
  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_box_bld_1(int cx, int cy) {
  const int clkmap[] = {
    ACTION_MINIMAP_BLD_1, 0, 0, 64, 51,
    ACTION_MINIMAP_BLD_2, 64, 0, 48, 51,
    ACTION_MINIMAP_BLD_3, 16, 64, 32, 32,
    ACTION_MINIMAP_BLD_4, 48, 60, 64, 71,
    ACTION_MINIMAP_BLD_FLAG, 25, 110, 16, 34,
    ACTION_MINIMAP_BLD_NEXT, 0, 128, 16, 16,
    ACTION_MINIMAP_BLD_EXIT, 112, 128, 16, 16,
    -1
  };

  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_box_bld_2(int cx, int cy) {
  const int clkmap[] = {
    ACTION_MINIMAP_BLD_5, 0, 0, 64, 56,
    ACTION_MINIMAP_BLD_6, 64, 0, 32, 51,
    ACTION_MINIMAP_BLD_7, 0, 64, 64, 32,
    ACTION_MINIMAP_BLD_8, 64, 64, 32, 32,
    ACTION_MINIMAP_BLD_9, 96, 60, 32, 36,
    ACTION_MINIMAP_BLD_10, 32, 104, 32, 36,
    ACTION_MINIMAP_BLD_11, 64, 104, 32, 36,
    ACTION_MINIMAP_BLD_NEXT, 0, 128, 16, 16,
    ACTION_MINIMAP_BLD_EXIT, 112, 128, 16, 16,
    -1
  };

  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_box_bld_3(int cx, int cy) {
  const int clkmap[] = {
    ACTION_MINIMAP_BLD_12, 0, 0, 64, 48,
    ACTION_MINIMAP_BLD_13, 64, 0, 64, 48,
    ACTION_MINIMAP_BLD_14, 0, 56, 32, 34,
    ACTION_MINIMAP_BLD_15, 32, 86, 32, 54,
    ACTION_MINIMAP_BLD_16, 64, 56, 64, 34,
    ACTION_MINIMAP_BLD_17, 64, 100, 48, 40,
    ACTION_MINIMAP_BLD_NEXT, 0, 128, 16, 16,
    ACTION_MINIMAP_BLD_EXIT, 112, 128, 16, 16,
    -1
  };

  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_box_bld_4(int cx, int cy) {
  const int clkmap[] = {
    ACTION_MINIMAP_BLD_18, 0, 0, 32, 64,
    ACTION_MINIMAP_BLD_19, 32, 0, 32, 64,
    ACTION_MINIMAP_BLD_20, 61, 0, 35, 64,
    ACTION_MINIMAP_BLD_21, 96, 0, 32, 64,
    ACTION_MINIMAP_BLD_22, 16, 95, 48, 41,
    ACTION_MINIMAP_BLD_23, 64, 95, 48, 41,
    ACTION_MINIMAP_BLD_NEXT, 0, 128, 16, 16,
    ACTION_MINIMAP_BLD_EXIT, 112, 128, 16, 16,
    -1
  };

  handle_clickmap(cx, cy, clkmap);
}

void
PopupBox::handle_save_clk(int cx, int cy) {
  const int clkmap[] = {
    ACTION_SAVE, 0, 128, 32, 64,
    ACTION_CLOSE_BOX, 112, 128, 16, 16,
    -1
  };

  handle_clickmap(cx, cy, clkmap);
}

bool
PopupBox::handle_click_left(int cx, int cy) {
  cx -= 8;
  cy -= 8;

  switch (box) {
  case TypeStatSelect:
    handle_stat_select_click(cx, cy);
    break;
  case TypeStat1:
  case TypeStat2:
  case TypeStat3:
  case TypeStat4:
  case TypeStat6:
    handle_stat_1_2_3_4_6_click(cx, cy);
    break;
  case TypeStatBld1:
  case TypeStatBld2:
  case TypeStatBld3:
  case TypeStatBld4:
    handle_stat_bld_click(cx, cy);
    break;
  case TypeStat8:
    handle_stat_8_click(cx, cy);
    break;
  case TypeStat7:
    handle_stat_7_click(cx, cy);
    break;
  case TypeStartAttack:
  case TypeStartAttackRedraw:
    handle_start_attack_click(cx, cy);
    break;
  case TypeSettSelect:
    handle_sett_select_clk(cx, cy);
    break;
  case TypeSett1:
    handle_sett_1_click(cx, cy);
    break;
  case TypeSett2:
    handle_sett_2_click(cx, cy);
    break;
  case TypeSett3:
    handle_sett_3_click(cx, cy);
    break;
  case TypeKnightLevel:
    handle_knight_level_click(cx, cy);
    break;
  case TypeSett4:
    handle_sett_4_click(cx, cy);
    break;
  case TypeSett5:
    handle_sett_5_6_click(cx, cy);
    break;
  case TypeOptions:
    handle_box_options_clk(cx, cy);
    break;
  case TypeTransportInfo:
    handle_transport_info_clk(cx, cy);
    break;
  case TypeSett8:
    handle_sett_8_click(cx, cy);
    break;
  case TypeSett6:
    handle_sett_5_6_click(cx, cy);
    break;
  case TypeBld1:
    handle_box_bld_1(cx, cy);
    break;
  case TypeBld2:
    handle_box_bld_2(cx, cy);
    break;
  case TypeBld3:
    handle_box_bld_3(cx, cy);
    break;
  case TypeBld4:
    handle_box_bld_4(cx, cy);
    break;
  case TypeMessage:
    handle_message_clk(cx, cy);
    break;
  case TypeBldStock:
    handle_box_close_clk(cx, cy);
    break;
  case TypePlayerFaces:
    handle_player_faces_click(cx, cy);
    break;
  case TypeDemolish:
    handle_box_demolish_clk(cx, cy);
    break;
  case TypeLoadSave:
    handle_save_clk(cx, cy);
    break;
  default:
    Log::Debug["popup"] << "unhandled box: " << box;
    break;
  }

  return true;
}

void PopupBox::show(Type box_) {
  set_box(box_);
  set_displayed(true);
}

void PopupBox::hide() {
  set_box((Type)0);
  set_displayed(false);
}

void
PopupBox::set_box(Type box_) {
  if (!initialized) {
    init();
    initialized = true;
  }

  box = box_;
  if (box == TypeMap) {
//    minimap->set_displayed(true);
  } else {
//    minimap->set_displayed(false);
  }
  set_redraw();
}
