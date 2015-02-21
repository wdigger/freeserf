/*
 * event_loop.cc - User and system events handling
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

#include "event_loop.h"
#include "gfx.h"
#include "freeserf.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
#ifndef _MSC_VER
}
#endif

#include "SDL.h"

/* How fast consequtive mouse events need to be generated
 in order to be interpreted as click and double click. */
#define MOUSE_TIME_SENSITIVITY  600
/* How much the mouse can move between events to be still
 considered as a double click. */
#define MOUSE_MOVE_SENSITIVITY  8

event_loop_t *event_loop_t::instance = NULL;

event_loop_t *
event_loop_t::get_instance()
{
  if (instance == NULL) {
    instance = new event_loop_t();
  }

  return instance;
}

event_loop_t::event_loop_t()
{
  event_handlers = NULL;
}

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
event_loop_t::quit()
{
  SDL_Event event;
  event.type = SDL_USEREVENT;
  event.user.type = SDL_USEREVENT;
  event.user.code = SDL_QUIT;
  event.user.data1 = 0;
  event.user.data2 = 0;
  SDL_PushEvent(&event);
}

void
event_loop_t::notify_handlers(event_t *event) {
  if (event_handlers == NULL) {
    return;
  }

  for (int i = 0; event_handlers[i] != NULL; i++) {
    event_handlers[i]->handle_event(event);
  }
}

/* event_loop() has been turned into a SDL based loop.
 The code for one iteration of the original game_loop is
 in game_loop_iter. */
