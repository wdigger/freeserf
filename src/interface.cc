/*
 * interface.c - Top-level GUI interface
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

#include "interface.h"
#include "viewport.h"
#include "panel.h"
#include "game-init.h"
#include "notification.h"
#include "popup.h"
#include "data.h"
#include "gfx.h"
#include "audio.h"
#include "freeserf.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "debug.h"
  #include "game.h"
  #include "savegame.h"
#ifndef _MSC_VER
}
#endif

viewport_t *
interface_t::get_viewport()
{
  return viewport;
}

panel_bar_t *
interface_t::get_panel_bar()
{
  return panel;
}

popup_box_t *
interface_t::get_popup_box()
{
  return popup;
}


/* Open popup box */
void
interface_t::open_popup(int box)
{
  popup->set_box((box_t)box);
  popup->set_displayed(1);
}

/* Close the current popup. */
void
interface_t::close_popup()
{
  popup->set_box((box_t)0);
  popup->set_displayed(0);
  panel->set_button_type(2, PANEL_BTN_MAP);
  panel->set_button_type(3, PANEL_BTN_STATS);
  panel->set_button_type(4, PANEL_BTN_SETT);

  update_map_cursor_pos(map_cursor_pos);
}

/* Open box for starting a new game */
void
interface_t::open_game_init()
{
  init_box->set_displayed(1);
  panel->set_enabled(0);
  viewport->set_enabled(0);
}

void
interface_t::close_game_init()
{
  init_box->set_displayed(0);
  panel->set_enabled(1);
  viewport->set_enabled(1);

  update_map_cursor_pos(map_cursor_pos);
}

/* Open box for next message in the message queue */
void
interface_t::open_message()
{
  if (player->msg_queue_type[0] == 0) {
    audio_t::get_audio()->sfx_play_clip(SFX_CLICK);
    return;
  } else if (!BIT_TEST(msg_flags, 3)) {
    msg_flags |= BIT(4);
    msg_flags |= BIT(3);
    map_pos_t pos = viewport->get_current_map_pos();
    return_pos = pos;
  }

  int type = player->msg_queue_type[0] & 0x1f;

  if (type == 16) {
    /* TODO */
  }

  int param = (player->msg_queue_type[0] >> 5) & 7;
  notification_box->type = type;
  notification_box->param = param;
  notification_box->set_displayed(1);

  if (BIT_TEST(0x8f3fe, type)) {
    /* Move screen to new position */
    map_pos_t new_pos = player->msg_queue_pos[0];

    viewport->move_to_map_pos(new_pos);
    update_map_cursor_pos(new_pos);
  }

  /* Move notifications forward in the queue. */
  int i;
  for (i = 1; i < 64 && player->msg_queue_type[i] != 0; i++) {
    player->msg_queue_type[i-1] = player->msg_queue_type[i];
    player->msg_queue_pos[i-1] = player->msg_queue_pos[i];
  }
  player->msg_queue_type[i-1] = 0;

  msg_flags |= BIT(1);
  return_timeout = 60*TICKS_PER_SEC;
  audio_t::get_audio()->sfx_play_clip(SFX_CLICK);
}

void
interface_t::return_from_message()
{
  if (BIT_TEST(msg_flags, 3)) { /* Return arrow present */
    msg_flags |= BIT(4);
    msg_flags &= ~BIT(3);

    return_timeout = 0;
    viewport->move_to_map_pos(return_pos);

    if (popup->get_box() == BOX_MESSAGE) close_popup();
    audio_t::get_audio()->sfx_play_clip(SFX_CLICK);
  }
}

void
interface_t::close_message()
{
  notification_box->set_displayed(0);
}


