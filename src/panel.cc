/*
 * panel.c - Panel GUI component
 *
 * Copyright (C) 2013  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "panel.h"
#include "interface.h"
#include "minimap.h"
#include "popup.h"
#include "viewport.h"
#include "data.h"
#include "audio.h"
#include "freeserf.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "game.h"
#ifndef _MSC_VER
}
#endif

/* Draw the frame around action buttons. */
void
panel_bar_t::draw_panel_frame(frame_t *frame)
{
  const int bottom_svga_layout[] = {
    DATA_FRAME_BOTTOM_SHIELD, 0, 0,
    DATA_FRAME_BOTTOM_BASE+0, 40, 0,
    DATA_FRAME_BOTTOM_BASE+20, 48, 0,

    DATA_FRAME_BOTTOM_BASE+7, 64, 0,
    DATA_FRAME_BOTTOM_BASE+8, 64, 36,
    DATA_FRAME_BOTTOM_BASE+21, 96, 0,

    DATA_FRAME_BOTTOM_BASE+9, 112, 0,
    DATA_FRAME_BOTTOM_BASE+10, 112, 36,
    DATA_FRAME_BOTTOM_BASE+22, 144, 0,

    DATA_FRAME_BOTTOM_BASE+11, 160, 0,
    DATA_FRAME_BOTTOM_BASE+12, 160, 36,
    DATA_FRAME_BOTTOM_BASE+23, 192, 0,

    DATA_FRAME_BOTTOM_BASE+13, 208, 0,
    DATA_FRAME_BOTTOM_BASE+14, 208, 36,
    DATA_FRAME_BOTTOM_BASE+24, 240, 0,

    DATA_FRAME_BOTTOM_BASE+15, 256, 0,
    DATA_FRAME_BOTTOM_BASE+16, 256, 36,
    DATA_FRAME_BOTTOM_BASE+25, 288, 0,

    DATA_FRAME_BOTTOM_TIMERS, 304, 0,
    DATA_FRAME_BOTTOM_SHIELD, 312, 0,
    -1
  };

  /* TODO request full buffer swap(?) */

  const int *layout = bottom_svga_layout;

  /* Draw layout */
  for (int i = 0; layout[i] != -1; i += 3) {
    frame->draw_sprite(layout[i+1], layout[i+2], layout[i]);
  }
}

/* Draw notification icon in action panel. */
void
panel_bar_t::draw_message_notify(frame_t *frame)
{
  interface->set_msg_flags(interface->get_msg_flags() | BIT(2));
  frame->draw_sprite(40, 4, DATA_FRAME_BOTTOM_NOTIFY);
}

/* Draw return arrow icon in action panel. */
static void
draw_return_arrow(panel_bar_t *panel, frame_t *frame)
{
  frame->draw_sprite(40, 28, DATA_FRAME_BOTTOM_ARROW);
}

/* Draw buttons in action panel. */
void
panel_bar_t::draw_panel_buttons(frame_t *frame)
{
  if (enabled) {
    /* Blinking message icon. */
    if (interface->get_player()->msg_queue_type[0] != 0) {
      if (game.const_tick & 0x30) {
        draw_message_notify(frame);
      }
    }

    /* Return arrow icon. */
    if (BIT_TEST(interface->get_msg_flags(), 3)) {
      draw_return_arrow(this, frame);
    }
  }

  const int inactive_buttons[] = {
    PANEL_BTN_BUILD_INACTIVE,
    PANEL_BTN_DESTROY_INACTIVE,
    PANEL_BTN_MAP_INACTIVE,
    PANEL_BTN_STATS_INACTIVE,
    PANEL_BTN_SETT_INACTIVE
  };

  for (int i = 0; i < 5; i++) {
    int button = panel_btns[i];
    if (!enabled) button = inactive_buttons[i];

    int x = 64 + i*48;
    int y = 4;
    int sprite = DATA_FRAME_BUTTON_BASE + button;

    frame->draw_sprite(x, y, sprite);
  }
}

void
panel_bar_t::internal_draw()
{
  draw_panel_frame(frame);
  draw_panel_buttons(frame);
}

