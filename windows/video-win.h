/*
 * video-win.h - Video rendering functions
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

#ifndef WINDOWS_VIDEO_WIN_H_
#define WINDOWS_VIDEO_WIN_H_

#include <Windows.h>
#include <gdiplus.h>

#include "src/video.h"

class Video::Frame {
 public:
  Gdiplus::Bitmap *texture;

  Video::Frame() : texture(NULL) {}
};

class Video::Image {
 public:
  BYTE *data;
  Gdiplus::Bitmap *texture;

  Video::Image() : texture(NULL), data(NULL) {}
};

class VideoWin : public Video {
 protected:
  HWND window;
  ULONG_PTR gdiplusToken;

  Video::Frame *screen;
  bool fullscreen;

  HCURSOR cursor;

 public:
  VideoWin();
  virtual ~VideoWin();

  virtual void set_resolution(unsigned int width, unsigned int height,
                              bool fullscreen);
  virtual void get_resolution(unsigned int *width, unsigned int *height);
  virtual void set_fullscreen(bool enable);
  virtual bool is_fullscreen();

  virtual Video::Frame *get_screen_frame();
  virtual Video::Frame *create_frame(unsigned int width, unsigned int height);
  virtual void destroy_frame(Video::Frame *frame);

  virtual Video::Image *create_image(void *data, unsigned int width,
                                      unsigned int height);
  virtual void destroy_image(Video::Image *image);

  virtual void warp_mouse(int x, int y);

  virtual void draw_image(const Video::Image *image, int x, int y,
                          int y_offset, Video::Frame *dest);
  virtual void draw_frame(int dx, int dy, Video::Frame *dest, int sx, int sy,
                          Video::Frame *src, int w, int h);
  virtual void draw_rect(int x, int y, unsigned int width, unsigned int height,
                         const Video::Color color, Video::Frame *dest);
  virtual void fill_rect(int x, int y, unsigned int width, unsigned int height,
                         const Video::Color color, Video::Frame *dest);
  virtual void draw_line(int x, int y, int x1, int y1,
                         const Video::Color color, Frame *dest);
  virtual void swap_buffers();

  virtual void set_cursor(void *data, unsigned int width, unsigned int height);

  virtual float get_zoom_factor() { return 1.f; }
  virtual bool set_zoom_factor(float factor) { return true; }
  virtual void get_screen_factor(float *fx, float *fy);

  HWND get_window() { return window; }
  HCURSOR get_cursor() { return cursor; }
};

#endif  // WINDOWS_VIDEO_WIN_H_
