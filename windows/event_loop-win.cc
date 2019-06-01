/*
 * event_loop-win.cc - User and system events handling
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

#include "windows/event_loop-win.h"

#include <string>
#include <Shlobj.h>
#include <Windowsx.h>

#include "src/gfx.h"
#include "src/freeserf.h"
#include "src/application.h"
#include "src/log.h"
#include "windows/video-win.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

EventLoop&
EventLoop::get_instance() {
	static EventLoopWin event_loop;
	return event_loop;
}

/* How fast consequtive mouse events need to be generated
 in order to be interpreted as click and double click. */
#define MOUSE_TIME_SENSITIVITY  600
/* How much the mouse can move between events to be still
 considered as a double click. */
#define MOUSE_MOVE_SENSITIVITY  8

EventLoopWin::EventLoopWin() {
  dragging = false;
  video = reinterpret_cast<VideoWin*>(
            &Application::get_instance().get_video());
}

void
EventLoopWin::quit() {
  ::PostQuitMessage(0);
}

#define WM_DEFFERED_CALL (WM_USER + 123)

LRESULT CALLBACK
EventLoopWin::WndProc(HWND hWnd, UINT message,
                      WPARAM wParam, LPARAM lParam) {
  EventLoopWin *pthis = reinterpret_cast<EventLoopWin*>(
                              ::GetWindowLongPtr(hWnd, GWLP_USERDATA));
  return pthis->process_event(hWnd, message, wParam, lParam);
}

LRESULT
EventLoopWin::process_event(HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam) {
  switch (message) {
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
      Gdiplus::Graphics *graphics = new Gdiplus::Graphics(hdc);
      Video::Frame *video_frame = Video::get_instance().get_screen_frame();
      graphics->DrawImage(video_frame->texture, 0, 0);
      delete graphics;
      EndPaint(hWnd, &ps);
      return 0;
      break;
    }
    case WM_SIZE: {
      notify_resize(LOWORD(lParam), HIWORD(lParam));
      break;
    }
    case WM_SIZING: {
      RECT *pRect = reinterpret_cast<RECT*>(lParam);
      notify_resize(pRect->right - pRect->left, pRect->bottom - pRect->top);
      break;
    }
    case WM_KEYDOWN: {
      switch (wParam) {
      case VK_LEFT:
        notify_drag(0, 0, -32, 0, Event::ButtonLeft);
        break;
      case VK_RIGHT:
        notify_drag(0, 0, 32, 0, Event::ButtonLeft);
        break;
      case VK_UP:
        notify_drag(0, 0, 0, -32, Event::ButtonLeft);
        break;
      case VK_DOWN:
        notify_drag(0, 0, 0, 32, Event::ButtonLeft);
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
        } else {
          modifier |= 1;
        }
      }
      if (GetKeyState(VK_SHIFT)) {
        modifier |= 2;
      }

      if (key >= 'A' && key <= 'Z') {
        key = 'a' + (key - 'A');
      } else if (key == '=') {
        key = '+';
      }

      if (key == 'q' && modifier == 1) {
        quit();
        return 0;
      }

      notify_key_pressed(key, modifier);
      return 0;
    }
    case WM_RBUTTONDOWN: {
      POINT point;
      point.x = GET_X_LPARAM(lParam);
      point.y = GET_Y_LPARAM(lParam);

      notify_click(point.x, point.y, Event::ButtonRight);

      drag_start_pos_x = point.x;
      drag_start_pos_y = point.y;

      drag_pos_x = 0;
      drag_pos_y = 0;

      dragging = true;

      RAWINPUTDEVICE Rid[1];
      Rid[0].usUsagePage = 1;
      Rid[0].usUsage = 2;
      Rid[0].dwFlags = RIDEV_INPUTSINK;
      Rid[0].hwndTarget = hWnd;
      ::RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

      ::GetCursorPos(&point);
      RECT rect = {point.x-2, point.y-2, point.x+2, point.y+2};
      ::ClipCursor(&rect);
      ::SetCursor(NULL);

      return 0;
    }
    case WM_RBUTTONUP: {
      if (dragging) {
        dragging = false;
        POINT point = {drag_start_pos_x, drag_start_pos_y};
        ::ClientToScreen(hWnd, &point);
        ::SetCursorPos(point.x, point.y);

        RAWINPUTDEVICE Rid[1];
        Rid[0].usUsagePage = 1;
        Rid[0].usUsage = 2;
        Rid[0].dwFlags = RIDEV_INPUTSINK | RIDEV_REMOVE;
        Rid[0].hwndTarget = hWnd;
        ::RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
      }
      ::ClipCursor(NULL);
      ::ShowCursor(TRUE);
      ::SetCursor(video->get_cursor());
      return 0;
    }
    case WM_INPUT: {
      if (dragging) {
        RAWINPUT raw;
        UINT uiSize = sizeof(raw);
        ::GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &uiSize,
                          sizeof(RAWINPUTHEADER));
        if (raw.header.dwType == RIM_TYPEMOUSE) {
          int dx = 0;
          int dy = 0;
          if (MOUSE_MOVE_RELATIVE == (raw.data.mouse.usFlags & 0x01)) {
            drag_pos_x += raw.data.mouse.lLastX;
            drag_pos_y += raw.data.mouse.lLastY;
            dx = raw.data.mouse.lLastX;
            dy = raw.data.mouse.lLastY;
          } else {
            if ((drag_pos_x != 0) && (drag_pos_y != 0)) {
              dx = raw.data.mouse.lLastX - drag_pos_x;
              dy = raw.data.mouse.lLastY - drag_pos_y;
            }
            drag_pos_x = raw.data.mouse.lLastX;
            drag_pos_y = raw.data.mouse.lLastY;
          }

          notify_drag(drag_start_pos_x, drag_start_pos_y,
                      dx, dy,
                      Event::ButtonRight);

          return 0;
        }
      }
      break;
    }
    case WM_LBUTTONDOWN: {
      POINT point;
      point.x = GET_X_LPARAM(lParam);
      point.y = GET_Y_LPARAM(lParam);
      notify_click(point.x, point.y, Event::ButtonLeft);
      break;
    }
    case WM_SETCURSOR: {
      if (!dragging) {
        ::SetCursor(video->get_cursor());
      }
      return 1;
    }
    case WM_TIMER: {
      if (wParam == 123) {
        notify_update();
        Frame *screen = Graphics::get_instance().get_screen_frame();
        notify_draw(screen);
        RECT rect;
        delete screen;
        ::GetClientRect(hWnd, &rect);
        ::InvalidateRect(hWnd, &rect, FALSE);
      }
      return 0;
    }
    case WM_DEFFERED_CALL: {
      DeferredCall *deferred_callee = reinterpret_cast<DeferredCall*>(wParam);
      void *data = reinterpret_cast<void*>(lParam);
      if (deferred_callee != nullptr) {
        (*deferred_callee)(data);
      }
      return 0;
    }
  }

  return ::DefWindowProc(hWnd, message, wParam, lParam);
}