/* Return the cursor type and various related values of a map_pos_t. */
static void
get_map_cursor_type(const player_t *player, map_pos_t pos, panel_btn_t *panel_btn,
        map_cursor_type_t *cursor_type)
{
  if (game_can_build_castle(pos, player)) {
    *panel_btn = PANEL_BTN_BUILD_CASTLE;
  } else if (game_can_player_build(pos, player) &&
       map_space_from_obj[MAP_OBJ(pos)] == MAP_SPACE_OPEN &&
       (game_can_build_flag(MAP_MOVE_DOWN_RIGHT(pos), player) ||
        MAP_HAS_FLAG(MAP_MOVE_DOWN_RIGHT(pos)))) {
    if (game_can_build_mine(pos)) {
      *panel_btn = PANEL_BTN_BUILD_MINE;
    } else if (game_can_build_large(pos)) {
      *panel_btn = PANEL_BTN_BUILD_LARGE;
    } else if (game_can_build_small(pos)) {
      *panel_btn = PANEL_BTN_BUILD_SMALL;
    } else if (game_can_build_flag(pos, player)) {
      *panel_btn = PANEL_BTN_BUILD_FLAG;
    } else {
      *panel_btn = PANEL_BTN_BUILD_INACTIVE;
    }
  } else if (game_can_build_flag(pos, player)) {
    *panel_btn = PANEL_BTN_BUILD_FLAG;
  } else {
    *panel_btn = PANEL_BTN_BUILD_INACTIVE;
  }

  if (MAP_OBJ(pos) == MAP_OBJ_FLAG &&
      MAP_OWNER(pos) == player->player_num) {
    if (game_can_demolish_flag(pos, player)) {
      *cursor_type = MAP_CURSOR_TYPE_REMOVABLE_FLAG;
    } else {
      *cursor_type = MAP_CURSOR_TYPE_FLAG;
    }
  } else if (!MAP_HAS_BUILDING(pos) && !MAP_HAS_FLAG(pos)) {
    int paths = MAP_PATHS(pos);
    if (paths == 0) {
      if (MAP_OBJ(MAP_MOVE_DOWN_RIGHT(pos)) == MAP_OBJ_FLAG) {
        *cursor_type = MAP_CURSOR_TYPE_CLEAR_BY_FLAG;
      } else if (MAP_PATHS(MAP_MOVE_DOWN_RIGHT(pos)) == 0) {
        *cursor_type = MAP_CURSOR_TYPE_CLEAR;
      } else {
        *cursor_type = MAP_CURSOR_TYPE_CLEAR_BY_PATH;
      }
    } else if (MAP_OWNER(pos) == player->player_num) {
      *cursor_type = MAP_CURSOR_TYPE_PATH;
    } else {
      *cursor_type = MAP_CURSOR_TYPE_NONE;
    }
  } else if ((MAP_OBJ(pos) == MAP_OBJ_SMALL_BUILDING ||
        MAP_OBJ(pos) == MAP_OBJ_LARGE_BUILDING) &&
       MAP_OWNER(pos) == player->player_num) {
    building_t *bld = game_get_building(MAP_OBJ_INDEX(pos));
    if (!BUILDING_IS_BURNING(bld)) {
      *cursor_type = MAP_CURSOR_TYPE_BUILDING;
    } else {
      *cursor_type = MAP_CURSOR_TYPE_NONE;
    }
  } else {
    *cursor_type = MAP_CURSOR_TYPE_NONE;
  }
}


/* Update the interface_t object with the information returned
   in get_map_cursor_type(). */
void
interface_t::interface_determine_map_cursor_type()
{
  map_pos_t cursor_pos = get_map_cursor_pos();
  get_map_cursor_type(get_player(), cursor_pos,
          &panel_btn_type,
          &map_cursor_type);
}

/* Update the interface_t object with the information returned
   in get_map_cursor_type(). This is sets the appropriate values
   when the player interface is in road construction mode. */
void
interface_t::interface_determine_map_cursor_type_road()
{
  map_pos_t pos = map_cursor_pos;
  int h = MAP_HEIGHT(pos);
  int valid_dir = 0;
  int length = building_road_length;

  for (int d = DIR_RIGHT; d <= DIR_UP; d++) {
    int sprite = 0;

    if (length > 0 && building_road_dirs[length-1] == DIR_REVERSE(d)) {
      sprite = 45; /* undo */
      valid_dir |= BIT(d);
    } else if (map_road_segment_valid(pos, (dir_t)d, &game.map)) {
      /* Check that road does not cross itself. */
      map_pos_t road_pos = building_road_source;
      int crossing_self = 0;
      for (int i = 0; i < length; i++) {
        road_pos = MAP_MOVE(road_pos, building_road_dirs[i]);
        if (road_pos == MAP_MOVE(pos, d)) {
          crossing_self = 1;
          break;
        }
      }

      if (!crossing_self) {
        int h_diff = MAP_HEIGHT(MAP_MOVE(pos, d)) - h;
        sprite = 39 + h_diff; /* height indicators */
        valid_dir |= BIT(d);
      } else {
        sprite = 44;
      }
    } else {
      sprite = 44; /* striped */
    }
    map_cursor_sprites[d+1] = sprite;
  }

  building_road_valid_dir = valid_dir;
}

