/*
 * player_controller.cc - Player controller related implementation.
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

#include "src/player_controller.h"

#include <algorithm>
#include <cstdlib>

#include "src/player.h"
#include "src/game.h"
#include "src/debug.h"
#include "src/popup.h"
#include "src/audio.h"

PlayerController::PlayerController(Player *_player) {
  player = _player;
  player->add_handler(this);
  game = player->get_game();
  config = 0x39;
  msg_flags = 0;

  cursor_pos = -1;
  set_cursor_pos(0);
  opened_dialog = 0;
}

void
PlayerController::add_timer(unsigned int timeout) {
  Timer::create(cursor_pos, timeout * 1000, this);
}

void
PlayerController::set_cursor_pos(MapPos pos, bool scroll) {
  if (pos == cursor_pos) {
    return;
  }

  cursor_pos = pos;
  update_build_possibility();
  update_cursor_type();

  for (Handler *handler : handlers) {
    handler->cursor_position_changed(cursor_pos, scroll);
  }
}

bool
PlayerController::is_ground_analysis_possible() const {
  return !player->has_castle();
}

// Return the cursor type and various related values of a map_pos_t.
void
PlayerController::update_build_possibility() {
  BuildPossibility new_possibility = build_possibility_at(cursor_pos);

  if (new_possibility != build_possibility) {
    build_possibility = new_possibility;
    for (Handler *handler : handlers) {
      handler->build_possibility_changed(build_possibility);
    }
  }
}

PlayerController::BuildPossibility
PlayerController::build_possibility_at(MapPos pos) const {
  BuildPossibility possibility = BuildPossibilityNone;

  if (game->can_build_castle(pos, player)) {
    possibility = BuildPossibilityCastle;
  } else {
    if (game->can_player_build(pos, player) &&
        Map::map_space_from_obj[game->get_map()->get_obj(pos)] ==
        Map::SpaceOpen &&
        (game->can_build_flag(game->get_map()->move_down_right(pos), player) ||
         game->get_map()->has_flag(game->get_map()->move_down_right(pos)))) {
      if (game->can_build_mine(pos)) {
        possibility = BuildPossibilityMine;
      } else if (game->can_build_large(pos)) {
        possibility = BuildPossibilityLarge;
      } else if (game->can_build_small(pos)) {
        possibility = BuildPossibilitySmall;
      } else if (game->can_build_flag(pos, player)) {
        possibility = BuildPossibilityFlag;
      } else {
        possibility = BuildPossibilityNone;
      }
    } else if (game->can_build_flag(pos, player)) {
      possibility = BuildPossibilityFlag;
    }
  }

  return possibility;
}

void
PlayerController::update_cursor_type() {
  CursorType new_type = CursorTypeNone;

  if (game->get_map()->get_obj(cursor_pos) == Map::ObjectFlag &&
      game->get_map()->get_owner(cursor_pos) == player->get_index()) {
    if (game->can_demolish_flag(cursor_pos, player)) {
      new_type = CursorTypeRemovableFlag;
    } else {
      new_type = CursorTypeFlag;
    }
  } else if (!game->get_map()->has_building(cursor_pos) &&
             !game->get_map()->has_flag(cursor_pos)) {
    int paths = game->get_map()->paths(cursor_pos);
    if (paths == 0) {
      if (game->get_map()->get_obj(game->get_map()->move_down_right(cursor_pos))
          == Map::ObjectFlag) {
        new_type = CursorTypeClearByFlag;
      } else if (game->get_map()->paths(
                           game->get_map()->move_down_right(cursor_pos)) == 0) {
        new_type = CursorTypeClear;
      } else {
        new_type = CursorTypeClearByPath;
      }
    } else if (game->get_map()->get_owner(cursor_pos) == player->get_index()) {
      new_type = CursorTypePath;
    } else {
      new_type = CursorTypeNone;
    }
  } else if ((game->get_map()->get_obj(cursor_pos) ==
                Map::ObjectSmallBuilding ||
              game->get_map()->get_obj(cursor_pos) ==
                Map::ObjectLargeBuilding) &&
             game->get_map()->get_owner(cursor_pos) == player->get_index()) {
    Building *bld = game->get_building_at_pos(cursor_pos);
    if (!bld->is_burning()) {
      new_type = CursorTypeBuilding;
    } else {
      new_type = CursorTypeNone;
    }
  } else {
    new_type = CursorTypeNone;
  }

  if (new_type != cursor_type) {
    cursor_type = new_type;
    for (Handler *handler : handlers) {
      handler->cursor_type_changed(cursor_type);
    }
  }
}

void
PlayerController::on_event(const Player::PEvent event) {
/*
  const int msg_category[] = {
    -1,
    5,
    5,
    5,
    4,
    0,
    4,
    3,
    4,
    5,
    5,
    5,
    4,
    4,
    4,
    4,
    0,
    0,
    0,
    0
  };
*/
  // Handle newly enqueued messages
