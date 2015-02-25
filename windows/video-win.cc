/*
 * video-win.cc - Video graphics rendering
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

#include "video-win.h"
#include "data.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
#ifndef _MSC_VER
}
#endif

video_t *
create_video()
{
  return new video_win_t();
}

LRESULT CALLBACK
TmpWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

video_win_t::video_win_t()
{
  screen = NULL;
  window = NULL;
  gdiplusToken = 0;

  /* Initialize Video subsystem */
  Gdiplus::GdiplusStartupInput gdiplusStartupInput = 0;
  Gdiplus::GdiplusStartupOutput gdiplusStartupOutput = { 0 };
  if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput) != Gdiplus::Ok) {
    LOGE("video-win", "Unable to initialize Gdiplus.");
    return;
  }

  /* Create window and renderer */
  GUID guid = { 0 };
  ::CoCreateGuid(&guid);
  LPTSTR className = new WCHAR[39];
  StringFromGUID2(guid, className, 39);

  WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_OWNDC,
    &TmpWindowProc,
    0, 0,
    GetModuleHandle(NULL),
    0,
    LoadCursor(NULL, IDC_ARROW),
    0, 0,
    className,
    0
  };

  if (!RegisterClassEx(&wcex)) {
    return;
  }

  DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SIZEBOX;
  DWORD windowExtendedStyle = 0;
  window = ::CreateWindowEx(windowExtendedStyle,
    className,
    TEXT("FreeSerf"),
    windowStyle,
    0, 0,
    800, 600,
    0, 0, GetModuleHandle(NULL), NULL);
}

video_win_t::~video_win_t()
{
  set_cursor(NULL);

  Gdiplus::GdiplusShutdown(gdiplusToken);
  gdiplusToken = 0;

  if (screen != NULL) {
    delete screen;
    screen = NULL;
  }
}

bool
video_win_t::set_resolution(unsigned int width, unsigned int height, bool fullscreen)
{
  ::SetWindowPos(window, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
  return true;
}

void
video_win_t::get_resolution(unsigned int &width, unsigned int &height)
{
  RECT rect;
  ::GetClientRect(window, &rect);
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;
}

bool
video_win_t::is_fullscreen_possible()
{
  return true;
}

bool
video_win_t::set_fullscreen(bool enable)
{
/*
  int width = 0;
  int height = 0;
  SDL_GL_GetDrawableSize(window, &width, &height);
  return set_resolution(width, height, enable);
*/
  return false;
}

bool
video_win_t::is_fullscreen()
{
  return false;
}

video_frame_t *
video_win_t::get_screen_frame()
{
  if (screen == NULL) {
    unsigned int width = 0;
    unsigned int height = 0;
    get_resolution(width, height);
    screen = new frame_win_t(width, height, this);
  }

  return screen;
}

frame_win_t::frame_win_t(unsigned int width, unsigned int height, video_win_t *video)
{
  texture = NULL;
  this->video = video;

  set_size(width, height);
}

frame_win_t::~frame_win_t()
{
  if (texture != NULL) {
    delete texture;
    texture = NULL;
  }
}

void
frame_win_t::set_size(unsigned int width, unsigned int height)
{
  if (texture != NULL) {
    delete texture;
    texture = NULL;
  }

  this->width = width;
  this->height = height;

  texture = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);

  if (texture->GetLastStatus() != Gdiplus::Ok) {
    LOGE("video_win", "Unable to create Gdiplus bitmap.");
    delete texture;
    texture = 0;
  }
}

video_frame_t *
video_win_t::frame_create(unsigned int width, unsigned int height)
{
  return new frame_win_t(width, height, this);
}

Gdiplus::Bitmap *
image_win_t::create_surface_from_sprite(const sprite_t *sprite)
{
  unsigned int size = sprite->width * 4 * sprite->height;
  BYTE *data = (BYTE*)malloc(size);
  memcpy(data, sprite->data, size);
  Gdiplus::Bitmap *bitmap = new Gdiplus::Bitmap(sprite->width,
    sprite->height, sprite->width * 4, PixelFormat32bppARGB, data);

  if (bitmap->GetLastStatus() != Gdiplus::Ok) {
    LOGE("video_win", "Unable to create Gdiplus bitmap from fprite.");
    delete bitmap;
    bitmap = NULL;
  }

  return bitmap;
}

image_win_t::image_win_t(sprite_t *sprite, video_win_t *video)
  : image_t(sprite)
{
  this->video = video;
  texture = create_surface_from_sprite(sprite);
}

image_win_t::~image_win_t()
{
  if (texture != NULL) {
    delete texture;
    texture = NULL;
  }
}

image_t *
video_win_t::image_from_sprite(sprite_t *sprite)
{
  return new image_win_t(sprite, this);
}

void
frame_win_t::draw(image_t *image, int x, int y, int y_offset)
{
  image_win_t *native_image = (image_win_t*)image;
  Gdiplus::Graphics *graphics = new Gdiplus::Graphics(texture);
  UINT width = image->get_width();
  UINT height = image->get_height();
  if (graphics->DrawImage(native_image->get_texture(),
    x, y + y_offset,
    0, y_offset,
    width, height - y_offset,
    Gdiplus::UnitPixel) != Gdiplus::Ok)
  {
    LOGE("video_win", "DrawImage failed.");
  }
  delete graphics;
}

void
frame_win_t::draw(int dx, int dy, video_frame_t *dest, int sx, int sy, int w, int h)
{
  frame_win_t *native_dest_frame = (frame_win_t*)dest;
  Gdiplus::Graphics *graphics = new Gdiplus::Graphics(native_dest_frame->get_texture());
  if (graphics->DrawImage(texture, dx, dy, sx, sy, w, h, Gdiplus::UnitPixel) != Gdiplus::Ok) {
    LOGE("video_win", "DrawImage failed.");
  }
  delete graphics;
}

void
frame_win_t::fill_rect(int x, int y, int width, int height, const color_t *color)
{
  /* Fill rectangle */
  Gdiplus::Graphics *graphics = new Gdiplus::Graphics(texture);
  Gdiplus::Brush *brush = new Gdiplus::SolidBrush(Gdiplus::Color(color->a, color->r, color->g, color->b));
  if (graphics->FillRectangle(brush, x, y, width, height) != Gdiplus::Ok) {
    LOGE("video_win", "FillRectangle failed.");
  }
}

void
video_win_t::swap_buffers()
{
  if (screen == NULL) {
    get_screen_frame();
  }

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(window, &ps);
  Gdiplus::Graphics *graphics = new Gdiplus::Graphics(hdc);
//  RECT cr = { 0 };
//  ::GetClientRect(window, &cr);
  graphics->DrawImage(screen->get_texture(), 0, 0);
  delete graphics;
  EndPaint(window, &ps);
}

void
video_win_t::set_cursor(const sprite_t *sprite)
{
/*
  if (cursor != NULL) {
    ::SetCursor(NULL);
    SDL_FreeCursor(cursor);
    cursor = NULL;
  }

  if (sprite == NULL) return;

  Gdiplus::Bitmap *bitmap = image_win_t::create_surface_from_sprite(sprite);
  bitmap->GetHBITMAP()
  cursor = SDL_CreateColorCursor(surface, 8, 8);
  SDL_SetCursor(cursor);
  SDL_FreeSurface(surface);
*/
}
