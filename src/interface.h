/*
 * interface.h - Top-level GUI interface
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

#ifndef SRC_INTERFACE_H_
#define SRC_INTERFACE_H_

#include <list>
#include <memory>

#include "src/gui.h"
#include "src/game-manager.h"
#include "src/player_controller.h"

static const unsigned int map_building_sprite[] = {
  0, 0xa7, 0xa8, 0xae, 0xa9,
  0xa3, 0xa4, 0xa5, 0xa6,
  0xaa, 0xc0, 0xab, 0x9a, 0x9c, 0x9b, 0xbc,
  0xa2, 0xa0, 0xa1, 0x99, 0x9d, 0x9e, 0x98, 0x9f, 0xb2
};

class Viewport;
class PanelBar;
class PopupBox;
class GameInitBox;
class NotificationBox;

class Interface : public GuiObject,
                  public GameManager::Handler,
                  public PlayerController::Handler {
 protected:
  typedef struct SpriteLoc {
    int sprite;
    int x, y;
  } SpriteLoc;

 protected:
  PGame game;

  typedef std::list<PPlayerController> PlayerControllers;
  PPlayerController player_controller;
  PlayerControllers player_controllers;

  std::unique_ptr<Viewport> viewport;
  std::unique_ptr<PanelBar> panel;
  std::unique_ptr<PopupBox> popup;
  std::unique_ptr<GameInitBox> init_box;
  std::unique_ptr<NotificationBox> notification_box;

  /* Increased by one no matter the game speed. */
  unsigned int const_tick;
  unsigned int last_const_tick;

  int sfx_queue[4];

  int water_in_view;
  int trees_in_view;

 public:
  Interface();
  virtual ~Interface();

  PGame get_game() { return game; }
  void set_game(PGame game);

  Viewport *get_viewport();
  PanelBar *get_panel_bar();
  PopupBox *get_popup_box();
  NotificationBox *get_notification_box() { return notification_box.get(); }

  void open_game_init();
  void close_game_init();

  void open_message();
  void return_from_message();
  void close_message();

  void set_player(Player *player);
  Color get_player_color(unsigned int index);
  unsigned int get_player_face(unsigned int index);

  void update();

  virtual bool handle_event(const Event *event);

  unsigned int get_const_tick() const { return const_tick; }

  int get_config() const;

 protected:
  virtual void internal_draw();
  virtual void layout();
  virtual bool handle_key_pressed(char key, int modifier);

  void determine_map_cursor_type();
  void determine_map_cursor_type_road();

  static void update_map_height(MapPos pos, void *data);

  // GameManager::Handler implementation
 public:
  virtual void on_new_game(PGame game);
  virtual void on_end_game(PGame game);

 public:
  virtual void road_building_state_changed(bool building_road) {}
  virtual void cursor_position_changed(MapPos pos, bool scroll) {}
  virtual void cursor_type_changed(PlayerController::CursorType type) {}
  virtual void build_possibility_changed(
                              PlayerController::BuildPossibility possibility) {}
  virtual void open_dialog(int id);
  virtual void close_dialog();
  virtual void present_notification(
                                 PlayerController::Notification notification) {}
};

#endif  // SRC_INTERFACE_H_
