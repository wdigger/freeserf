/*
 * video-win.cc - Video graphics rendering
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

#include "windows/video-win.h"

#include "src/data.h"
#include "src/log.h"

LRESULT CALLBACK
TmpWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

VideoWin::VideoWin()
  : screen(nullptr)
  , window(nullptr)
  , gdiplusToken(0)
  , cursor(nullptr)
  , fullscreen(false) {
  /* Initialize Video subsystem */
  Gdiplus::GdiplusStartupInput gdiplusStartupInput = 0;
  Gdiplus::GdiplusStartupOutput gdiplusStartupOutput = { 0 };
  if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput,
                              &gdiplusStartupOutput) != Gdiplus::Ok) {
    Log::Error["video-win"] << "Unable to initialize Gdiplus.";
    return;
  }

  /* Create window and renderer */
  const char *className = "FreeSerf_window_class";
  WNDCLASSEXA wcex = { sizeof(WNDCLASSEX), CS_OWNDC,
    &TmpWindowProc,
    0, 0,
    GetModuleHandle(nullptr),
    0,
    LoadCursor(nullptr, IDC_ARROW),
    0, 0,
    "FreeSerf_window_class",
    0
  };

  if (!RegisterClassExA(&wcex)) {
    return;
  }

  DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SIZEBOX;
  DWORD windowExtendedStyle = 0;
  window = ::CreateWindowExA(windowExtendedStyle,
    className,
    "FreeSerf",
    windowStyle,
    0, 0,
    800, 600,
    0, 0, GetModuleHandle(nullptr), nullptr);
}

VideoWin::~VideoWin() {
  set_cursor(nullptr, 0, 0);

  Gdiplus::GdiplusShutdown(gdiplusToken);
  gdiplusToken = 0;

  if (screen != nullptr) {
    delete screen;
    screen = nullptr;
  }
}

Video&
Video::get_instance() {
	static VideoWin instance;
	return instance;
}

void
VideoWin::set_resolution(unsigned int width, unsigned int height,
                         bool fullscreen) {
  RECT sr = {0};
  ::GetWindowRect(window, &sr);
  RECT cr = {0};
  ::GetClientRect(window, &cr);
  ::SetWindowPos(window, nullptr, 0, 0,
                 width + ((sr.right-sr.left) - (cr.right - cr.left)),
                 height + ((sr.bottom - sr.top) - (cr.bottom - cr.top)),
                 SWP_NOMOVE | SWP_NOZORDER);
}

void
VideoWin::get_resolution(unsigned int *width, unsigned int *height) {
  RECT rect;
  ::GetClientRect(window, &rect);
  if (width != nullptr) {
    *width = rect.right - rect.left;
  }
  if (height != nullptr) {
    *height = rect.bottom - rect.top;
  }
}

void
VideoWin::set_fullscreen(bool enable) {
/*
  int width = 0;
  int height = 0;
  SDL_GL_GetDrawableSize(window, &width, &height);
  return set_resolution(width, height, enable);
*/
}

bool
VideoWin::is_fullscreen() {
  return false;
}

Video::Frame *
VideoWin::get_screen_frame() {
  if (screen == nullptr) {
    unsigned int width = 0;
    unsigned int height = 0;
    get_resolution(&width, &height);
    screen = create_frame(width, height);
  }

  return screen;
}

Video::Frame *
VideoWin::create_frame(unsigned int width, unsigned int height) {
  Video::Frame *frame = new Video::Frame();
  frame->texture = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
  return frame;
}

void
VideoWin::destroy_frame(Video::Frame *frame) {
  delete frame->texture;
  delete frame;
}

Video::Image *
VideoWin::create_image(void *data, unsigned int width, unsigned int height) {
  Video::Image *image = new Video::Image();
  image->data = new BYTE[height * width * 4];
  memcpy(image->data, data, height * width * 4);
  image->texture = new Gdiplus::Bitmap(width, height, width * 4,
                                       PixelFormat32bppARGB, image->data);
  if (image->texture->GetLastStatus() != Gdiplus::Ok) {
    Log::Error["video_win"] << "Unable to create Gdiplus bitmap from fprite.";
    delete image->texture;
    image->texture = nullptr;
    delete[] image->data;
    image->data = nullptr;
  }
  return image;
}