/* Handle a click on the panel buttons. */
void
panel_bar_t::handle_panel_button_click(int button)
{
  switch (panel_btns[button]) {
  case PANEL_BTN_MAP:
  case PANEL_BTN_MAP_STARRED:
    sfx_play_clip(SFX_CLICK);
    if (interface->get_popup_box()->is_displayed()) {
      interface->close_popup();
    } else {
      panel_btns[0] = PANEL_BTN_BUILD_INACTIVE;
      panel_btns[1] = PANEL_BTN_DESTROY_INACTIVE;
      panel_btns[2] = PANEL_BTN_MAP_STARRED;
      panel_btns[3] = PANEL_BTN_STATS_INACTIVE;
      panel_btns[4] = PANEL_BTN_SETT_INACTIVE;

      /* Synchronize minimap window with viewport. */
      viewport_t *viewport = interface->get_viewport();
      popup_box_t *popup = interface->get_popup_box();
      map_pos_t pos = viewport->get_current_map_pos();
      popup->get_minimap()->move_to_map_pos(pos);

      interface->open_popup(BOX_MAP);
    }
    break;
  case PANEL_BTN_SETT:
  case PANEL_BTN_SETT_STARRED:
    sfx_play_clip(SFX_CLICK);
    if (interface->get_popup_box()->is_displayed()) {
      interface->close_popup();
    } else {
      panel_btns[0] = PANEL_BTN_BUILD_INACTIVE;
      panel_btns[1] = PANEL_BTN_DESTROY_INACTIVE;
      panel_btns[2] = PANEL_BTN_MAP_INACTIVE;
      panel_btns[3] = PANEL_BTN_STATS_INACTIVE;
      panel_btns[4] = PANEL_BTN_SETT_STARRED;
      interface->open_popup(BOX_SETT_SELECT);
    }
    break;
  case PANEL_BTN_STATS:
  case PANEL_BTN_STATS_STARRED:
    sfx_play_clip(SFX_CLICK);
    if (interface->get_popup_box()->is_displayed()) {
      interface->close_popup();
    } else {
      panel_btns[0] = PANEL_BTN_BUILD_INACTIVE;
      panel_btns[1] = PANEL_BTN_DESTROY_INACTIVE;
      panel_btns[2] = PANEL_BTN_MAP_INACTIVE;
      panel_btns[3] = PANEL_BTN_STATS_STARRED;
      panel_btns[4] = PANEL_BTN_SETT_INACTIVE;
      interface->open_popup(BOX_STAT_SELECT);
    }
    break;
  case PANEL_BTN_BUILD_ROAD:
  case PANEL_BTN_BUILD_ROAD_STARRED:
    sfx_play_clip(SFX_CLICK);
    if (interface->get_building_road()) {
      interface->build_road_end();
    } else {
      interface->build_road_begin();
    }
    break;
  case PANEL_BTN_BUILD_FLAG:
    sfx_play_clip(SFX_CLICK);
    interface->build_flag();
    break;
  case PANEL_BTN_BUILD_SMALL:
  case PANEL_BTN_BUILD_SMALL_STARRED:
    sfx_play_clip(SFX_CLICK);
    if (interface->get_popup_box()->is_displayed()) {
      interface->close_popup();
    } else {
      panel_btns[0] = PANEL_BTN_BUILD_SMALL_STARRED;
      panel_btns[1] = PANEL_BTN_DESTROY_INACTIVE;
      panel_btns[2] = PANEL_BTN_MAP_INACTIVE;
      panel_btns[3] = PANEL_BTN_STATS_INACTIVE;
      panel_btns[4] = PANEL_BTN_SETT_INACTIVE;
      interface->open_popup(BOX_BASIC_BLD);
    }
    break;
  case PANEL_BTN_BUILD_LARGE:
  case PANEL_BTN_BUILD_LARGE_STARRED:
    sfx_play_clip(SFX_CLICK);
    if (interface->get_popup_box()->is_displayed()) {
      interface->close_popup();
    } else {
      panel_btns[0] = PANEL_BTN_BUILD_LARGE_STARRED;
      panel_btns[1] = PANEL_BTN_DESTROY_INACTIVE;
      panel_btns[2] = PANEL_BTN_MAP_INACTIVE;
      panel_btns[3] = PANEL_BTN_STATS_INACTIVE;
      panel_btns[4] = PANEL_BTN_SETT_INACTIVE;
      interface->open_popup(BOX_BASIC_BLD_FLIP);
    }
    break;
  case PANEL_BTN_BUILD_MINE:
  case PANEL_BTN_BUILD_MINE_STARRED:
    sfx_play_clip(SFX_CLICK);
    if (interface->get_popup_box()->is_displayed()) {
      interface->close_popup();
    } else {
      panel_btns[0] = PANEL_BTN_BUILD_MINE_STARRED;
      panel_btns[1] = PANEL_BTN_DESTROY_INACTIVE;
      panel_btns[2] = PANEL_BTN_MAP_INACTIVE;
      panel_btns[3] = PANEL_BTN_STATS_INACTIVE;
      panel_btns[4] = PANEL_BTN_SETT_INACTIVE;
      interface->open_popup(BOX_MINE_BUILDING);
    }
    break;
  case PANEL_BTN_DESTROY:
    if (interface->current_map_cursor_type() == MAP_CURSOR_TYPE_REMOVABLE_FLAG) {
      interface->demolish_object();
    } else {
      panel_btns[0] = PANEL_BTN_BUILD_INACTIVE;
      panel_btns[1] = PANEL_BTN_DESTROY_INACTIVE;
      panel_btns[2] = PANEL_BTN_MAP_INACTIVE;
      panel_btns[3] = PANEL_BTN_STATS_INACTIVE;
      panel_btns[4] = PANEL_BTN_SETT_INACTIVE;
      interface->open_popup(BOX_DEMOLISH);
    }
    break;
  case PANEL_BTN_BUILD_CASTLE:
    interface->build_castle();
    break;
  case PANEL_BTN_DESTROY_ROAD:
  {
    int r = game_demolish_road(interface->get_map_cursor_pos(),
             interface->get_player());
    if (r < 0) {
      sfx_play_clip(SFX_NOT_ACCEPTED);
      interface->update_map_cursor_pos(interface->get_map_cursor_pos());
    } else {
      sfx_play_clip(SFX_ACCEPTED);
    }
  }
    break;
  case PANEL_BTN_GROUND_ANALYSIS:
  case PANEL_BTN_GROUND_ANALYSIS_STARRED:
    sfx_play_clip(SFX_CLICK);
    if (interface->get_popup_box()->is_displayed()) {
      interface->close_popup();
    } else {
      panel_btns[0] = PANEL_BTN_BUILD_INACTIVE;
      panel_btns[1] = PANEL_BTN_GROUND_ANALYSIS_STARRED;
      panel_btns[2] = PANEL_BTN_MAP_INACTIVE;
      panel_btns[3] = PANEL_BTN_STATS_INACTIVE;
      panel_btns[4] = PANEL_BTN_SETT_INACTIVE;
      interface->open_popup(BOX_GROUND_ANALYSIS);
    }
    break;
  }
}

