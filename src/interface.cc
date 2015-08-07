/*
 * interface.cc - Top-level GUI interface
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

#include "src/interface.h"

#include <iostream>
#include <fstream>
#include <utility>
#include <memory>

#include "src/audio.h"
#include "src/freeserf.h"
#include "src/popup.h"
#include "src/game-init.h"
#include "src/viewport.h"
#include "src/notification.h"
#include "src/panel.h"
#include "src/savegame.h"

// Interval between automatic save games
#define AUTOSAVE_INTERVAL  (10*60*TICKS_PER_SEC)

Interface::Interface()
  : sfx_queue{0}
  , water_in_view(false)
  , trees_in_view(false) {
  displayed = true;

  player_controller = nullptr;

  const_tick = 0;
  last_const_tick = 0;

  GameManager &game_manager = GameManager::get_instance();
  game_manager.add_handler(this);
  set_game(game_manager.get_current_game());
}

Interface::~Interface() {
  set_game(nullptr);
}

Viewport *
Interface::get_viewport() {
  return viewport.get();
}

PanelBar *
Interface::get_panel_bar() {
  return panel.get();
}

PopupBox *
Interface::get_popup_box() {
  return popup.get();
}

/* Open box for starting a new game */
void
Interface::open_game_init() {
  /* Game init box */
  init_box = std::unique_ptr<GameInitBox>(new GameInitBox(this));
  init_box->set_displayed(true);
  add_float(init_box.get(), 0, 0);

  if (panel != nullptr) {
    panel->set_enabled(false);
  }

  if (viewport != nullptr) {
    viewport->set_enabled(false);
  }

  layout();
}

void
Interface::close_game_init() {
  if (init_box != nullptr) {
    del_float(init_box.get());
    init_box.release();
  }

  if (panel != nullptr) {
    panel->set_enabled(true);
  }

  if (viewport != nullptr) {
    viewport->set_enabled(true);
  }

  layout();
}

/* Open box for next message in the message queue */
void
Interface::open_message() {
//  if (!player_controller->get_player()->has_notification()) {
//    play_sound(Audio::TypeSfxClick);
//    return;
//  } else if (!player_controller->has_msg_flag(3)) {
//    player_controller->set_msg_flag(4);
//    player_controller->set_msg_flag(3);
//    MapPos pos = viewport->get_current_map_pos();
//    player_controller->set_return_pos(pos);
//  }

//  message_t message = player->pop_notification();

//  if (player->msg_queue[0].type == 16) {
    /* TODO */
//  }

//  notification_box->show(player->msg_queue[0]);

//  if (BIT_TEST(0x8f3fe, type)) {
    /* Move screen to new position */
//    map_pos_t new_pos = player->msg_queue[0].pos;

//    viewport->move_to_map_pos(new_pos);
//  }
//  notification_box->show(message);
//  layout();

//  if (BIT_TEST(0x8f3fe, message.type)) {
    /* Move screen to new position */
//    viewport->move_to_map_pos(message.pos);
//    update_map_cursor_pos(message.pos);
//  }
//  player->msg_queue[i-1].type = NOTIFICATION_NONE;

  player_controller->set_msg_flag(1);
  player_controller->set_return_timeout(60*TICKS_PER_SEC);
  play_sound(Audio::TypeSfxClick);
}

void
Interface::return_from_message() {
  if (player_controller->has_msg_flag(3)) { /* Return arrow present */
    player_controller->set_msg_flag(4);
    player_controller->del_msg_flag(3);

    player_controller->set_return_timeout(0);
    viewport->move_to_map_pos(player_controller->get_return_pos());

    if (popup->get_box() == PopupBox::TypeMessage) close_dialog();
    play_sound(Audio::TypeSfxClick);
  }
}

void
Interface::close_message() {
  if (notification_box == nullptr) {
    return;
  }

  notification_box->set_displayed(false);
  del_float(notification_box.get());
  notification_box.release();
  layout();
}

void
Interface::set_player(Player *_player) {
  if (panel != nullptr) {
    del_float(panel.get());
    panel.release();
  }

  player_controller = nullptr;

  if (_player == nullptr) {
    return;
  }

  player_controller = std::make_shared<PlayerController>(_player);
  player_controller->add_handler(this);

  // Move viewport to initial position
  MapPos init_pos = game->get_map()->pos(0, 0);
  unsigned int castle_flag = _player->get_castle_flag();
  if (castle_flag != 0) {
    Flag *flag = game->get_flag(castle_flag);
    init_pos = game->get_map()->move_up_left(flag->get_position());
  }

  viewport->set_player_controller(player_controller);
  player_controller->set_cursor_pos(init_pos);

  // Panel bar
  panel = std::unique_ptr<PanelBar>(new PanelBar(player_controller));
  panel->set_displayed(true);
  add_float(panel.get(), 0, 0);

  layout();
}

Color
Interface::get_player_color(unsigned int index) {
  return Color(0, 0, 0, 0);
}

unsigned int
Interface::get_player_face(unsigned int index) {
  return 0;
}

void
Interface::update_map_height(MapPos pos, void *data) {
  Interface *interface = reinterpret_cast<Interface*>(data);
  interface->viewport->redraw_map_pos(pos);
}

void
Interface::internal_draw() {
}

