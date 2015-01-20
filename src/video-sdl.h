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

#include <SDL.h>

class sdl_frame_t
  : public video_frame_t
{
protected:
  unsigned int width;
  unsigned int height;
  SDL_Texture *texture;

public:
  sdl_frame_t(unsigned int width, unsigned int height);
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

public:
  sdl_image_t(sprite_t *sprite);
  virtual ~sdl_image_t();

  SDL_Texture *get_texture() { return texture; }
};

#endif /* ! _VIDEO_SDL_H */