/* Set the appropriate sprites for the panel buttons and the map cursor. */
void
interface_t::interface_update_interface()
{
  if (building_road) {
    panel->set_button_type(0, PANEL_BTN_BUILD_ROAD_STARRED);
    panel->set_button_type(1, PANEL_BTN_BUILD_INACTIVE);
  } else {
    switch (map_cursor_type) {
    case MAP_CURSOR_TYPE_NONE:
      panel->set_button_type(0, PANEL_BTN_BUILD_INACTIVE);
      if (PLAYER_HAS_CASTLE(player)) {
        panel->set_button_type(1, PANEL_BTN_DESTROY_INACTIVE);
      } else {
        panel->set_button_type(1, PANEL_BTN_GROUND_ANALYSIS);
      }
      map_cursor_sprites[0] = 32;
      map_cursor_sprites[2] = 33;
      break;
    case MAP_CURSOR_TYPE_FLAG:
      panel->set_button_type(0, PANEL_BTN_BUILD_ROAD);
      panel->set_button_type(1, PANEL_BTN_DESTROY_INACTIVE);
      map_cursor_sprites[0] = 51;
      map_cursor_sprites[2] = 33;
      break;
    case MAP_CURSOR_TYPE_REMOVABLE_FLAG:
      panel->set_button_type(0, PANEL_BTN_BUILD_ROAD);
      panel->set_button_type(1, PANEL_BTN_DESTROY);
      map_cursor_sprites[0] = 51;
      map_cursor_sprites[2] = 33;
      break;
    case MAP_CURSOR_TYPE_BUILDING:
      panel->set_button_type(0, panel_btn_type);
      panel->set_button_type(1, PANEL_BTN_DESTROY);
      map_cursor_sprites[0] = 32;
      map_cursor_sprites[2] = 33;
      break;
    case MAP_CURSOR_TYPE_PATH:
      panel->set_button_type(0, PANEL_BTN_BUILD_INACTIVE);
      panel->set_button_type(1, PANEL_BTN_DESTROY_ROAD);
      map_cursor_sprites[0] = 52;
      map_cursor_sprites[2] = 33;
      if (panel_btn_type != PANEL_BTN_BUILD_INACTIVE) {
        panel->set_button_type(0, PANEL_BTN_BUILD_FLAG);
        map_cursor_sprites[0] = 47;
      }
      break;
    case MAP_CURSOR_TYPE_CLEAR_BY_FLAG:
      if (panel_btn_type < PANEL_BTN_BUILD_MINE) {
        panel->set_button_type(0, PANEL_BTN_BUILD_INACTIVE);
        if (PLAYER_HAS_CASTLE(player)) {
          panel->set_button_type(1, PANEL_BTN_DESTROY_INACTIVE);
        } else {
          panel->set_button_type(1, PANEL_BTN_GROUND_ANALYSIS);
        }
        map_cursor_sprites[0] = 32;
        map_cursor_sprites[2] = 33;
      } else {
        panel->set_button_type(0, panel_btn_type);
        panel->set_button_type(1, PANEL_BTN_DESTROY_INACTIVE);
        map_cursor_sprites[0] = 46 + panel_btn_type;
        map_cursor_sprites[2] = 33;
      }
      break;
    case MAP_CURSOR_TYPE_CLEAR_BY_PATH:
      panel->set_button_type(0, panel_btn_type);
      panel->set_button_type(1, PANEL_BTN_DESTROY_INACTIVE);
      if (panel_btn_type != PANEL_BTN_BUILD_INACTIVE) {
        map_cursor_sprites[0] = 46 + panel_btn_type;
        if (panel_btn_type == PANEL_BTN_BUILD_FLAG) {
          map_cursor_sprites[2] = 33;
        } else {
          map_cursor_sprites[2] = 47;
        }
      } else {
        map_cursor_sprites[0] = 32;
        map_cursor_sprites[2] = 33;
      }
      break;
    case MAP_CURSOR_TYPE_CLEAR:
      panel->set_button_type(0, panel_btn_type);
      if (PLAYER_HAS_CASTLE(player)) {
        panel->set_button_type(1, PANEL_BTN_DESTROY_INACTIVE);
      } else {
        panel->set_button_type(1, PANEL_BTN_GROUND_ANALYSIS);
      }
      if (panel_btn_type) {
        if (panel_btn_type == PANEL_BTN_BUILD_CASTLE) {
          map_cursor_sprites[0] = 50;
        } else {
          map_cursor_sprites[0] = 46 + panel_btn_type;
        }
        if (panel_btn_type == PANEL_BTN_BUILD_FLAG) {
          map_cursor_sprites[2] = 33;
        } else {
          map_cursor_sprites[2] = 47;
        }
      } else {
        map_cursor_sprites[0] = 32;
        map_cursor_sprites[2] = 33;
      }
      break;
    default:
      NOT_REACHED();
      break;
    }
  }
}

