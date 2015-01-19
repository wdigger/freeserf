/*
 * video.h - Graphics rendering functions
 *
 * Copyright (C) 2012-2014  Jon Lund Steffensen <jonlst@gmail.com>
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

#ifndef _VIDEO_H
#define _VIDEO_H

#include "gfx.h"

class color_t;

class image_t
{
protected:
  int delta_x;
  int delta_y;
  int offset_x;
  int offset_y;
  unsigned int width;
  unsigned int height;

public:
  image_t(sprite_t *sprite);
  virtual ~image_t() {}

  unsigned int get_width() { return width; }
  unsigned int get_height() { return height; }
  int get_offset_x() { return offset_x; }
  int get_offset_y() { return offset_y; }
};

class video_frame_t
{
public:
  virtual ~video_frame_t() {}

  virtual unsigned int get_width() = 0;
  virtual unsigned int get_height() = 0;

  virtual void draw(int dx, int dy, video_frame_t *dest, int sx, int sy, int w, int h) = 0;
  virtual void draw(image_t *image, int x, int y, int y_offset) = 0;
  virtual void fill_rect(int x, int y, int width, int height, const color_t *color) = 0;
};

int video_init();
void video_deinit();
int video_set_resolution(int width, int height, int fullscreen);
void video_get_resolution(int *width, int *height);
int video_set_fullscreen(int enable);
int video_is_fullscreen();
int video_is_fullscreen_possible();

video_frame_t *video_get_screen_frame();
video_frame_t *video_frame_create(unsigned int width, unsigned int height);

void video_warp_mouse(int x, int y);

void video_swap_buffers();

void video_set_cursor(const sprite_t *sprite);

image_t *video_image_from_sprite(sprite_t *sprite);

#endif /* ! _VIDEO_H */
