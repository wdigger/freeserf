/*
 * event_loop-win.cc - User and system events handling
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

#include "event_loop-win.h"
#include "gfx.h"
#include "freeserf.h"
#include "video-win.h"
#include "application.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
#ifndef _MSC_VER
}
#endif

event_loop_t *
create_event_loop()
{
  return new event_loop_win_t();
}

/* How fast consequtive mouse events need to be generated
 in order to be interpreted as click and double click. */
#define MOUSE_TIME_SENSITIVITY  600
/* How much the mouse can move between events to be still
 considered as a double click. */
#define MOUSE_MOVE_SENSITIVITY  8

event_loop_win_t::event_loop_win_t()
{
  dragging = false;
  video = (video_win_t*)application_t::get_application()->get_video();
}

void
event_loop_win_t::quit()
{
  ::PostQuitMessage(0);
}

LRESULT CALLBACK
event_loop_win_t::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  event_loop_win_t *pthis = (event_loop_win_t*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
  return pthis->process_event(hWnd, message, wParam, lParam);
}

LRESULT
event_loop_win_t::process_event(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message){
    case WM_SYSCOMMAND: {
      if (wParam == SC_CLOSE) {
        notify_key_pressed('c', 1);
        return 0;
      }
      break;
    }
    case WM_ERASEBKGND:
      return 1;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      RECT cr = { 0 };
      ::GetClientRect(hWnd, &cr);
      video_frame_t *screen = video->frame_create(cr.right - cr.left, cr.bottom - cr.top);
      frame_t *frame = new frame_t(screen, gfx_t::get_gfx());
      notify_draw(frame);
      Gdiplus::Graphics *graphics = new Gdiplus::Graphics(hdc);
      graphics->DrawImage(((frame_win_t*)screen)->get_texture(), 0, 0);
      delete graphics;
      delete frame;
      EndPaint(hWnd, &ps);
      return 0;
      break;
    }
    case WM_SIZE: {
      notify_resize(LOWORD(lParam), HIWORD(lParam));
      break;
    }
    case WM_SIZING: {
      RECT *pRect = (RECT*)lParam;
      notify_resize(pRect->right - pRect->left, pRect->bottom - pRect->top);
      break;
    }
    case WM_KEYDOWN: {
      switch (wParam) {
      case VK_LEFT:
        notify_drag(0, 0, -32, 0, EVENT_BUTTON_LEFT);
        break;
      case VK_RIGHT:
        notify_drag(0, 0, 32, 0, EVENT_BUTTON_LEFT);
        break;
      case VK_UP:
        notify_drag(0, 0, 0, -32, EVENT_BUTTON_LEFT);
        break;
      case VK_DOWN:
        notify_drag(0, 0, 0, 32, EVENT_BUTTON_LEFT);
        break;
      case VK_F10:
        notify_key_pressed('n', 1);
        break;
      }

      unsigned char key = MapVirtualKey((UINT)wParam, MAPVK_VK_TO_CHAR);
      if (key == 0) {
        return 0;
      }

      char modifier = 0;
      if (GetKeyState(VK_CONTROL)) {
        if (lParam & (1 << 29)) {
          modifier |= 4;
        }
        else {
          modifier |= 1;
        }
      }
      if (GetKeyState(VK_SHIFT)) {
        modifier |= 2;
      }

      if (key >= 'A' && key <= 'Z') {
        key = 'a' + (key - 'A');
      }
      else if (key == '=') {
        key = '+';
      }

      if (key == 'q' && modifier == 1) {
        quit();
        return 0;
      }

      notify_key_pressed(key, modifier);

      return 0;
      break;
    }
    case WM_RBUTTONDOWN: {
      notify_click(LOWORD(lParam), HIWORD(lParam), EVENT_BUTTON_RIGHT);

      dragging = true;
      POINT point;
      ::GetCursorPos(&point);
      drag_start_pos_x = drag_pos_x = point.x;
      drag_start_pos_y = drag_pos_y = point.y;
      ::SetCapture(hWnd);
      ::ShowCursor(FALSE);

      break;
    }
    case WM_RBUTTONUP: {
      dragging = false;
      ::SetCursorPos(drag_start_pos_x, drag_start_pos_y);
      ::ReleaseCapture();
      ::ShowCursor(TRUE);
      break;
    }
    case WM_LBUTTONDOWN: {
      notify_click(LOWORD(lParam), HIWORD(lParam), EVENT_BUTTON_LEFT);
      break;
    }
    case WM_MOUSEMOVE: {
      if (dragging) {
        POINT point;
        ::GetCursorPos(&point);
        notify_drag(drag_start_pos_x, drag_start_pos_y, point.x - drag_pos_x, point.y - drag_pos_y, EVENT_BUTTON_RIGHT);
        drag_pos_x = point.x;
        drag_pos_y = point.y;
      }
      break;
    }
    case WM_TIMER: {
      if (wParam == 123) {
        notify_update();
        RECT rect;
        ::GetClientRect(hWnd, &rect);
        ::InvalidateRect(hWnd, &rect, FALSE);
      }
      return 0;
      break;
    }
  }

  return ::DefWindowProc(hWnd, message, wParam, lParam);
}

/* event_loop() has been turned into a SDL based loop.
 The code for one iteration of the original game_loop is
 in game_loop_iter. */
void
event_loop_win_t::run(event_handler_t **handlers)
{
  HWND window = video->get_window();
  if (window == NULL) {
    return;
  }

  event_handlers = handlers;

  ::SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)this);
  ::SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

  ::SetTimer(window, 123, TICK_LENGTH, NULL); // Game loop timer

  MSG msg = { 0 };
  while (0 != ::GetMessage(&msg, NULL, 0, 0)) {
    ::DispatchMessage(&msg);
  }
}

extern int freeserf_main(int argc, char *argv[]);

int __stdcall
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  int argc = 0;
  LPWSTR* lpArgv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

  char **argv = NULL;
  if (argc > 0) {
    argv = new char*[argc];
    for (int i = 0 ; i < argc ; i++ ) {
      size_t res = 0;
      size_t size = wcslen(lpArgv[i]);
      size++;
      argv[i] = (char*)malloc(size);
      wcstombs_s(&res, argv[i], size, lpArgv[i], size);
    }
  }

  if (::AttachConsole(ATTACH_PARENT_PROCESS)) {
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
  }

  int res = freeserf_main(argc, argv);

  ::FreeConsole();

  if (argc > 0) {
    for (int i = 0; i < argc; i++) {
      free(argv[i]);
    }
    delete[] argv;
  }

  return res;
}

