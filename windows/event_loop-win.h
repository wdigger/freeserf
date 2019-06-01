/*
 * event_loop-win.h - User and system events handling
 *
 * Copyright (C) 2019  Wicked_Digger <wicked_digger@mail.ru>
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

#ifndef WINDOWS_EVENT_LOOP_WIN_H_
#define WINDOWS_EVENT_LOOP_WIN_H_

#include <Windows.h>

#include "src/event_loop.h"

class VideoWin;

class EventLoopWin : public EventLoop {
 private:
  bool dragging;
  int drag_pos_x;
  int drag_pos_y;
  int drag_start_pos_x;
  int drag_start_pos_y;

  VideoWin *video;

 public:
  EventLoopWin();

  virtual void quit();
  virtual void run();
  virtual void deferred_call(DeferredCall deferred_call, void *data);

 private:
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                                  WPARAM wParam, LPARAM lParam);
  LRESULT process_event(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif  // WINDOWS_EVENT_LOOP_WIN_H_