int
panel_bar_t::handle_click_left(int x, int y)
{
  set_redraw();

  if (x >= 41 && x < 53) {
    /* Message bar click */
    if (BIT_TEST(game.svga, 3)) { /* Game has started */
      if (y < 16) {
        /* Message icon */
        interface->open_message();
      } else if (y >= 28) {
        /* Return arrow */
        interface->return_from_message();
      }
    }
  } else if (x >= 301 && x < 313) {
    /* Timer bar click */
    if (BIT_TEST(game.svga, 3)) { /* Game has started */
      if (interface->get_player()->timers_count >= 64) {
        sfx_play_clip(SFX_NOT_ACCEPTED);
        return 0;
      }

      /* Call to map position */
      int timer_id = interface->get_player()->timers_count++;
      int timer_length = -1;

      if (y < 7) timer_length = 5*60;
      else if (y < 14) timer_length = 10*60;
      else if (y < 21) timer_length = 20*60;
      else if (y < 28) timer_length = 30*60;
      else timer_length = 60*60;

      interface->get_player()->timers[timer_id].timeout = timer_length*TICKS_PER_SEC;
      interface->get_player()->timers[timer_id].pos = interface->get_map_cursor_pos();

      sfx_play_clip(SFX_ACCEPTED);
    }
  } else if (y >= 4 && y < 36 && x >= 64) {
    x -= 64;

    /* Figure out what button was clicked */
    int button = 0;
    while (1) {
      if (x < 32) {
        if (button < 5) break;
        else return 0;
      }
      button += 1;
      if (x < 48) return 0;
      x -= 48;
    }
    handle_panel_button_click(button);
  }

  return 1;
}

panel_bar_t::panel_bar_t(interface_t *interface)
  : gui_object_t()
{
  this->interface = interface;

  panel_btns[0] = PANEL_BTN_BUILD_INACTIVE;
  panel_btns[1] = PANEL_BTN_DESTROY_INACTIVE;
  panel_btns[2] = PANEL_BTN_MAP;
  panel_btns[3] = PANEL_BTN_STATS;
  panel_btns[4] = PANEL_BTN_SETT;
}

void
panel_bar_t::activate_button(int button)
{
  handle_panel_button_click(button);
}

void
panel_bar_t::set_button_type(int button, panel_btn_t type)
{
  panel_btns[button] = type;
  set_redraw();
}
