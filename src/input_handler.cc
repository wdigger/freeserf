/*
 * input_handler.h - User and system events handling
 *
 * Copyright (C) 2012-2014  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "input_handler.h"
#include "interface.h"
#include "viewport.h"
#include "notification.h"
#include "popup.h"
#include "audio.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
  #include "savegame.h"
  #include "game.h"
#ifndef _MSC_VER
}
#endif

#include "SDL.h"

/* Autosave interval */
#define AUTOSAVE_INTERVAL  (10*60*TICKS_PER_SEC)

/* How fast consequtive mouse events need to be generated
 in order to be interpreted as click and double click. */
#define MOUSE_TIME_SENSITIVITY  600
/* How much the mouse can move between events to be still
 considered as a double click. */
#define MOUSE_MOVE_SENSITIVITY  8

Uint32 timer_callback(Uint32 interval, void *param)
{
  SDL_Event event;
  event.type = SDL_USEREVENT;
  event.user.type = SDL_USEREVENT;
  event.user.code = 0;
  event.user.data1 = 0;
  event.user.data2 = 0;
  SDL_PushEvent(&event);

  return interval;
}

void
game_loop_quit()
{
  SDL_Event event;
  event.type = SDL_USEREVENT;
  event.user.type = SDL_USEREVENT;
  event.user.code = SDL_QUIT;
  event.user.data1 = 0;
  event.user.data2 = 0;
  SDL_PushEvent(&event);
}

/* game_loop() has been turned into a SDL based loop.
 The code for one iteration of the original game_loop is
 in game_loop_iter. */