void
interface_t::set_player(uint player)
{
  if(!PLAYER_IS_ACTIVE(game.player[player])) {
    return;
  }
  this->player = game.player[player];

  /* Move viewport to initial position */
  map_pos_t init_pos = MAP_POS(0,0);
  if (this->player->castle_flag != 0) {
    flag_t *flag = game_get_flag(this->player->castle_flag);
    init_pos = MAP_MOVE_UP_LEFT(flag->pos);
  }

  update_map_cursor_pos(init_pos);
  viewport->move_to_map_pos(map_cursor_pos);
}

void
interface_t::update_map_cursor_pos(map_pos_t pos)
{
  map_cursor_pos = pos;
  if (building_road) {
    interface_determine_map_cursor_type_road();
  } else {
    interface_determine_map_cursor_type();
  }
  interface_update_interface();
}


/* Start road construction mode for player interface. */
void
interface_t::build_road_begin()
{
  interface_determine_map_cursor_type();

  if (map_cursor_type != MAP_CURSOR_TYPE_FLAG &&
      map_cursor_type != MAP_CURSOR_TYPE_REMOVABLE_FLAG) {
    interface_update_interface();
    return;
  }

  panel->set_button_type(0, PANEL_BTN_BUILD_ROAD_STARRED);
  panel->set_button_type(1, PANEL_BTN_BUILD_INACTIVE);
  panel->set_button_type(2, PANEL_BTN_MAP_INACTIVE);
  panel->set_button_type(3, PANEL_BTN_STATS_INACTIVE);
  panel->set_button_type(4, PANEL_BTN_SETT_INACTIVE);

  building_road = 1;
  building_road_length = 0;
  building_road_source = map_cursor_pos;

  update_map_cursor_pos(map_cursor_pos);
}

/* End road construction mode for player interface. */
void
interface_t::build_road_end()
{
  panel->set_button_type(2, PANEL_BTN_MAP);
  panel->set_button_type(3, PANEL_BTN_STATS);
  panel->set_button_type(4, PANEL_BTN_SETT);

  map_cursor_sprites[1] = 33;
  map_cursor_sprites[2] = 33;
  map_cursor_sprites[3] = 33;
  map_cursor_sprites[4] = 33;
  map_cursor_sprites[5] = 33;
  map_cursor_sprites[6] = 33;

  building_road = 0;
  update_map_cursor_pos(map_cursor_pos);
}

/* Build a single road segment. Return -1 on fail, 0 on successful
   construction, and 1 if this segment completed the path. */
int
interface_t::build_road_segment(dir_t dir)
{
  if (building_road_length+1 >= MAX_ROAD_LENGTH) {
    /* Max length reached */
    return -1;
  }

  building_road_dirs[building_road_length] = dir;
  building_road_length += 1;

  map_pos_t dest;
  int r = game_can_build_road(building_road_source,
            building_road_dirs,
            building_road_length,
            player, &dest, NULL);
  if (!r) {
    /* Invalid construction, undo. */
    return remove_road_segment();
  }

  if (MAP_OBJ(dest) == MAP_OBJ_FLAG) {
    /* Existing flag at destination, try to connect. */
    int r = game_build_road(building_road_source,
          building_road_dirs,
          building_road_length,
          player);
    if (r < 0) {
      build_road_end();
      return -1;
    } else {
      build_road_end();
      update_map_cursor_pos(dest);
      return 1;
    }
  } else if (MAP_PATHS(dest) == 0) {
    /* No existing paths at destination, build segment. */
    update_map_cursor_pos(dest);

    /* TODO Pathway scrolling */
  } else {
    /* TODO fast split path and connect on double click */
    return -1;
  }

  return 0;
}

