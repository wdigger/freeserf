/*
 * player_controller.h - Player controller related definitions.
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

#ifndef SRC_PLAYER_CONTROLLER_H_
#define SRC_PLAYER_CONTROLLER_H_

#include <list>
#include <memory>

#include "src/map.h"
#include "src/building.h"
#include "src/player.h"
#include "src/event_loop.h"
#include "src/mission.h"

#define MAX_ROAD_LENGTH  256

class PlayerController : public Timer::Handler, public Player::Handler {
 public:
  typedef enum CursorType {
    CursorTypeNone = 0,
    CursorTypeFlag,
    CursorTypeRemovableFlag,
    CursorTypeBuilding,
    CursorTypePath,
    CursorTypeClearByFlag,
    CursorTypeClearByPath,
    CursorTypeClear
  } CursorType;

  typedef enum BuildPossibility {
    BuildPossibilityNone = 0,
    BuildPossibilityFlag = 1,
    BuildPossibilityMine = 2,
    BuildPossibilitySmall = 3,
    BuildPossibilityLarge = 4,
    BuildPossibilityCastle = 5
  } BuildPossibility;

  typedef enum PavePossibility {
    PavePossibilityNone = 0,
    PavePossibilityUndo,
    PavePossibilityDo
  } PavePossibility;

  class Notification {
   public:
    typedef enum Type {
      TypeNone = 0,
      TypeUnderAttack = 1,
      TypeLoseFight = 2,
      TypeWinFight = 3,
      TypeMineEmpty = 4,
      TypeKnightOccupied = 5,
      TypeNewStock = 6,
      TypeLostLand = 7,
      TypeLostBuildings = 8,
      TypeEmergencyActive = 9,
      TypeEmergencyNeutral = 10,
      TypeFoundGold = 11,
      TypeFoundIron = 12,
      TypeFoundCoal = 13,
      TypeFoundStone = 14,
      TypeCallToLocation = 15,
      TypeCallToMenu = 16,
      TypeCallToStock = 17
    } Type;

   public:
    Type type;
    MapPos pos;
    unsigned int data;

    Notification() {}
    explicit Notification(const Player::Event &event);
  };

  class Handler {
   public:
    virtual void road_building_state_changed(bool building_road) = 0;
    virtual void cursor_position_changed(MapPos pos, bool scroll) = 0;
    virtual void cursor_type_changed(CursorType type) = 0;
    virtual void build_possibility_changed(BuildPossibility possibility) = 0;
    virtual void open_dialog(int id) = 0;
    virtual void close_dialog() = 0;
    virtual void present_notification(Notification notification) = 0;
  };

 protected:
  typedef std::list<Handler*> Handlers;

  Game *game;
  Player *player;

  PlayerInfo::Color color;
  unsigned int face;

  int config;
  int msg_flags;
  int opened_dialog;

  int return_timeout;
  MapPos return_pos;

  MapPos cursor_pos;
  CursorType cursor_type;
  BuildPossibility build_possibility;

  Handlers handlers;

  Road building_road;

  Player::Events message_queue;

 public:
  explicit PlayerController(Player *player);

  Player *get_player() { return player; }

  PlayerInfo::Color get_color() const { return color; }

  bool get_config(int i) const { return (BIT_TEST(config, i) != 0); }
  void set_config(int i) { config |= BIT(i); }
  void switch_config(int i) { BIT_INVERT(config, i); }

  void set_msg_flag(int n) { msg_flags |= BIT(n); }
  void del_msg_flag(int n) { msg_flags &= ~BIT(n); }
  bool has_msg_flag(int n) const { return (BIT_TEST(msg_flags, n) != 0); }

  void set_return_timeout(unsigned int timeout) { return_timeout = timeout; }
  void set_return_pos(MapPos pos) { return_pos = pos; }
  MapPos get_return_pos() const { return return_pos; }

  void add_timer(unsigned int timeout);

  MapPos get_cursor_pos() const { return cursor_pos; }
  void set_cursor_pos(MapPos pos, bool scroll = false);
  CursorType get_cursor_type() const { return cursor_type; }
  bool is_ground_analysis_possible() const;

  BuildPossibility get_build_possibility() const { return build_possibility; }
  BuildPossibility build_possibility_at(MapPos pos) const;
  bool demolish_road();
  bool demolish_flag();
  bool demolish_object();
  bool can_build_flag();
  bool can_build_castle();
  bool can_build_military();
  void get_ground_analysis(int estimates[5]);
  bool send_geologist();
  bool build_flag();
  bool build_building(Building::Type type);
  bool build_castle();
  bool build_road();

  void add_handler(Handler *handler);
  void del_handler(Handler *handler);

  void build_road_begin();
  void build_road_end();
  bool is_building_road() { return building_road.is_valid(); }
  Road &get_road() { return building_road; }
  int build_road_segment(Direction dir);
  int extend_road(const Road &road);
  bool remove_road_segment();
  PavePossibility get_pave_possibility(Direction dir);
  int get_road_elevation(Direction dir);

  bool has_new_notification();
  void notification_open();
  void notification_return();

  void dialog_open(int id);
  void dialog_close();
  bool is_dialog_opened() { return (opened_dialog != 0); }
  int get_opened_dialog_id() { return opened_dialog; }

  bool is_my_position(MapPos pos);
  void set_temp_index(int index);
  void set_building_attacked(int building_index);

  bool initiate_atack(MapPos pos);

  void play_sound(int sound);

 protected:
  void update_build_possibility();
  void update_cursor_type();

  virtual void on_timer_fired(Timer *timer, unsigned int id);

  // Player::Handler
 public:
  virtual void on_event(const Player::PEvent event);
};

typedef std::shared_ptr<PlayerController> PPlayerController;

#endif  // SRC_PLAYER_CONTROLLER_H_