void
VideoWin::destroy_image(Video::Image *image) {
  delete image->texture;
  image->texture = nullptr;
  delete[] image->data;
  image->data = nullptr;
  delete image;
}

void
VideoWin::warp_mouse(int x, int y) {
}

void
VideoWin::draw_image(const Video::Image *image, int x, int y, int y_offset,
                     Video::Frame *dest) {
  Gdiplus::Graphics *graphics = new Gdiplus::Graphics(dest->texture);
  UINT width = image->texture->GetWidth();
  UINT height = image->texture->GetHeight();
  if (graphics->DrawImage(image->texture, x, y + y_offset,
                          0, y_offset, width, height - y_offset,
                          Gdiplus::UnitPixel) != Gdiplus::Ok) {
    Log::Error["video_win"] << "DrawImage failed.";
  }
  delete graphics;
}

void
VideoWin::draw_frame(int dx, int dy, Video::Frame *dest, int sx, int sy,
                     Video::Frame *src, int w, int h) {
  Gdiplus::Graphics *graphics = new Gdiplus::Graphics(dest->texture);
  if (graphics->DrawImage(src->texture, dx, dy, sx, sy, w, h,
                          Gdiplus::UnitPixel) != Gdiplus::Ok) {
    Log::Error["video_win"] << "DrawImage failed.";
  }
  delete graphics;
}

void
VideoWin::draw_rect(int x, int y, unsigned int width, unsigned int height,
                    const Video::Color color, Video::Frame *dest) {
  /* Fill rectangle */
  Gdiplus::Graphics graphics(dest->texture);
  Gdiplus::Pen pen(Gdiplus::Color(color.a, color.r, color.g, color.b));
  if (graphics.DrawRectangle(&pen, x, y, width, height) != Gdiplus::Ok) {
    Log::Error["video_win"] << "FillRectangle failed.";
  }
}

void
VideoWin::fill_rect(int x, int y, unsigned int width, unsigned int height,
                    const Video::Color color, Video::Frame *dest) {
  /* Fill rectangle */
  Gdiplus::Graphics graphics(dest->texture);
  Gdiplus::SolidBrush brush(Gdiplus::Color(color.a, color.r, color.g, color.b));
  if (graphics.FillRectangle(&brush, x, y, width, height) != Gdiplus::Ok) {
    Log::Error["video_win"] << "FillRectangle failed.";
  }
}

void
VideoWin::draw_line(int x, int y, int x1, int y1,
                    const Video::Color color, Frame *dest) {
  Gdiplus::Graphics graphics(dest->texture);
  Gdiplus::Pen pen(Gdiplus::Color(color.a, color.r, color.g, color.b));
  if (graphics.DrawLine(&pen, x, y, x1, y1) != Gdiplus::Ok) {
    Log::Error["video_win"] << "DrawLine failed.";
  }
}

void
VideoWin::swap_buffers() {
  if (screen == nullptr) {
    get_screen_frame();
  }

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(window, &ps);
  Gdiplus::Graphics *graphics = new Gdiplus::Graphics(hdc);
//  RECT cr = { 0 };
//  ::GetClientRect(window, &cr);
  graphics->DrawImage(screen->texture, 0, 0);
  delete graphics;
  EndPaint(window, &ps);
}

void
VideoWin::set_cursor(void *data, unsigned int width, unsigned int height) {
  if (cursor != nullptr) {
    ::SetCursor(nullptr);
    ::DestroyCursor(cursor);
    cursor = nullptr;
  }

  if (data == nullptr) {
    return;
  }

  Video::Image *image = create_image(data, width, height);
  HICON icon = nullptr;
  image->texture->GetHICON(&icon);
  cursor = icon;
  destroy_image(image);
  ::SetCursor(cursor);
}

void
VideoWin::get_screen_factor(float *fx, float *fy) {
  if (fx != nullptr) {
    *fx = 1.f;
  }

  if (fy != nullptr) {
    *fy = 1.f;
  }
}