int
interface_t::remove_road_segment()
{
  building_road_length -= 1;

  map_pos_t dest;
  int r = game_can_build_road(building_road_source,
            building_road_dirs,
            building_road_length,
            player, &dest, NULL);
  if (!r) {
    /* Road construction is no longer valid, abort. */
    build_road_end();
    return -1;
  }

  update_map_cursor_pos(dest);

  /* TODO Pathway scrolling */

  return 0;
}

/* Extend currently constructed road with an array of directions. */
int
interface_t::extend_road(dir_t *dirs, uint length)
{
  for (uint i = 0; i < length; i++) {
    dir_t dir = dirs[i];
    int r = build_road_segment(dir);
    if (r < 0) {
      /* Backtrack */
      for (int j = i-1; j >= 0; j--) {
        remove_road_segment();
      }
      return -1;
    } else if (r == 1) {
      return 1;
    }
  }

  return 0;
}

void
interface_t::demolish_object()
{
  interface_determine_map_cursor_type();

  if (map_cursor_type == MAP_CURSOR_TYPE_REMOVABLE_FLAG) {
    audio_t::get_audio()->sfx_play_clip(SFX_CLICK);
    game_demolish_flag(map_cursor_pos, player);
  } else if (map_cursor_type == MAP_CURSOR_TYPE_BUILDING) {
    building_t *building = game_get_building(MAP_OBJ_INDEX(map_cursor_pos));

    if (BUILDING_IS_DONE(building) &&
        (BUILDING_TYPE(building) == BUILDING_HUT ||
         BUILDING_TYPE(building) == BUILDING_TOWER ||
         BUILDING_TYPE(building) == BUILDING_FORTRESS)) {
      /* TODO */
    }

    audio_t::get_audio()->sfx_play_clip(SFX_AHHH);
    game_demolish_building(map_cursor_pos, player);
  } else {
    audio_t::get_audio()->sfx_play_clip(SFX_NOT_ACCEPTED);
    interface_update_interface();
  }
}

/* Build new flag. */
void
interface_t::build_flag()
{
  int r = game_build_flag(map_cursor_pos, player);
  if (r < 0) {
    audio_t::get_audio()->sfx_play_clip(SFX_NOT_ACCEPTED);
    return;
  }

  update_map_cursor_pos(map_cursor_pos);
}

/* Build a new building. */
void
interface_t::build_building(building_type_t type)
{
  int r = game_build_building(map_cursor_pos, type, player);
  if (r < 0) {
    audio_t::get_audio()->sfx_play_clip(SFX_NOT_ACCEPTED);
    return;
  }

  audio_t::get_audio()->sfx_play_clip(SFX_ACCEPTED);
  close_popup();

  /* Move cursor to flag. */
  map_pos_t flag_pos = MAP_MOVE_DOWN_RIGHT(map_cursor_pos);
  update_map_cursor_pos(flag_pos);
}

/* Build castle. */
void
interface_t::build_castle()
{
  int r = game_build_castle(map_cursor_pos, player);
  if (r < 0) {
    audio_t::get_audio()->sfx_play_clip(SFX_NOT_ACCEPTED);
    return;
  }

  audio_t::get_audio()->sfx_play_clip(SFX_ACCEPTED);
  update_map_cursor_pos(map_cursor_pos);
}


static void
update_map_height(map_pos_t pos, interface_t *interface)
{
  interface->get_viewport()->redraw_map_pos(pos);
}

void
interface_t::internal_draw()
{
}

