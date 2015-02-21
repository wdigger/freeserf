/*
 * event_loop.h - User and system events handling
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

#ifndef _EVENT_LOOP
#define _EVENT_LOOP

typedef enum {
  EVENT_TYPE_CLICK,
  EVENT_TYPE_DBL_CLICK,
  EVENT_TYPE_DRAG,
  EVENT_KEY_PRESSED,
  EVENT_RESIZE,
  EVENT_UPDATE,
  EVENT_DRAW,
} event_type_t;

typedef enum {
  EVENT_BUTTON_LEFT = 1,
  EVENT_BUTTON_MIDDLE,
  EVENT_BUTTON_RIGHT
} event_button_t;

typedef struct {
  event_type_t type;
  int x, y;
  int dx, dy;
  void *object;
  event_button_t button;
} event_t;

class frame_t;

class event_handler_t {
public:
  virtual bool handle_event(const event_t *event) = 0;
};

class event_loop_t
{
protected:
  static event_loop_t *instance;
  event_handler_t **event_handlers;

public:
  static event_loop_t *get_instance();
  virtual void quit() = 0;
  virtual void run(event_handler_t **handlers) = 0;

protected:
  event_loop_t();

  bool notify_handlers(event_t *event);

  bool notify_click(int x, int y, event_button_t button);
  bool notify_dbl_click(int x, int y, event_button_t button);
  bool notify_drag(int x, int y, int dx, int dy, event_button_t button);
  bool notify_key_pressed(unsigned char key, unsigned char morifier);
  bool notify_resize(unsigned int width, unsigned int height);
  bool notify_update();
  bool notify_draw(frame_t *frame);
};

#endif /* ! _EVENT_LOOP */