void
game_loop(interface_t *interface)
{
  SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_TIMER);

  SDL_TimerID timer_id = SDL_AddTimer(TICK_LENGTH, timer_callback, 0);
  if (timer_id == 0) {
    return;
  }

  int drag_button = 0;
  int drag_x = 0;
  int drag_y = 0;

  uint last_down[3] = {0};
  uint last_click[3] = {0};
  uint last_click_x = 0;
  uint last_click_y = 0;

  SDL_Event event;
  gui_event_t ev;

  gfx_t *gfx = gfx_t::get_gfx();
  frame_t *screen = NULL;

  while (SDL_WaitEvent(&event)) {
    uint current_ticks = SDL_GetTicks();

    switch (event.type) {
      case SDL_MOUSEBUTTONUP:
        if (drag_button == event.button.button) {
          ev.type = GUI_EVENT_TYPE_DRAG_END;
          ev.x = event.button.x;
          ev.y = event.button.y;
          ev.button = (gui_event_button_t)drag_button;
          interface->handle_event(&ev);

          drag_button = 0;
        }

        ev.type = GUI_EVENT_TYPE_BUTTON_UP;
        ev.x = event.button.x;
        ev.y = event.button.y;
        ev.button = (gui_event_button_t)event.button.button;
        interface->handle_event(&ev);

        if (event.button.button <= 3 &&
            current_ticks - last_down[event.button.button-1] < MOUSE_TIME_SENSITIVITY) {
          ev.type = GUI_EVENT_TYPE_CLICK;
          ev.x = event.button.x;
          ev.y = event.button.y;
          ev.button = (gui_event_button_t)event.button.button;
          interface->handle_event(&ev);

          if (current_ticks - last_click[event.button.button-1] < MOUSE_TIME_SENSITIVITY &&
              event.button.x >= static_cast<int>(last_click_x - MOUSE_MOVE_SENSITIVITY) &&
              event.button.x <= static_cast<int>(last_click_x + MOUSE_MOVE_SENSITIVITY) &&
              event.button.y >= static_cast<int>(last_click_y - MOUSE_MOVE_SENSITIVITY) &&
              event.button.y <= static_cast<int>(last_click_y + MOUSE_MOVE_SENSITIVITY)) {
            ev.type = GUI_EVENT_TYPE_DBL_CLICK;
            ev.x = event.button.x;
            ev.y = event.button.y;
            ev.button = (gui_event_button_t)event.button.button;
            interface->handle_event(&ev);
          }

          last_click[event.button.button-1] = current_ticks;
          last_click_x = event.button.x;
          last_click_y = event.button.y;
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        ev.type = GUI_EVENT_TYPE_BUTTON_DOWN;
        ev.x = event.button.x;
        ev.y = event.button.y;
        ev.button = (gui_event_button_t)event.button.button;
        interface->handle_event(&ev);

        if (event.button.button <= 3) last_down[event.button.button-1] = current_ticks;
        break;
      case SDL_MOUSEMOTION:
        for (int button = 1; button <= 3; button++) {
          if (event.motion.state & SDL_BUTTON(button)) {
            if (drag_button == 0) {
              drag_button = button;
              drag_x = event.motion.x;
              drag_y = event.motion.y;

              ev.type = GUI_EVENT_TYPE_DRAG_START;
              ev.x = event.motion.x;
              ev.y = event.motion.y;
              ev.button = (gui_event_button_t)drag_button;
              interface->handle_event(&ev);
            }

            ev.type = GUI_EVENT_TYPE_DRAG_MOVE;
            ev.x = drag_x;
            ev.y = drag_y;
            ev.dx = event.motion.x - drag_x;
            ev.dy = event.motion.y - drag_y;
            ev.button = (gui_event_button_t)drag_button;
            interface->handle_event(&ev);

            gfx->warp_mouse(drag_x, drag_y);

            break;
          }
        }
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym <= SDLK_z) {
          ev.type = GUI_EVENT_KEY_PRESSED;
          ev.dx = event.key.keysym.sym;
          ev.dy = event.key.keysym.mod;
          if (interface->handle_event(&ev)) {
            break;
          }
        }

        if (event.key.keysym.sym == SDLK_q &&
            (event.key.keysym.mod & KMOD_CTRL)) {
          game_loop_quit();
          break;
        }

        switch (event.key.keysym.sym) {
            /* Map scroll */
          case SDLK_UP: {
            viewport_t *viewport = interface->get_viewport();
            viewport->move_by_pixels(0, -32);
          }
            break;
          case SDLK_DOWN: {
            viewport_t *viewport = interface->get_viewport();
            viewport->move_by_pixels(0, 32);
          }
            break;
          case SDLK_LEFT: {
            viewport_t *viewport = interface->get_viewport();
            viewport->move_by_pixels(-32, 0);
          }
            break;
          case SDLK_RIGHT: {
            viewport_t *viewport = interface->get_viewport();
            viewport->move_by_pixels(32, 0);
          }
            break;

          case SDLK_TAB:
            if (event.key.keysym.mod & KMOD_SHIFT) {
              interface->return_from_message();
            } else {
              interface->open_message();
            }
            break;

            /* Game speed */
          case SDLK_PLUS:
          case SDLK_KP_PLUS:
          case SDLK_EQUALS:
            if (game.game_speed < 40) game.game_speed += 1;
            LOGI("main", "Game speed: %u", game.game_speed);
            break;
          case SDLK_MINUS:
          case SDLK_KP_MINUS:
            if (game.game_speed >= 1) game.game_speed -= 1;
            LOGI("main", "Game speed: %u", game.game_speed);
            break;
          case SDLK_0:
            game.game_speed = DEFAULT_GAME_SPEED;
            LOGI("main", "Game speed: %u", game.game_speed);
            break;
          case SDLK_p:
            if (game.game_speed == 0) game_pause(0);
            else game_pause(1);
            break;

            /* Audio */
          case SDLK_s:
            sfx_enable(!sfx_is_enabled());
            break;
          case SDLK_m:
            midi_enable(!midi_is_enabled());
            break;

            /* Video */
          case SDLK_f:
            if (event.key.keysym.mod & KMOD_CTRL) {
              gfx->set_fullscreen(!gfx->is_fullscreen());
            }
            break;

            /* Misc */
          case SDLK_ESCAPE:
            if (interface->get_notification_box()->is_displayed()) {
              interface->close_message();
            } else if (interface->get_popup_box()->is_displayed()) {
              interface->close_popup();
            } else if (interface->get_building_road()) {
              interface->build_road_end();
            }
            break;

            /* Debug */
          case SDLK_g: {
            viewport_t *viewport = interface->get_viewport();
            viewport->switch_layer(VIEWPORT_LAYER_GRID);
          }
            break;
          case SDLK_b: {
            viewport_t *viewport = interface->get_viewport();
            viewport->switch_possible_build();
          }
            break;
          case SDLK_j: {
            int current = 0;
            for (int i = 0; i < GAME_MAX_PLAYER_COUNT; i++) {
              if (interface->get_player() == game.player[i]) {
                current = i;
                break;
              }
            }

            for (int i = (current+1) % GAME_MAX_PLAYER_COUNT;
                 i != current; i = (i+1) % GAME_MAX_PLAYER_COUNT) {
              if (PLAYER_IS_ACTIVE(game.player[i])) {
                interface->set_player(i);
                LOGD("main", "Switched to player %i.", i);
                break;
              }
            }
          }
            break;
          case SDLK_z:
            if (event.key.keysym.mod & KMOD_CTRL) {
              save_game(0);
            }
            break;
          case SDLK_F10:
            interface->open_game_init();
            break;

          default:
            break;
        }
        break;
      case SDL_QUIT:
        interface->open_popup(BOX_QUIT_CONFIRM);
        break;
      case SDL_WINDOWEVENT:
        if (SDL_WINDOWEVENT_SIZE_CHANGED == event.window.event){
          unsigned int width = 0;
          unsigned int height = 0;
          gfx->get_resolution(width, height);
          gfx->set_resolution(width, height, gfx->is_fullscreen());
          interface->set_size(width, height);
        }
        break;
      case SDL_USEREVENT:
        if (event.user.code == SDL_QUIT) {
          if (screen != NULL) {
            delete screen;
          }
          return;
        }
        game_update();

        /* Autosave periodically */
        if ((game.const_tick % AUTOSAVE_INTERVAL) == 0 &&
            game.game_speed > 0) {
          int r = save_game(1);
          if (r < 0) LOGW("main", "Autosave failed.");
        }

        /* Update and draw interface */
        interface->update();

        if (screen == NULL) {
          screen = gfx->get_screen_frame();
        }
        interface->draw(screen);

        /* Swap video buffers */
        gfx->swap_buffers();

        break;
    }
  }
  
  if (screen != NULL) {
    delete screen;
  }
}
