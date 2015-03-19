/*
 * event_loop-win.h - User and system events handling
 *
 * Copyright (C) 2015  Wicked_Digger <wicked_digger@mail.ru>
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

#ifndef _EVENT_LOOP_WIN
#define _EVENT_LOOP_WIN

#include "event_loop.h"

#include <Windows.h>

class video_win_t;

class event_loop_win_t
  : public event_loop_t
{
private:
  bool dragging;
  int drag_pos_x;
  int drag_pos_y;
  int drag_start_pos_x;
  int drag_start_pos_y;

  video_win_t *video;

public:
  event_loop_win_t();

  virtual void quit();
  virtual void run(event_handler_t **handlers);

private:
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT process_event(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif /* ! _EVENT_LOOP_WIN */