void
Interface::layout() {
  int panel_width = 352;
  int panel_height = 40;
  int panel_x = (width - panel_width) / 2;
  int panel_y = height - panel_height;

  if (panel != nullptr) {
    panel->move_to(panel_x, panel_y);
    panel->set_size(panel_width, panel_height);
  }

  if (popup != nullptr) {
    int popup_width = 144;
    int popup_height = 160;
    int popup_x = (width - popup_width) / 2;
    int popup_y = (height - popup_height) / 2;
    popup->move_to(popup_x, popup_y);
    popup->set_size(popup_width, popup_height);
  }

  if (init_box != nullptr) {
    int init_box_width;
    int init_box_height;
    init_box->get_size(&init_box_width, &init_box_height);
    int init_box_x = (width - init_box_width) / 2;
    int init_box_y = (height - init_box_height) / 2;
    init_box->move_to(init_box_x, init_box_y);
  }

  if (notification_box != nullptr) {
    int notification_box_width = 200;
    int notification_box_height = 88;
    int notification_box_x = panel_x + 40;
    int notification_box_y = panel_y - notification_box_height;
    notification_box->move_to(notification_box_x, notification_box_y);
    notification_box->set_size(notification_box_width, notification_box_height);
  }

  if (viewport != nullptr) {
    viewport->set_size(width, height);
  }

  set_redraw();
}

void
Interface::set_game(PGame _game) {
  game = _game;
  set_player(nullptr);

  if (viewport != nullptr) {
    del_float(viewport.get());
    viewport.release();
  }

  if (popup != nullptr) {
    del_float(popup.get());
    popup.release();
  }

  if (game == nullptr) {
    return;
  }

  /* Viewport */
  viewport = std::unique_ptr<Viewport>(new Viewport(game));
  viewport->set_displayed(true);
  add_float(viewport.get(), 0, 0);

  layout();
}

/* Called periodically when the game progresses. */
void
Interface::update() {
  /* Increment tick counters */
  const_tick += 1;
  last_const_tick = const_tick;

  if (viewport != nullptr) {
    viewport->update();
  }

  set_redraw();

  /* Autosave periodically */
//  if ((const_tick % AUTOSAVE_INTERVAL) == 0 &&
//      game->game_speed > 0) {
//    int r = save_game(1, game);
//    if (r < 0) LOGW("main", "Autosave failed.");
//  }
}

bool
Interface::handle_key_pressed(char key, int modifier) {
  switch (key) {
    /* Interface control */
    case '\t': {
      if (modifier & 2) {
        return_from_message();
      } else {
        open_message();
      }
      break;
    }
    case 27: {
      if ((notification_box != nullptr) && notification_box->is_displayed()) {
        close_message();
      } else if (get_popup_box()->is_displayed()) {
        close_dialog();
      } else if (player_controller->is_building_road()) {
        player_controller->build_road_end();
      }
      break;
    }

    /* Game speed */
    case '+': {
      game->speed_increase();
      break;
    }
    case '-': {
      game->speed_decrease();
      break;
    }
    case '0': {
      game->speed_reset();
      break;
    }
    case 'p': {
      game->pause();
      break;
    }

    /* Audio */
    case 's': {
      Audio &audio = Audio::get_instance();
      Audio::PPlayer splayer = audio.get_sound_player();
      if (splayer) {
        splayer->enable(!splayer->is_enabled());
      }
      break;
    }
    case 'm': {
      Audio &audio = Audio::get_instance();
      Audio::PPlayer splayer = audio.get_music_player();
      if (splayer) {
        splayer->enable(!splayer->is_enabled());
      }
      break;
    }

    /* Debug */
    case 'g': {
      viewport->switch_layer(Viewport::LayerGrid);
      break;
    }

    /* Game control */
    case 'b': {
      viewport->switch_layer(Viewport::LayerBuilds);
      break;
    }
    case 'j': {
      Player *player = player_controller->get_player();
      unsigned int index = game->get_next_player(player)->get_index();
      set_player(game->get_player(index));
      Log::Debug["main"] << "Switched to player " << index << ".";
      break;
    }
    case 'z':
      if (modifier & 1) {
        GameStore::get_instance().quick_save("quicksave", game.get());
      }
      break;
    case 'n':
      if (modifier & 1) {
        open_game_init();
      }
      break;
    case 'c':
      if (modifier & 1) {
        open_dialog(PopupBox::TypeQuitConfirm);
      }
      break;

    default:
      return false;
  }

  return true;
}

bool
Interface::handle_event(const Event *event) {
  switch (event->type) {
    case Event::TypeResize:
      set_size(event->dx, event->dy);
      break;
    case Event::TypeUpdate:
      if (game) {
        game->update();
      }
      update();
      break;
    case Event::TypeDraw:
      draw(reinterpret_cast<Frame*>(event->object));
      break;

    default:
      return GuiObject::handle_event(event);
      break;
  }

  return true;
}

void
Interface::on_new_game(PGame game) {
  set_game(game);
  set_player(game->get_player(0));
}

void
Interface::on_end_game(PGame /*game*/) {
  set_game(nullptr);
}

// Open popup box
void
Interface::open_dialog(int id) {
  if (popup == nullptr) {
    popup = std::unique_ptr<PopupBox>(new PopupBox(this, game,
                                                   player_controller));
    add_float(popup.get(), 0, 0);
  }

  popup->show((PopupBox::Type)id);
//  if (id == BOX_MAP) {
//    /* Synchronize minimap window with viewport. */
//    map_pos_t pos = viewport->get_current_map_pos();
//    popup->get_minimap()->move_to_map_pos(pos);
//  }
//  popup->set_displayed(1);
  layout();
}

// Close the current popup
void
Interface::close_dialog() {
  if (popup != nullptr) {
    del_float(popup.get());
    popup.release();
  }
}