void
event_loop_t::run(event_handler_t **handlers)
{
  SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_TIMER);

  SDL_TimerID timer_id = SDL_AddTimer(TICK_LENGTH, timer_callback, 0);
  if (timer_id == 0) {
    return;
  }

  event_handlers = handlers;

  int drag_button = 0;
  int drag_x = 0;
  int drag_y = 0;

  uint last_down[3] = {0};
  uint last_click[3] = {0};
  uint last_click_x = 0;
  uint last_click_y = 0;

  SDL_Event event;
  event_t ev;

  gfx_t *gfx = gfx_t::get_gfx();
  frame_t *screen = NULL;

  while (SDL_WaitEvent(&event)) {
    uint current_ticks = SDL_GetTicks();

    switch (event.type) {
      case SDL_MOUSEBUTTONUP:
        if (drag_button == event.button.button) {
          drag_button = 0;
        }

        if (event.button.button <= 3 &&
            current_ticks - last_down[event.button.button-1] < MOUSE_TIME_SENSITIVITY) {
          ev.type = EVENT_TYPE_CLICK;
          ev.x = event.button.x;
          ev.y = event.button.y;
          ev.button = (event_button_t)event.button.button;
          notify_handlers(&ev);

          if (current_ticks - last_click[event.button.button-1] < MOUSE_TIME_SENSITIVITY &&
              event.button.x >= static_cast<int>(last_click_x - MOUSE_MOVE_SENSITIVITY) &&
              event.button.x <= static_cast<int>(last_click_x + MOUSE_MOVE_SENSITIVITY) &&
              event.button.y >= static_cast<int>(last_click_y - MOUSE_MOVE_SENSITIVITY) &&
              event.button.y <= static_cast<int>(last_click_y + MOUSE_MOVE_SENSITIVITY)) {
            ev.type = EVENT_TYPE_DBL_CLICK;
            ev.x = event.button.x;
            ev.y = event.button.y;
            ev.button = (event_button_t)event.button.button;
            notify_handlers(&ev);
          }

          last_click[event.button.button-1] = current_ticks;
          last_click_x = event.button.x;
          last_click_y = event.button.y;
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button <= 3) last_down[event.button.button-1] = current_ticks;
        break;
      case SDL_MOUSEMOTION:
        for (int button = 1; button <= 3; button++) {
          if (event.motion.state & SDL_BUTTON(button)) {
            if (drag_button == 0) {
              drag_button = button;
              drag_x = event.motion.x;
              drag_y = event.motion.y;
            }

            ev.type = EVENT_TYPE_DRAG;
            ev.x = drag_x;
            ev.y = drag_y;
            ev.dx = event.motion.x - drag_x;
            ev.dy = event.motion.y - drag_y;
            ev.button = (event_button_t)drag_button;
            notify_handlers(&ev);

            gfx->warp_mouse(drag_x, drag_y);

            break;
          }
        }
        break;
      case SDL_KEYDOWN: {
        if (event.key.keysym.sym == SDLK_q &&
            (event.key.keysym.mod & KMOD_CTRL)) {
          quit();
          break;
        }

        ev.type = EVENT_KEY_PRESSED;
        ev.dx = event.key.keysym.sym;
        ev.dy = 0;
        if (event.key.keysym.mod & KMOD_CTRL) {
          ev.dy |= 1;
        }
        if (event.key.keysym.mod & KMOD_SHIFT) {
          ev.dy |= 2;
        }
        if (event.key.keysym.mod & KMOD_ALT) {
          ev.dy |= 4;
        }

        switch (event.key.keysym.sym) {
          /* Map scroll */
          case SDLK_UP: {
            ev.type = EVENT_TYPE_DRAG;
            ev.x = 0;
            ev.y = 0;
            ev.dx = 0;
            ev.dy = -32;
            ev.button = EVENT_BUTTON_LEFT;
            notify_handlers(&ev);
          }
            break;
          case SDLK_DOWN: {
            ev.type = EVENT_TYPE_DRAG;
            ev.x = 0;
            ev.y = 0;
            ev.dx = 0;
            ev.dy = 32;
            ev.button = EVENT_BUTTON_LEFT;
            notify_handlers(&ev);
          }
            break;
          case SDLK_LEFT: {
            ev.type = EVENT_TYPE_DRAG;
            ev.x = 0;
            ev.y = 0;
            ev.dx = -32;
            ev.dy = 0;
            ev.button = EVENT_BUTTON_LEFT;
            notify_handlers(&ev);
          }
            break;
          case SDLK_RIGHT: {
            ev.type = EVENT_TYPE_DRAG;
            ev.x = 0;
            ev.y = 0;
            ev.dx = 32;
            ev.dy = 0;
            ev.button = EVENT_BUTTON_LEFT;
            notify_handlers(&ev);
          }
            break;

          case SDLK_PLUS:
          case SDLK_KP_PLUS:
          case SDLK_EQUALS:
            ev.dx = '+';
            notify_handlers(&ev);
            break;
          case SDLK_MINUS:
          case SDLK_KP_MINUS:
            ev.dx = '-';
            notify_handlers(&ev);
            break;

            /* Video */
          case SDLK_f:
            if (event.key.keysym.mod & KMOD_CTRL) {
              gfx->set_fullscreen(!gfx->is_fullscreen());
            }
            break;

            /* Misc */
          case SDLK_F10:
            ev.dx = 'n';
            ev.dy = 1;
            notify_handlers(&ev);
            break;

          default:
            notify_handlers(&ev);
            break;
        }

        break;
      }
      case SDL_QUIT:
        ev.type = EVENT_KEY_PRESSED;
        ev.dx = 'c';
        ev.dy = 1;
        notify_handlers(&ev);
        break;
      case SDL_WINDOWEVENT:
        if (SDL_WINDOWEVENT_SIZE_CHANGED == event.window.event){
          unsigned int width = 0;
          unsigned int height = 0;
          gfx->get_resolution(width, height);
          gfx->set_resolution(width, height, gfx->is_fullscreen());
          ev.type = EVENT_RESIZE;
          ev.x = width;
          ev.y = height;
          notify_handlers(&ev);
        }
        break;
      case SDL_USEREVENT:
        if (event.user.code == SDL_QUIT) {
          if (screen != NULL) {
            delete screen;
          }
          return;
        }

        /* Update and draw interface */
        ev.type = EVENT_UPDATE;
        notify_handlers(&ev);

        if (screen == NULL) {
          screen = gfx->get_screen_frame();
        }
        ev.type = EVENT_DRAW;
        ev.object = screen;
        notify_handlers(&ev);

        /* Swap video buffers */
        gfx->swap_buffers();

        break;
    }
  }
  
  if (screen != NULL) {
    delete screen;
  }
}