void
interface_t::layout()
{
  int panel_width = 352;
  int panel_height = 40;
  int panel_x = (width - panel_width) / 2;
  int panel_y = height - panel_height;
  panel->move_to(panel_x, panel_y);
  panel->set_size(panel_width, panel_height);

  int popup_width = 144;
  int popup_height = 160;
  int popup_x = (width - popup_width) / 2;
  int popup_y = (height - popup_height) / 2;
  popup->move_to(popup_x, popup_y);
  popup->set_size(popup_width, popup_height);

  int init_box_width;
  int init_box_height;
  init_box->get_size(init_box_width, init_box_height);
  int init_box_x = (width - init_box_width) / 2;
  int init_box_y = (height - init_box_height) / 2;
  init_box->move_to(init_box_x, init_box_y);

  int notification_box_width = 200;
  int notification_box_height = 88;
  int notification_box_x = panel_x + 40;
  int notification_box_y = panel_y - notification_box_height;
  notification_box->move_to(notification_box_x, notification_box_y);
  notification_box->set_size(notification_box_width, notification_box_height);

  viewport->set_size(width, height);

  set_redraw();
}

interface_t::interface_t()
  : gui_object_t()
{
  displayed = true;

  /* Viewport */
  viewport = new viewport_t(this);
  viewport->set_displayed(true);

  /* Panel bar */
  panel = new panel_bar_t(this);
  panel->set_displayed(true);

  /* Popup box */
  popup = new popup_box_t(this);

  /* Add objects to interface container. */
  add_float(viewport, 0, 0);
  add_float(popup, 0, 0);
  add_float(panel, 0, 0);

  /* Game init box */
  init_box = new game_init_box_t(this);
  init_box->set_displayed(true);
  add_float(init_box, 0, 0);

  /* Notification box */
  notification_box = new notification_box_t(this);
  add_float(notification_box, 0, 0);

  map_cursor_pos = MAP_POS(0, 0);
  map_cursor_type = (map_cursor_type_t)0;
  panel_btn_type = (panel_btn_t)0;

  building_road = 0;

  player = NULL;

  /* Settings */
  config = 0x39;
  msg_flags = 0;
  return_timeout = 0;

  map_cursor_sprites[0] = 32;
  map_cursor_sprites[1] = 33;
  map_cursor_sprites[2] = 33;
  map_cursor_sprites[3] = 33;
  map_cursor_sprites[4] = 33;
  map_cursor_sprites[5] = 33;
  map_cursor_sprites[6] = 33;

  /* Randomness for interface */
  random = random_generate_random_state();

  last_const_tick = 0;

  /* Listen for updates to the map height */
  game.map.update_map_height_cb =
    (game_update_map_height_func *)update_map_height;
  game.map.update_map_height_data = this;
}

interface_t::~interface_t()
{
  delete viewport;
  delete panel;
  delete popup;
  delete init_box;
  delete notification_box;
}

/* Called periodically when the game progresses. */
void
interface_t::update()
{
  uint tick_diff = game.const_tick - last_const_tick;
  last_const_tick = game.const_tick;

  /* Update timers */
  for (int i = 0; i < player->timers_count; i++) {
    player->timers[i].timeout -= tick_diff;
    if (player->timers[i].timeout < 0) {
      /* Timer has expired. */
      /* TODO box (+ pos) timer */
      player_add_notification(player, 5,
            player->timers[i].pos);

      /* Delete timer from list. */
      player->timers_count -= 1;
      for (int j = i; j < player->timers_count; j++) {
        player->timers[j].timeout = player->timers[j+1].timeout;
        player->timers[j].pos = player->timers[j+1].pos;
      }
    }
  }

  /* Clear return arrow after a timeout */
  if (return_timeout < (int)tick_diff) {
    msg_flags |= BIT(4);
    msg_flags &= ~BIT(3);
    return_timeout = 0;
  } else {
    return_timeout -= tick_diff;
  }

  const int msg_category[] = {
    -1, 5, 5, 5, 4, 0, 4, 3, 4, 5,
    5, 5, 4, 4, 4, 4, 0, 0, 0, 0
  };

  /* Handle newly enqueued messages */
  if (PLAYER_HAS_MESSAGE(player)) {
    player->flags &= ~BIT(3);
    while (player->msg_queue_type[0] != 0) {
      int type = player->msg_queue_type[0] & 0x1f;
      if (BIT_TEST(config, msg_category[type])) {
        audio_t::get_audio()->sfx_play_clip(SFX_MESSAGE);
        msg_flags |= BIT(0);
        break;
      }

      /* Message is ignored. Remove. */
      int i;
      for (i = 1; i < 64 && player->msg_queue_type[i] != 0; i++) {
        player->msg_queue_type[i-1] = player->msg_queue_type[i];
        player->msg_queue_pos[i-1] = player->msg_queue_pos[i];
      }
      player->msg_queue_type[i-1] = 0;
    }
  }

  if (BIT_TEST(msg_flags, 1)) {
    msg_flags &= ~BIT(1);
    while (1) {
      if (player->msg_queue_type[0] == 0) {
        msg_flags &= ~BIT(0);
        break;
      }

      int type = player->msg_queue_type[0] & 0x1f;
      if (BIT_TEST(config, msg_category[type])) break;

      /* Message is ignored. Remove. */
      int i;
      for (i = 1; i < 64 && player->msg_queue_type[i] != 0; i++) {
        player->msg_queue_type[i-1] = player->msg_queue_type[i];
        player->msg_queue_pos[i-1] = player->msg_queue_pos[i];
      }
      player->msg_queue_type[i-1] = 0;
    }
  }

  viewport->update();
  set_redraw();
}

