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

#include <map>

typedef struct {
	int x;
	int y;
	int w;
	int h;
} rect_t;

class sprite_t;
class image_t;

class video_frame_t;
class gfx_t;

class frame_t
{
protected:
  video_frame_t *native_frame;
  gfx_t *gfx;

public:
  frame_t(video_frame_t *native_frame, gfx_t *gfx);
  frame_t(unsigned int width, unsigned int height, gfx_t *gfx);
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

class image_cache_t
{
public:
  virtual ~image_cache_t() {}

  virtual void add_image_to_cache(unsigned int sprite, unsigned int mask, unsigned char offset, image_t *image) = 0;
  virtual image_t *get_image_from_cache(unsigned int sprite, unsigned int mask, unsigned char offset) = 0;
};

class video_t;

class gfx_t
  : public image_cache_t
{
protected:
  static video_t *video;
  static gfx_t *gfx;

  typedef std::map<uint64_t, image_t *> image_map_t;
  image_map_t image_cache;

public:
  gfx_t(unsigned int width, unsigned int height, bool fullscreen);
  virtual ~gfx_t();

  static gfx_t *get_gfx();

  video_t *get_video() { return video; }

  /* Frame functions */
  frame_t *create_frame(unsigned int width, unsigned int height);

  /* Screen functions */
  frame_t *get_screen_frame();
  void set_resolution(unsigned int width, unsigned int height, bool fullscreen);
  void get_resolution(unsigned int &width, unsigned int &height);
  bool set_fullscreen(int enable);
  bool is_fullscreen();
  bool is_fullscreen_possible();

  void swap_buffers();

  void warp_mouse(int x, int y);

  virtual void add_image_to_cache(unsigned int sprite, unsigned int mask, unsigned char offset, image_t *image);
  virtual image_t *get_image_from_cache(unsigned int sprite, unsigned int mask, unsigned char offset);

protected:
  uint64_t gfx_image_id(unsigned int sprite, unsigned int mask, unsigned char offset);
};

#endif /* ! _GFX_H */
