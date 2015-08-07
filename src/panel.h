/*
 * panel.h - Panel GUI component
 *
 * Copyright (C) 2012  Jon Lund Steffensen <jonlst@gmail.com>
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

#ifndef SRC_PANEL_H_
#define SRC_PANEL_H_

#include "src/gui.h"
#include "src/player_controller.h"

class PanelBar : public GuiObject
               , public PlayerController::Handler
               , public Timer::Handler {
 protected:
  typedef enum Button {
    ButtonBuildInactive = 0,
    ButtonBuildFlag,
    ButtonBuildMine,
    ButtonBuildSmall,
    ButtonBuildLarge,
    ButtonBuildCastle,
    ButtonDestroy,
    ButtonDestroyInactive,
    ButtonBuildRoad,
    ButtonMapInactive,
    ButtonMap,
    ButtonStatsInactive,
    ButtonStats,
    ButtonSettInactive,
    ButtonSett,
    ButtonDestroyRoad,
    ButtonGroundAnalysis,
    ButtonBuildSmallStarred,
    ButtonBuildLargeStarred,
    ButtonMapStarred,
    ButtonStatsStarred,
    ButtonSettStarred,
    ButtonGroundAnalysisStarred,
    ButtonBuildMineStarred,
    ButtonBuildRoadStarred
  } Button;

  PPlayerController player_controller;

  int panel_btns[5];
  Timer *blink_timer;
  bool blink_trigger;

 public:
  explicit PanelBar(PPlayerController player_controller);
  virtual ~PanelBar();

  void activate_button(int button);

  void update();

 protected:
  void draw_panel_frame();
  void draw_message_notify();
  void draw_return_arrow();
  void draw_panel_buttons();
  void button_click(int button);
  Button button_type_with_build_possibility(int build_possibility);

  virtual void internal_draw();
  virtual bool handle_click_left(int x, int y);
  virtual bool handle_key_pressed(char key, int modifier);
  void handle_panel_button_click(int button);

 public:
  virtual void road_building_state_changed(bool building_road);
  virtual void cursor_position_changed(MapPos pos, bool scroll);
  virtual void cursor_type_changed(PlayerController::CursorType type);
  virtual void build_possibility_changed(
                                PlayerController::BuildPossibility possibility);
  virtual void open_dialog(int id);
  virtual void close_dialog();
  virtual void present_notification(
                                 PlayerController::Notification notification) {}

  // timer_handler_t
  virtual void on_timer_fired(Timer *timer, unsigned int id);
};

#endif  // SRC_PANEL_H_