//  bool has_new_message = false;
//  while (player->has_notification()) {
//    Player::Event message = player->pop_notification();
//    int type = message.type & 0x1f;
//    if (BIT_TEST(config, msg_category[type])) {
//      has_new_message = true;
//      message_queue.push(message);
//    }
//  }

  play_sound(Audio::TypeSfxMessage);
}

bool
PlayerController::build_road() {
  return game->build_road(building_road, player);
}

bool
PlayerController::demolish_road() {
  bool res = player->get_game()->demolish_road(cursor_pos, player);
  update_build_possibility();
  update_cursor_type();
  return res;
}

bool
PlayerController::demolish_flag() {
  bool res = game->demolish_flag(cursor_pos, player);
  update_build_possibility();
  update_cursor_type();
  return res;
}

bool
PlayerController::demolish_object() {
  bool res = false;

  if (cursor_type == CursorTypeRemovableFlag) {
    play_sound(Audio::TypeSfxClick);
    return demolish_flag();
  } else if (cursor_type == CursorTypeBuilding) {
    Building *building = game->get_building_at_pos(cursor_pos);

    if (building->is_done() &&
        (building->get_type() == Building::TypeHut ||
         building->get_type() == Building::TypeTower ||
         building->get_type() == Building::TypeFortress)) {
          // TODO(Digger): what to do?
        }

    play_sound(Audio::TypeSfxAhhh);
    res = game->demolish_building(cursor_pos, player);
  } else {
    play_sound(Audio::TypeSfxNotAccepted);
  }

  update_build_possibility();
  update_cursor_type();

  return res;
}

bool
PlayerController::can_build_flag() {
  return player->get_game()->can_build_flag(cursor_pos, player);
}

bool
PlayerController::can_build_military() {
  return player->get_game()->can_build_military(cursor_pos);
}

void
PlayerController::get_ground_analysis(int estimates[5]) {
  player->get_game()->prepare_ground_analysis(cursor_pos, estimates);
}

bool
PlayerController::send_geologist() {
  unsigned int flag_index =
                       player->get_game()->get_map()->get_obj_index(cursor_pos);
  Flag *flag = player->get_game()->get_flag(flag_index);
  if (flag == nullptr) {
    return false;
  }

  return player->get_game()->send_geologist(flag);
}

// Start road construction mode for player interface.
void
PlayerController::build_road_begin() {
  if (cursor_type != CursorTypeFlag && cursor_type != CursorTypeRemovableFlag) {
    return;
  }

  building_road.invalidate();
  building_road.start(cursor_pos);

  update_cursor_type();
  update_build_possibility();

  for (Handler *handler : handlers) {
    handler->road_building_state_changed(true);
  }
}

// End road construction mode for player interface.
void
PlayerController::build_road_end() {
  building_road.invalidate();

  update_cursor_type();
  update_build_possibility();

  for (Handler *handler : handlers) {
    handler->road_building_state_changed(false);
  }
}

// Build a single road segment. Return -1 on fail, 0 on successful
// construction, and 1 if this segment completed the path.
int
PlayerController::build_road_segment(Direction dir) {
  building_road.extend(dir);

  MapPos dest;
  int r = game->can_build_road(building_road, player, &dest, nullptr);
  if (!r) {
    // Invalid construction, undo.
    return remove_road_segment();
  }

  if (game->get_map()->get_obj(dest) == Map::ObjectFlag) {
    // Existing flag at destination, try to connect.
    if (!game->build_road(building_road, player)) {
      build_road_end();
      return -1;
    } else {
      build_road_end();
      set_cursor_pos(dest);
      return 1;
    }
  } else if (game->get_map()->paths(dest) == 0) {
    // No existing paths at destination, build segment.
    set_cursor_pos(dest);

    // TODO(Digger): Pathway scrolling
  } else {
    // TODO(Digger): fast split path and connect on double click
    return -1;
  }

  return 0;
}

// Extend currently constructed road with an array of directions.
int
PlayerController::extend_road(const Road &road) {
  Road saved = building_road;

  for (Direction dir : road.get_dirs()) {
    int r = build_road_segment(dir);
    if (r < 0) {
      building_road = saved;
      return -1;
    } else if (r == 1) {
      return 1;
    }
  }

  return 0;
}

