/*
 * gfx.h - General graphics and data file functions
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

#ifndef _GFX_H
#define _GFX_H

typedef struct {
	int x;
	int y;
	int w;
	int h;
} rect_t;

class sprite_t;
class image_t;

class video_frame_t;

class frame_t
{
protected:
  video_frame_t *native_frame;

public:
  frame_t(video_frame_t *native_frame);
  frame_t(unsigned int width, unsigned int height);
  virtual ~frame_t();

  /* Sprite functions */
  void draw_sprite(int x, int y, unsigned int sprite);
  void draw_transp_sprite(int x, int y, unsigned int sprite, bool use_off);
  void draw_transp_sprite(int x, int y, unsigned int sprite, bool use_off, float progress);
  void draw_transp_sprite(int x, int y, unsigned int sprite, bool use_off, unsigned char color_offs);
  void draw_transp_sprite_relatively(int x, int y, unsigned int sprite, unsigned int offs_sprite);
  void draw_masked_sprite(int x, int y, unsigned int mask, unsigned int sprite);
  void draw_overlay_sprite(int x, int y, unsigned int sprite);
  void draw_overlay_sprite(int x, int y, unsigned int sprite, float progress);
  void draw_waves_sprite(int x, int y, unsigned int mask, unsigned int sprite);

  /* Drawing functions */
  void fill_rect(int x, int y, int width, int height, unsigned char color);
  void draw_rect(int x, int y, int width, int height, unsigned char color);
  void draw_frame(int dx, int dy, int sx, int sy, frame_t *src, int w, int h);

  /* Text functions */
  void draw_string(int x, int y, int color, int shadow, const char *str);
  void draw_number(int x, int y, int color, int shadow, int n);

protected:
  void draw_char_sprite(int x, int y, unsigned int c, int color, int shadow);
};

bool gfx_init(int width, int height, int fullscreen);
void gfx_deinit();

/* Frame functions */
frame_t *gfx_frame_create(unsigned int width, unsigned int height);

/* Screen functions */
frame_t *gfx_get_screen_frame();
void gfx_set_resolution(unsigned int width, unsigned int height, bool fullscreen);
void gfx_get_resolution(int *width, int *height);
int gfx_set_fullscreen(int enable);
int gfx_is_fullscreen();
int gfx_is_fullscreen_possible();

/* Image caching functions */
void gfx_add_image_to_cache(int sprite, int mask, int offset, image_t *image);
image_t *gfx_get_image_from_cache(int sprite, int mask, int offset);
void gfx_clear_cache();

void gfx_swap_buffers();

void gfx_warp_mouse(int x, int y);

#endif /* ! _GFX_H */
