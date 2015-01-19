/*
 * gfx.cc - General graphics and data file functions
 *
 * Copyright (C) 2013-2014  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "gfx.h"
#include "video.h"
#include "data.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
#ifndef _MSC_VER
}
#endif

#include <cstdlib>

bool
gfx_init(int width, int height, int fullscreen)
{
  LOGI("graphics", "Init...");

  int r = video_init();
  if (r < 0) {
    return false;
  }

  LOGI("graphics", "Setting resolution to %ix%i...", width, height);

  video_get_resolution(&width, &height);
  r = video_set_resolution(width, height, fullscreen);
  if (r < 0) {
    return false;
  }

  sprite_t *sprite = data_get_cursor();
  video_set_cursor(sprite);
  data_sprite_free(sprite);

  return true;
}

void
gfx_deinit()
{
  gfx_clear_cache();
  video_deinit();
}

/* Draw the opaque sprite with data file index of
   sprite at x, y in dest frame. */
void
frame_t::draw_sprite(int x, int y, unsigned int sprite)
{
  image_t *image = gfx_get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_sprite_for_index(sprite);
    if (spr == NULL) {
      return;
    }
    image = video_image_from_sprite(spr);
    data_sprite_free(spr);
    gfx_add_image_to_cache(sprite, 0, 0, image);
  }

  x += image->get_offset_x();
  y += image->get_offset_y();

  native_frame->draw(image, x, y, 0);
}

/* Draw the transparent sprite with data file index of
   sprite at x, y in dest frame.*/
void
frame_t::draw_transp_sprite(int x, int y, unsigned int sprite, bool use_off)
{
  image_t *image = gfx_get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_transparent_sprite_for_index(sprite, 0);
    if (spr == NULL) {
      return;
    }
    image = video_image_from_sprite(spr);
    data_sprite_free(spr);
    gfx_add_image_to_cache(sprite, 0, 0, image);
  }

  if (use_off) {
    x += image->get_offset_x();
    y += image->get_offset_y();
  }

  native_frame->draw(image, x, y, 0);
}

void
frame_t::draw_transp_sprite(int x, int y, unsigned int sprite, bool use_off, float progress)
{
  image_t *image = gfx_get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_transparent_sprite_for_index(sprite, 0);
    if (spr == NULL) {
      return;
    }
    image = video_image_from_sprite(spr);
    data_sprite_free(spr);
    gfx_add_image_to_cache(sprite, 0, 0, image);
  }

  if (use_off) {
    x += image->get_offset_x();
    y += image->get_offset_y();
  }
  int y_off = image->get_height() - (int)(image->get_height() * progress);

  native_frame->draw(image, x, y, y_off);
}

void
frame_t::draw_transp_sprite(int x, int y, unsigned int sprite, bool use_off, unsigned char color_offs)
{
  image_t *image = gfx_get_image_from_cache(sprite, 0, color_offs);
  if (image == NULL) {
    sprite_t *spr = data_transparent_sprite_for_index(sprite, color_offs);
    if (spr == NULL) {
      return;
    }
    image = video_image_from_sprite(spr);
    data_sprite_free(spr);
    gfx_add_image_to_cache(sprite, 0, color_offs, image);
  }

  if (use_off) {
    x += image->get_offset_x();
    y += image->get_offset_y();
  }

  native_frame->draw(image, x, y, 0);
}

void
frame_t::draw_transp_sprite_relatively(int x, int y, unsigned int sprite, unsigned int offs_sprite)
{
  image_t *image = gfx_get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_transparent_sprite_for_index(sprite, 0);
    if (spr == NULL) {
      return;
    }
    image = video_image_from_sprite(spr);
    data_sprite_free(spr);
    gfx_add_image_to_cache(sprite, 0, 0, image);
  }

  x += image->get_offset_x();
  y += image->get_offset_y();

  int dx, dy;
  data_get_sprite_offset(offs_sprite, &dx, &dy);
  x += dx;
  y += dy;

  native_frame->draw(image, x, y, 0);
}

/* Draw the masked sprite with given mask and sprite
   indices at x, y in dest frame. */
void
frame_t::draw_masked_sprite(int x, int y, unsigned int mask, unsigned int sprite)
{
  image_t *image = gfx_get_image_from_cache(sprite, mask, 0);
  if (image == NULL) {
    sprite_t *spr = data_sprite_for_index(sprite);
    if (NULL == spr) {
      return;
    }
    sprite_t *msk = data_mask_sprite_for_index(mask);
    if (msk == NULL) {
      delete spr;
      return;
    }

    sprite_t *masked = data_apply_mask(spr, msk);
    data_sprite_free(spr);
    data_sprite_free(msk);
    image = video_image_from_sprite(masked);
    data_sprite_free(masked);
    gfx_add_image_to_cache(sprite, mask, 0, image);
  }

  x += image->get_offset_x();
  y += image->get_offset_y();

  native_frame->draw(image, x, y, 0);
}

/* Draw the overlay sprite with data file index of
   sprite at x, y in dest frame. Rendering will be
   offset in the vertical axis from y_off in the
   sprite. */
void
frame_t::draw_overlay_sprite(int x, int y, unsigned int sprite)
{
  image_t *image = gfx_get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_overlay_sprite_for_index(sprite);
    if (spr == NULL) {
      return;
    }
    image = video_image_from_sprite(spr);
    data_sprite_free(spr);
    gfx_add_image_to_cache(sprite, 0, 0, image);
  }

  x += image->get_offset_x();
  y += image->get_offset_y();

  native_frame->draw(image, x, y, 0);
}