bool
PlayerController::remove_road_segment() {
  building_road.undo();

  MapPos dest;
  if (!game->can_build_road(building_road, player, &dest, nullptr)) {
    /* Road construction is no longer valid, abort. */
    build_road_end();
    return false;
  }

  set_cursor_pos(building_road.get_end(game->get_map().get()), true);

  return true;
}

PlayerController::PavePossibility
PlayerController::get_pave_possibility(Direction dir) {
  if (!building_road.is_valid()) {
    return PavePossibilityNone;
  }

  if (building_road.is_undo(dir)) {
    return PavePossibilityUndo;
  }

  if (!building_road.is_valid_extension(game->get_map().get(), dir)) {
    return PavePossibilityNone;
  }

  return PavePossibilityDo;
}

int
PlayerController::get_road_elevation(Direction dir) {
  int h = game->get_map()->get_height(cursor_pos);
  int h_diff =
        game->get_map()->get_height(game->get_map()->move(cursor_pos, dir)) - h;
  return h_diff;
}

void
PlayerController::add_handler(Handler *handler) {
  handlers.push_back(handler);
}

void
PlayerController::del_handler(Handler *handler) {
  handlers.remove(handler);
}

// Build new flag.
bool
PlayerController::build_flag() {
  bool res = game->build_flag(cursor_pos, player);
  if (!res) {
    play_sound(Audio::TypeSfxNotAccepted);
  }

  update_cursor_type();
  update_build_possibility();

  return res;
}

// Build a new building.
bool
PlayerController::build_building(Building::Type type) {
  bool res = game->build_building(cursor_pos, type, player);
  if (res) {
    dialog_close();

    // Move cursor to flag.
    MapPos flag_pos = game->get_map()->move_down_right(cursor_pos);
    set_cursor_pos(flag_pos);
  }

  return res;
}

/* Build castle. */
bool
PlayerController::build_castle() {
  bool res = game->build_castle(cursor_pos, player);
  if (!res) {
    play_sound(Audio::TypeSfxNotAccepted);
  }

  play_sound(Audio::TypeSfxAccepted);

  update_cursor_type();
  update_build_possibility();

  return res;
}

bool
PlayerController::has_new_notification() {
  return false;
}

void
PlayerController::notification_open() {
}

void
PlayerController::notification_return() {
}

void
PlayerController::dialog_open(int id) {
  opened_dialog = id;
  for (Handler *handler : handlers) {
    handler->open_dialog(id);
  }
}

void
PlayerController::dialog_close() {
  opened_dialog = 0;
  for (Handler *handler : handlers) {
    handler->close_dialog();
  }
}

bool
PlayerController::is_my_position(MapPos pos) {
  return (game->get_map()->has_owner(pos) &&
          game->get_map()->get_owner(pos) == player->get_index());
}

void
PlayerController::set_temp_index(int index) {
  player->temp_index = index;
}

void
PlayerController::set_building_attacked(int building_index) {
  player->building_attacked = building_index;
}

bool
PlayerController::initiate_atack(MapPos pos) {
  Building *building = game->get_building_at_pos(pos);
  player->building_attacked = building->get_index();

  if (building->is_done() && building->is_military()) {
    if (!building->is_active() || building->is_burning()) {
      // It is not allowed to attack if currently not occupied or
      // is too far from the border.
      return false;
    }

    int found = 0;
    for (int i = 257; i >= 0; i--) {
      MapPos pos = game->get_map()->pos_add_spirally(building->get_position(),
                                                     7+257-i);
      if (is_my_position(pos)) {
        found = 1;
        break;
      }
    }

    if (!found) {
      return false;
    }

    // Action accepted
    int max_knights = 0;
    switch (building->get_type()) {
      case Building::TypeHut: max_knights = 3; break;
      case Building::TypeTower: max_knights = 6; break;
      case Building::TypeFortress: max_knights = 12; break;
      case Building::TypeCastle: max_knights = 20; break;
      default: NOT_REACHED(); break;
    }

    int knights = player->knights_available_for_attack(pos);
    player->knights_attacking = std::min(knights, max_knights);
    dialog_open(PopupBox::TypeStartAttack);
  }

  return true;
}

void
PlayerController::on_timer_fired(Timer *timer, unsigned int id) {
//  player->add_notification(Player::Event::TypeCallToLocation, id, 0);
}

void
PlayerController::play_sound(int sound) {
  Audio::get_instance().get_sound_player()->play_track(sound);
}
