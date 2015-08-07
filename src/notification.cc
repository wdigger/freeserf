/*
 * notification.cc - Notification GUI component
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

#include "src/notification.h"

#include <sstream>
#include <string>
#include <map>

#include "src/interface.h"

std::map<PlayerController::Notification::Type,
         NotificationBox::NotificationAppearance> notification_views = {
  { PlayerController::Notification::TypeUnderAttack, {
    NotificationBox::DecorationOpponent,
    0,
    "Your settlement\nis under attack" } },
  { PlayerController::Notification::TypeLoseFight, {
    NotificationBox::DecorationOpponent,
    0,
    "Your knights\njust lost the\nfight" } },
  { PlayerController::Notification::TypeWinFight, {
    NotificationBox::DecorationOpponent,
    0,
    "You gained\na victory here" } },
  { PlayerController::Notification::TypeMineEmpty, {
    NotificationBox::DecorationMine,
    0,
    "This mine hauls\nno more raw\nmaterials" } },
  { PlayerController::Notification::TypeCallToLocation, {
    NotificationBox::DecorationMapObject,
    0x90,
    "You wanted me\nto call you to\nthis location" } },
  { PlayerController::Notification::TypeKnightOccupied, {
    NotificationBox::DecorationBuilding,
    0,
    "A knight has\noccupied this\nnew building" } },
  { PlayerController::Notification::TypeNewStock, {
    NotificationBox::DecorationMapObject,
    map_building_sprite[Building::TypeStock],
    "A new stock\nhas been built" } },
  { PlayerController::Notification::TypeLostLand, {
    NotificationBox::DecorationOpponent,
    0,
    "Because of this\nenemy building\nyou lost some\nland" } },
  { PlayerController::Notification::TypeLostBuildings, {
    NotificationBox::DecorationOpponent,
    0,
    "Because of this\nenemy building\nyou lost some\n"
    "land and\nsome buildings" } },
  { PlayerController::Notification::TypeEmergencyActive, {
    NotificationBox::DecorationMapObject,
    map_building_sprite[Building::TypeCastle] + 1,
    "Emergency\nprogram\nactivated" } },
  { PlayerController::Notification::TypeEmergencyNeutral, {
    NotificationBox::DecorationMapObject,
    map_building_sprite[Building::TypeCastle],
    "Emergency\nprogram\nneutralized" } },
  { PlayerController::Notification::TypeFoundGold, {
    NotificationBox::DecorationIcon,
    0x2f,
    "A geologist\nhas found gold" } },
  { PlayerController::Notification::TypeFoundIron, {
    NotificationBox::DecorationIcon,
    0x2c,
    "A geologist\nhas found iron" } },
  { PlayerController::Notification::TypeFoundCoal, {
    NotificationBox::DecorationIcon,
    0x2e,
    "A geologist\nhas found coal" } },
  { PlayerController::Notification::TypeFoundStone, {
    NotificationBox::DecorationIcon,
    0x2b,
    "A geologist\nhas found stone" } },
  { PlayerController::Notification::TypeCallToMenu, {
    NotificationBox::DecorationMenu,
    0,
    "You wanted me\nto call you\nto this menu" } },
  { PlayerController::Notification::TypeCallToStock, {
    NotificationBox::DecorationMapObject,
    map_building_sprite[Building::TypeStock],
    "You wanted me\nto call you\nto this stock" } }
};

NotificationBox::NotificationBox(Interface *_interface) {
  interface = _interface;
}

void
NotificationBox::draw_notification(NotificationAppearance *view) {
  const int map_menu_sprite[] = { 230, 231, 232, 233, 234, 235, 298, 299 };

  draw_string(1, 10, view->text);
  switch (view->decoration) {
    case DecorationOpponent:
      draw_player_face(18, 8, notification.data);
      break;
    case DecorationMine:
      draw_map_object(18, 8, map_building_sprite[Building::TypeStoneMine] +
                             notification.data);
      break;
    case DecorationBuilding:
      switch (notification.data) {
        case 0:
          draw_map_object(18, 8, map_building_sprite[Building::TypeHut]);
          break;
        case 1:
          draw_map_object(18, 8, map_building_sprite[Building::TypeTower]);
          break;
        case 2:
          draw_map_object(16, 8, map_building_sprite[Building::TypeFortress]);
          break;
        default:
          NOT_REACHED();
          break;
      }
      break;
    case DecorationMapObject:
      draw_map_object(16, 8, view->icon);
      break;
    case DecorationIcon:
      draw_icon(20, 14, view->icon);
      break;
    case DecorationMenu:
      draw_icon(18, 8, map_menu_sprite[notification.data]);
      break;
    default:
      break;
  }
}

void
NotificationBox::internal_draw() {
  draw_background(width, height, 0x13a);
  draw_icon(14, 128, 0x120);  // Checkbox

  draw_notification(&notification_views[notification.type]);
}

bool
NotificationBox::handle_click_left(int x, int y) {
  set_displayed(0);
  return true;
}

void
NotificationBox::show(const PlayerController::Notification _notification) {
  notification = _notification;
  set_displayed(1);
}

void
NotificationBox::draw_icon(int x, int y, int sprite) {
  frame->draw_sprite(8*x, y, Data::AssetIcon, sprite);
}

void
NotificationBox::draw_background(int width, int height, int sprite) {
  for (int y = 0; y < height; y += 16) {
    for (int x = 0; x < width; x += 16) {
      frame->draw_sprite(x, y, Data::AssetIcon, sprite);
    }
  }
}

void
NotificationBox::draw_string(int x, int y, const std::string &str) {
  std::stringstream sin;
  sin << str;
  std::string line;
  int cy = y;
  while (std::getline(sin, line)) {
    frame->draw_string(x*8, cy, line, Color::green);
    cy += 10;
  }
}

void
NotificationBox::draw_map_object(int x, int y, int sprite) {
  frame->draw_sprite(8*x, y, Data::AssetMapObject, sprite);
}

unsigned int
NotificationBox::get_player_face_sprite(size_t face) {
  if (face != 0) {
    return static_cast<unsigned int>(0x10b + face);
  }
  return 0x119;  // sprite_face_none
}

void
NotificationBox::draw_player_face(int x, int y, int player) {
  Color color = interface->get_player_color(player);
  frame->fill_rect(8*x, y, 48, 72, color);
  unsigned int face = interface->get_player_face(player);
  draw_icon(x+1, y+4, get_player_face_sprite(face));
}
