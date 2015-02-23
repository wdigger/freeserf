/*
 * video-sdl.h - SDL rendering functions
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

#ifndef _VIDEO_SDL_H
#define _VIDEO_SDL_H

#include "video.h"

#include "SDL.h"

class sdl_video_t;

class sdl_frame_t
  : public video_frame_t
{
protected:
  unsigned int width;
  unsigned int height;
  SDL_Texture *texture;
  sdl_video_t *video;

public:
  sdl_frame_t(unsigned int width, unsigned int height, sdl_video_t *video);
  virtual ~sdl_frame_t();

  virtual unsigned int get_width() { return width; }
  virtual unsigned int get_height() { return height; }

  virtual void draw(int dx, int dy, video_frame_t *dest, int sx, int sy, int sw, int sh);
  virtual void draw(image_t *image, int x, int y, int y_offset);
  virtual void fill_rect(int x, int y, int width, int height, const color_t *color);

  SDL_Texture *get_texture() { return texture; }

  void set_size(unsigned int width, unsigned int height);
};

class sdl_image_t
  : public image_t
{
protected:
  SDL_Texture *texture;
  sdl_video_t *video;

public:
  sdl_image_t(sprite_t *sprite, sdl_video_t *video);
  virtual ~sdl_image_t();

  SDL_Texture *get_texture() { return texture; }

  static SDL_Surface *create_surface_from_sprite(const sprite_t *sprite, Uint32 pixel_format);
};

class sdl_video_t
  : public video_t
{
protected:
  static int bpp;
  static Uint32 Rmask;
  static Uint32 Gmask;
  static Uint32 Bmask;
  static Uint32 Amask;
  static Uint32 pixel_format;

  SDL_Window *window;
  SDL_Renderer *renderer;

  sdl_frame_t *screen;
  bool fullscreen;
  SDL_Cursor *cursor;

public:
  sdl_video_t();
  virtual ~sdl_video_t();

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

  SDL_Renderer *get_renderer() { return renderer; }
  Uint32 get_pixel_format() { return pixel_format; }
};

#endif /* ! _VIDEO_SDL_H */