void
frame_t::draw_overlay_sprite(int x, int y, unsigned int sprite, float progress)
{
  image_t *image = gfx_get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_overlay_sprite_for_index(sprite);
    if (spr == NULL) {
      return;
    }
    image = video_image_from_sprite(spr);
    data_sprite_free(spr);
    gfx_add_image_to_cache(sprite, 0, 0, image);
  }

  x += image->get_offset_x();
  y += image->get_offset_y();
  int y_off = image->get_height() - (int)(image->get_height() * progress);

  native_frame->draw(image, x, y, y_off);
}

/* Draw the waves sprite with given mask and sprite
   indices at x, y in dest frame. */
void
frame_t::draw_waves_sprite(int x, int y, unsigned int mask, unsigned int sprite)
{
  if (mask == 0) {
    draw_transp_sprite(x, y, sprite, false);
    return;
  }

  image_t *image = gfx_get_image_from_cache(sprite, mask, 0);
  if (image == NULL) {
    sprite_t *spr = data_transparent_sprite_for_index(sprite, 0);
    if (spr == NULL) {
      return;
    }
    sprite_t *msk = data_mask_sprite_for_index(mask);
    if (msk == NULL) {
      delete spr;
      return;
    }

    sprite_t *masked = data_apply_mask(spr, msk);
    data_sprite_free(spr);
    data_sprite_free(msk);
    image = video_image_from_sprite(masked);
    data_sprite_free(masked);
    gfx_add_image_to_cache(sprite, mask, 0, image);
  }

  x += image->get_offset_x();
  y += image->get_offset_y();

  native_frame->draw(image, x, y, 0);
}


/* Draw a character at x, y in the dest frame. */
void
frame_t::draw_char_sprite(int x, int y, unsigned int c, int color, int shadow)
{
  static const int sprite_offset_from_ascii[] = {

    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, 43, -1, -1,
    -1, -1, -1, -1, -1, 40, 39, -1,
    29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 41, -1, -1, -1, -1, 42,
    -1,  0,  1,  2,  3,  4,  5,  6,
     7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,
     7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, -1, -1, -1, -1, -1,

    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
  };

  int s = sprite_offset_from_ascii[c];
  if (s < 0) return;

  if (shadow) {
    draw_transp_sprite(x, y, DATA_FONT_SHADOW_BASE + s, false, (unsigned char)shadow);
  }
  draw_transp_sprite(x, y, DATA_FONT_BASE + s, false, (unsigned char)color);
}

/* Draw the string str at x, y in the dest frame. */
void
frame_t::draw_string(int x, int y, int color, int shadow, const char *str)
{
  for (; *str != 0; x += 8) {
    if (/*string_bg*/ 0) {
      color_t color = data_get_color(0);
      native_frame->fill_rect(x, y, 8, 8, &color);
    }

    unsigned char c = *str++;
    draw_char_sprite(x, y, c, color, shadow);
  }
}

/* Draw the number n at x, y in the dest frame. */
void
frame_t::draw_number(int x, int y, int color, int shadow, int n)
{
  if (n < 0) {
    draw_char_sprite(x, y, '-', color, shadow);
    x += 8;
    n *= -1;
  }

  if (n == 0) {
    draw_char_sprite(x, y, '0', color, shadow);
    return;
  }

  int digits = 0;
  for (int i = n; i > 0; i /= 10) digits += 1;

  for (int i = digits-1; i >= 0; i--) {
    draw_char_sprite(x+8*i, y, '0'+(n % 10), color, shadow);
    n /= 10;
  }
}

/* Draw a rectangle with color at x, y in the dest frame. */
void
frame_t::draw_rect(int x, int y, int width, int height, unsigned char color)
{
  color_t col = data_get_color(color);
  native_frame->fill_rect(x, y, width, 1, &col);
  native_frame->fill_rect(x, y+height-1, width, 1, &col);
  native_frame->fill_rect(x, y, 1, height, &col);
  native_frame->fill_rect(x+width-1, y, 1, height, &col);
}

/* Fill a rectangle with color at x, y in the dest frame. */
void
frame_t::fill_rect(int x, int y, int width, int height, unsigned char color)
{
  color_t clr = data_get_color(color);
  native_frame->fill_rect(x, y, width, height, &clr);
}


/* Initialize new graphics frame. If dest is NULL a new
   backing surface is created, otherwise the same surface
   as dest is used. */
frame_t *
gfx_frame_create(unsigned int width, unsigned int height)
{
  return new frame_t(width, height);
}

frame_t::frame_t(video_frame_t *native_frame)
{
  this->native_frame = native_frame;
}

frame_t::frame_t(unsigned int width, unsigned int height)
{
  native_frame = video_frame_create(width, height);
}

frame_t::~frame_t()
{
  if (native_frame != NULL) {
    delete native_frame;
    native_frame = NULL;
  }
}

/* Draw source frame from rectangle at sx, sy with given
   width and height, to destination frame at dx, dy. */
void
frame_t::draw_frame(int dx, int dy, int sx, int sy, frame_t *src, int w, int h)
{
  src->native_frame->draw(dx, dy, native_frame, sx, sy, w, h);
}

frame_t *
gfx_get_screen_frame()
{
  return new frame_t(video_get_screen_frame());
}

/* Enable or disable fullscreen mode */
int
gfx_set_fullscreen(int enable)
{
  return video_set_fullscreen(enable);
}

/* Check whether fullscreen mode is enabled */
int
gfx_is_fullscreen()
{
  return video_is_fullscreen();
}

int
gfx_is_fullscreen_possible()
{
  return video_is_fullscreen_possible();
}

void
gfx_get_resolution(int *width, int *height)
{
  video_get_resolution(width, height);
}
