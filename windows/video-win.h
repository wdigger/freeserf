/*
 * video-win.h - Video rendering functions
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

#ifndef _VIDEO_WIN_H
#define _VIDEO_WIN_H

#include "video.h"

#include <Windows.h>
#include <gdiplus.h>

class video_win_t;

class frame_win_t
  : public video_frame_t
{
protected:
  unsigned int width;
  unsigned int height;
  Gdiplus::Bitmap *texture;
  video_win_t *video;

public:
  frame_win_t(unsigned int width, unsigned int height, video_win_t *video);
  virtual ~frame_win_t();

  virtual unsigned int get_width() { return width; }
  virtual unsigned int get_height() { return height; }

  virtual void draw(int dx, int dy, video_frame_t *dest, int sx, int sy, int sw, int sh);
  virtual void draw(image_t *image, int x, int y, int y_offset);
  virtual void fill_rect(int x, int y, int width, int height, const color_t *color);

  Gdiplus::Bitmap *get_texture() { return texture; }

  void set_size(unsigned int width, unsigned int height);
};

class image_win_t
  : public image_t
{
protected:
  Gdiplus::Bitmap *texture;
  video_win_t *video;
  void *data;

public:
  image_win_t(sprite_t *sprite, video_win_t *video);
  virtual ~image_win_t();

  Gdiplus::Bitmap *get_texture() { return texture; }

  Gdiplus::Bitmap *create_surface_from_sprite(const sprite_t *sprite);
};

class video_win_t
  : public video_t
{
protected:
  HWND window;
  ULONG_PTR gdiplusToken;

  frame_win_t *screen;
  bool fullscreen;

public:
  video_win_t();
  virtual ~video_win_t();

  virtual bool set_resolution(unsigned int width, unsigned int height, bool fullscreen);
  virtual void get_resolution(unsigned int &width, unsigned int &height);

  virtual bool set_fullscreen(bool enable);
  virtual bool is_fullscreen();
  virtual bool is_fullscreen_possible();

  virtual video_frame_t *get_screen_frame();
  virtual video_frame_t *frame_create(unsigned int width, unsigned int height);

  virtual void swap_buffers();

  virtual void set_cursor(const sprite_t *sprite);

  virtual image_t *image_from_sprite(sprite_t *sprite);

  HWND get_window() { return window; }
};

#endif /* ! _VIDEO_WIN_H */
