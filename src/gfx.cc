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
#include "application.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
#ifndef _MSC_VER
}
#endif

#include <cstdlib>

video_t *gfx_t::video = NULL;
gfx_t *gfx_t::gfx = NULL;

gfx_t::gfx_t(unsigned int width, unsigned int height, bool fullscreen)
{
  LOGI("graphics", "Init...");

  video = application_t::get_application()->get_video();

  LOGI("graphics", "Setting resolution to %ix%i...", width, height);

  if (!video->set_resolution(width, height, fullscreen)) {
    return;
  }

  sprite_t *sprite = data_get_cursor();
  video->set_cursor(sprite);
  data_sprite_free(sprite);

  gfx = this;
}

gfx_t::~gfx_t()
{
  while(!image_cache.empty()) {
    delete image_cache.begin()->second;
    image_cache.erase(image_cache.begin());
  }
}

gfx_t *
gfx_t::get_gfx()
{
  return gfx;
}

/* Draw the opaque sprite with data file index of
   sprite at x, y in dest frame. */
void
frame_t::draw_sprite(int x, int y, unsigned int sprite)
{
  image_t *image = gfx->get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_sprite_for_index(sprite);
    if (spr == NULL) {
      return;
    }
    image = gfx->get_video()->image_from_sprite(spr);
    data_sprite_free(spr);
    gfx->add_image_to_cache(sprite, 0, 0, image);
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
  image_t *image = gfx->get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_transparent_sprite_for_index(sprite, 0);
    if (spr == NULL) {
      return;
    }
    image = gfx->get_video()->image_from_sprite(spr);
    data_sprite_free(spr);
    gfx->add_image_to_cache(sprite, 0, 0, image);
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
  image_t *image = gfx->get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_transparent_sprite_for_index(sprite, 0);
    if (spr == NULL) {
      return;
    }
    image = gfx->get_video()->image_from_sprite(spr);
    data_sprite_free(spr);
    gfx->add_image_to_cache(sprite, 0, 0, image);
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
  image_t *image = gfx->get_image_from_cache(sprite, 0, color_offs);
  if (image == NULL) {
    sprite_t *spr = data_transparent_sprite_for_index(sprite, color_offs);
    if (spr == NULL) {
      return;
    }
    image = gfx->get_video()->image_from_sprite(spr);
    data_sprite_free(spr);
    gfx->add_image_to_cache(sprite, 0, color_offs, image);
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
  image_t *image = gfx->get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_transparent_sprite_for_index(sprite, 0);
    if (spr == NULL) {
      return;
    }
    image = gfx->get_video()->image_from_sprite(spr);
    data_sprite_free(spr);
    gfx->add_image_to_cache(sprite, 0, 0, image);
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
  image_t *image = gfx->get_image_from_cache(sprite, mask, 0);
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
    image = gfx->get_video()->image_from_sprite(masked);
    data_sprite_free(masked);
    gfx->add_image_to_cache(sprite, mask, 0, image);
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
  image_t *image = gfx->get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_overlay_sprite_for_index(sprite);
    if (spr == NULL) {
      return;
    }
    image = gfx->get_video()->image_from_sprite(spr);
    data_sprite_free(spr);
    gfx->add_image_to_cache(sprite, 0, 0, image);
  }

  x += image->get_offset_x();
  y += image->get_offset_y();

  native_frame->draw(image, x, y, 0);
}

void
frame_t::draw_overlay_sprite(int x, int y, unsigned int sprite, float progress)
{
  image_t *image = gfx->get_image_from_cache(sprite, 0, 0);
  if (image == NULL) {
    sprite_t *spr = data_overlay_sprite_for_index(sprite);
    if (spr == NULL) {
      return;
    }
    image = gfx->get_video()->image_from_sprite(spr);
    data_sprite_free(spr);
    gfx->add_image_to_cache(sprite, 0, 0, image);
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

  image_t *image = gfx->get_image_from_cache(sprite, mask, 0);
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
    image = gfx->get_video()->image_from_sprite(masked);
    data_sprite_free(masked);
    gfx->add_image_to_cache(sprite, mask, 0, image);
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
gfx_t::create_frame(unsigned int width, unsigned int height)
{
  return new frame_t(width, height, this);
}

frame_t::frame_t(video_frame_t *native_frame, gfx_t *gfx)
{
  this->gfx = gfx;
  this->native_frame = native_frame;
}

frame_t::frame_t(unsigned int width, unsigned int height, gfx_t *gfx)
{
  this->gfx = gfx;
  native_frame = gfx->get_video()->frame_create(width, height);
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
gfx_t::get_screen_frame()
{
  return new frame_t(video->get_screen_frame(), this);
}

/* Enable or disable fullscreen mode */
bool
gfx_t::set_fullscreen(bool enable)
{
  return video->set_fullscreen(enable);
}

/* Check whether fullscreen mode is enabled */
bool
gfx_t::is_fullscreen()
{
  return video->is_fullscreen();
}

bool
gfx_t::is_fullscreen_possible()
{
  return video->is_fullscreen_possible();
}

void
gfx_t::set_resolution(unsigned int width, unsigned int height, bool fullscreen)
{
  video->set_resolution(width, height, fullscreen);
}

void
gfx_t::get_resolution(unsigned int &width, unsigned int &height)
{
  video->get_resolution(width, height);
}

void
gfx_t::swap_buffers()
{
  video->swap_buffers();
}

void
gfx_t::add_image_to_cache(unsigned int sprite, unsigned int mask, unsigned char offset, image_t *image)
{
  image_cache[gfx_image_id(sprite, mask, offset)] = image;
}

image_t *
gfx_t::get_image_from_cache(unsigned int sprite, unsigned int mask, unsigned char offset)
{
  image_map_t::iterator result = image_cache.find(gfx_image_id(sprite, mask, offset));
  if(result == image_cache.end()) {
    return NULL;
  }
  return result->second;
}

uint64_t
gfx_t::gfx_image_id(unsigned int sprite, unsigned int mask, unsigned char offset)
{
  uint64_t result = (uint64_t)sprite + (((uint64_t)mask) << 32) + (((uint64_t)offset) << 48);
  return result;
}