/* event_loop() has been turned into a SDL based loop.
 The code for one iteration of the original game_loop is
 in game_loop_iter. */
void
EventLoopWin::run() {
  HWND window = video->get_window();
  if (window == NULL) {
    return;
  }

  ::SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)this);
  ::SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

  ::SetTimer(window, 123, TICK_LENGTH, NULL);  // Game loop timer

  MSG msg = { 0 };
  while (0 != ::GetMessage(&msg, NULL, 0, 0)) {
    ::DispatchMessage(&msg);
  }
}

void
EventLoopWin::deferred_call(DeferredCall deferred_call, void *data) {
  HWND window = video->get_window();
  ::PostMessage(window, WM_DEFFERED_CALL, (WPARAM)&deferred_call, (LPARAM)data);
}

extern int freeserf_main(int argc, char *argv[]);

int __stdcall
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPSTR lpCmdLine, int nCmdShow) {
  int argc = 0;
  LPWSTR *lpArgv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

  char **argv = nullptr;
  if (argc > 0) {
    argv = new char*[argc];
    for (int i = 0 ; i < argc ; i++) {
      size_t res = 0;
      size_t size = wcslen(lpArgv[i]);
      size++;
      argv[i] = new char[size];
      wcstombs_s(&res, argv[i], size, lpArgv[i], size);
    }
  }

  if (::AttachConsole(ATTACH_PARENT_PROCESS)) {
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
  }

  std::string saved_game_folder;
  PWSTR path;
  if (::SHGetKnownFolderPath(FOLDERID_SavedGames, KF_FLAG_INIT |
                             KF_FLAG_CREATE, NULL, &path) == S_OK) {
    int len = static_cast<int>(wcslen(path));
    char *folder_path = new char[len + 1];
    ::WideCharToMultiByte(CP_ACP, 0, path, len+1, folder_path, len+1,
                          NULL, NULL);
    ::CoTaskMemFree(path);
    saved_game_folder = folder_path;
    saved_game_folder += "\\freeserf";
    delete[] folder_path;
  }

  int res = freeserf_main(argc, argv);

  ::FreeConsole();

  if (argv != nullptr) {
    for (int i = 0; i < argc; i++) {
      delete[] argv[i];
    }
    delete[] argv;
  }

  return res;
}

class TimerWin : public Timer {
 public:
  TimerWin(unsigned int _id, unsigned int _interval, Timer::Handler *_handler)
    : Timer(_id, _interval, _handler) {}

  virtual ~TimerWin() {
    stop();
  }

  virtual void run() {
    VideoWin &video =
       reinterpret_cast<VideoWin&>(Application::get_instance().get_video());
    HWND window = video.get_window();
    ::SetTimer(window, (UINT_PTR)this, interval, TimerWin::callback);
  }

  virtual void stop() {
    VideoWin &video =
       reinterpret_cast<VideoWin&>(Application::get_instance().get_video());
    HWND window = video.get_window();
    ::KillTimer(window, (UINT_PTR)this);
  }

  static void CALLBACK callback(HWND hwnd, UINT uMsg, UINT_PTR idEvent,
                                DWORD dwTime) {
    TimerWin *timer = reinterpret_cast<TimerWin*>(idEvent);
    if (timer->handler != nullptr) {
      timer->handler->on_timer_fired(timer->id);
    }
  }
};

Timer *
Timer::create(unsigned int _id, unsigned int _interval,
  Timer::Handler *_handler) {
  return new TimerWin(_id, _interval, _handler);
}