bool
interface_t::handle_key_pressed(char key, int modifier)
{
  switch (key) {
    /* Panel click shortcuts */
    case '1': {
      panel->activate_button(0);
      break;
    }
    case '2': {
      panel->activate_button(1);
      break;
    }
    case '3': {
      panel->activate_button(2);
      break;
    }
    case '4': {
      panel->activate_button(3);
    }
      break;
    case '5': {
      panel->activate_button(4);
      break;
    }

    /* Interface control */
    case '\t': {
      if (modifier & 2) {
        return_from_message();
      }
      else {
        open_message();
      }
      break;
    }
    case '\033': {
      if (get_notification_box()->is_displayed()) {
        close_message();
      }
      else if (get_popup_box()->is_displayed()) {
        close_popup();
      }
      else if (get_building_road()) {
        build_road_end();
      }
      break;
    }

    /* Game speed */
    case '+': {
      if (game.game_speed < 40) game.game_speed += 1;
      LOGI("game", "Game speed: %u", game.game_speed);
      break;
    }
    case '-': {
      if (game.game_speed >= 1) game.game_speed -= 1;
      LOGI("game", "Game speed: %u", game.game_speed);
      break;
    }
    case '0': {
      game.game_speed = DEFAULT_GAME_SPEED;
      LOGI("game", "Game speed: %u", game.game_speed);
      break;
    }
    case 'p': {
      game_pause((game.game_speed == 0) ? 0 : 1);
      break;
    }

    /* Audio */
    case 's': {
      audio_t::get_audio()->sfx_enable(!audio_t::get_audio()->sfx_is_enabled());
      break;
    }
    case 'm': {
      audio_t::get_audio()->midi_enable(!audio_t::get_audio()->midi_is_enabled());
      break;
    }

    /* Debug */
    case 'g': {
      viewport->switch_layer(VIEWPORT_LAYER_GRID);
      break;
    }

    /* Game control */
    case 'b': {
      viewport->switch_possible_build();
      break;
    }
    case 'j': {
      int current = 0;
      for (int i = 0; i < GAME_MAX_PLAYER_COUNT; i++) {
        if (get_player() == game.player[i]) {
          current = i;
          break;
        }
      }

      for (int i = (current+1) % GAME_MAX_PLAYER_COUNT;
           i != current; i = (i+1) % GAME_MAX_PLAYER_COUNT) {
        if (PLAYER_IS_ACTIVE(game.player[i])) {
          set_player(i);
          LOGD("main", "Switched to player %i.", i);
          break;
        }
      }
      break;
    }
    case 'z':
      if (modifier & 1) {
        save_game(0);
      }
      break;
    case 'n':
      if (modifier & 1) {
        open_game_init();
      }
      break;
    case 'c':
      if (modifier & 1) {
        open_popup(BOX_QUIT_CONFIRM);
      }
      break;

    default:
      return 0;
  }

  return true;
}

bool
interface_t::handle_event(const event_t *event)
{
  switch (event->type) {
    case EVENT_RESIZE:
      set_size(event->dx, event->dy);
      break;
    case EVENT_UPDATE:
      update();
      break;
    case EVENT_DRAW:
      draw((frame_t*)event->object);
      break;

    default:
      return gui_object_t::handle_event(event);
      break;
  }

  return true;
}
