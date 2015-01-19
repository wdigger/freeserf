/*
 * video-sdl.cc - SDL graphics rendering
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

#include "video.h"
#include "video-sdl.h"
#include "version.h"
#include "data.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
#ifndef _MSC_VER
}
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <signal.h>

#include "SDL.h"

static SDL_Window *window;
static SDL_Renderer *renderer;
static int bpp = 32;
static Uint32 Rmask = 0xFF000000;
static Uint32 Gmask = 0x00FF0000;
static Uint32 Bmask = 0x0000FF00;
static Uint32 Amask = 0x000000FF;
static Uint32 pixel_format = SDL_PIXELFORMAT_RGBA8888;

static sdl_frame_t *screen = NULL;
static int is_fullscreen;
static SDL_Cursor *cursor = NULL;

int
video_init()
{
  /* Initialize defaults and Video subsystem */
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    LOGE("sdl-video", "Unable to initialize SDL: %s.", SDL_GetError());
    return -1;
  }

  /* Display program name and version in caption */
  char caption[64];
  snprintf(caption, 64, "freeserf %s", FREESERF_VERSION);

  /* Create window and renderer */
  window = SDL_CreateWindow(caption,
          SDL_WINDOWPOS_UNDEFINED,
          SDL_WINDOWPOS_UNDEFINED,
          800, 600, SDL_WINDOW_RESIZABLE);
  if (window == NULL) {
    LOGE("sdl-video", "Unable to create SDL window: %s.", SDL_GetError());
    return -1;
  }

  /* Create renderer for window */
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  if (renderer == NULL) {
    LOGE("sdl-video", "Unable to create SDL renderer: %s.", SDL_GetError());
    return -1;
  }
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  /* Determine optimal pixel format for current window */
  SDL_RendererInfo render_info = {0};
  SDL_GetRendererInfo(renderer, &render_info);
  for (Uint32 i = 0; i < render_info.num_texture_formats; i++) {
    Uint32 format = render_info.texture_formats[i];
    if (SDL_BITSPERPIXEL(format) == 32) {
      pixel_format = format;
      break;
    }
  }
  SDL_PixelFormatEnumToMasks(pixel_format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);

  /* Set scaling mode */
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

  /* Exit on signals */
  signal(SIGINT, exit);
  signal(SIGTERM, exit);

  return 0;
}

void
video_deinit()
{
  if (screen != NULL) {
    delete screen;
    screen = NULL;
  }

  video_set_cursor(NULL);
  SDL_Quit();
}

int
video_set_resolution(int width, int height, int fullscreen)
{
  /* Set fullscreen mode */
  int r = SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
  if (r < 0) {
    LOGE("sdl-video", "Unable to set window fullscreen: %s.", SDL_GetError());
    return -1;
  }

  /* Allocate new screen surface and texture */
  if (screen != NULL) {
    delete screen;
    screen = NULL;
  }
  screen = new sdl_frame_t(width, height);

  /* Set logical size of screen */
  r = SDL_RenderSetLogicalSize(renderer, width, height);
  if (r < 0) {
    LOGE("sdl-video", "Unable to set logical size: %s.", SDL_GetError());
    return -1;
  }

  is_fullscreen = fullscreen;

  return 0;
}

void
video_get_resolution(int *width, int *height)
{
  SDL_GL_GetDrawableSize(window, width, height);
}

int
video_is_fullscreen_possible()
{
  return 1;
}

int
video_set_fullscreen(int enable)
{
  video_frame_t *screen = video_get_screen_frame();
  return video_set_resolution(screen->get_width(), screen->get_height(), enable);
}

int
video_is_fullscreen()
{
  return is_fullscreen;
}

video_frame_t *
video_get_screen_frame()
{
  return screen;
}

sdl_frame_t::sdl_frame_t(unsigned int width, unsigned int height)
{
  this->width = width;
  this->height = height;

  texture = SDL_CreateTexture(renderer, pixel_format,
                                  SDL_TEXTUREACCESS_TARGET,
                                  width, height);

  if (texture == NULL) {
    LOGE("sdl-video", "Unable to create SDL texture: %s.", SDL_GetError());
  }
}

sdl_frame_t::~sdl_frame_t()
{
  if (texture != NULL) {
    SDL_DestroyTexture(texture);
  }
}

video_frame_t *
video_frame_create(unsigned int width, unsigned int height)
{
  return new sdl_frame_t(width, height);
}

void
video_warp_mouse(int x, int y)
{
  SDL_WarpMouseInWindow(NULL, x, y);
}

static SDL_Surface *
video_create_surface_from_sprite(const sprite_t *sprite)
{
  /* Create sprite surface */
  SDL_Surface *sprite_surf =
    SDL_CreateRGBSurfaceFrom(sprite->data, (int)sprite->width, (int)sprite->height,
           32, (int)sprite->width*4,
           0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
  if (sprite_surf == NULL) {
    LOGE("sdl-video", "Unable to create sprite surface: %s.",
         SDL_GetError());
    exit(EXIT_FAILURE);
  }

  /* Covert to screen format */
  SDL_Surface *surf = NULL;

  surf = SDL_ConvertSurfaceFormat(sprite_surf, pixel_format, 0);
  if (surf == NULL) {
    LOGE("sdl-video", "Unable to convert sprite surface: %s.",
         SDL_GetError());
    exit(EXIT_FAILURE);
  }

  SDL_FreeSurface(sprite_surf);

  return surf;
}

sdl_image_t::sdl_image_t(sprite_t *sprite)
  : image_t(sprite)
{
  /* Create sprite surface */
  SDL_Surface *surf = video_create_surface_from_sprite(sprite);

  texture = SDL_CreateTextureFromSurface(renderer, surf);
  if (texture == NULL) {
    LOGE("sdl-video", "Unable to create SDL texture: %s.", SDL_GetError());
  }

  SDL_FreeSurface(surf);
}

sdl_image_t::~sdl_image_t()
{
  if (texture != NULL) {
    SDL_DestroyTexture(texture);
  }
}

image_t *
video_image_from_sprite(sprite_t *sprite)
{
  return new sdl_image_t(sprite);
}

void
sdl_frame_t::draw(image_t *image, int x, int y, int y_offset)
{
  SDL_Rect dest_rect = { x, y + y_offset, (int)image->get_width(), (int)image->get_height() - y_offset};
  SDL_Rect src_rect = { 0, y_offset, (int)image->get_width(), (int)image->get_height() - y_offset};

  /* Blit sprite */
  SDL_SetRenderTarget(renderer, texture);
  int r = SDL_RenderCopy(renderer, ((sdl_image_t*)image)->get_texture(), &src_rect, &dest_rect);
  if (r < 0) {
    LOGE("sdl-video", "RenderCopy error: %s.", SDL_GetError());
  }
}

void
sdl_frame_t::draw(int dx, int dy, video_frame_t *dest, int sx, int sy, int w, int h)
{
  SDL_Rect dest_rect = { dx, dy, w, h };
  SDL_Rect src_rect = { sx, sy, w, h };

  SDL_SetRenderTarget(renderer, ((sdl_frame_t*)dest)->texture);
  int r = SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect);
  if (r < 0) {
    LOGE("sdl-video", "RenderCopy error: %s", SDL_GetError());
  }
}

void
sdl_frame_t::fill_rect(int x, int y, int width, int height, const color_t *color)
{
  SDL_Rect rect = {
    x, y,
    width, height
  };

  /* Fill rectangle */
  SDL_SetRenderTarget(renderer, texture);
  SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, 0xff);
  int r = SDL_RenderFillRect(renderer, &rect);
  if (r < 0) {
    LOGE("sdl-video", "RenderFillRect error: %s.", SDL_GetError());
  }
}

void
video_swap_buffers()
{
  SDL_SetRenderTarget(renderer, NULL);
  SDL_RenderCopy(renderer, screen->get_texture(), NULL, NULL);
  SDL_RenderPresent(renderer);
}

void
video_set_cursor(const sprite_t *sprite)
{
  if (cursor != NULL) {
    SDL_SetCursor(NULL);
    SDL_FreeCursor(cursor);
    cursor = NULL;
  }

  if (sprite == NULL) return;

  SDL_Surface *surface = video_create_surface_from_sprite(sprite);
  cursor = SDL_CreateColorCursor(surface, 8, 8);
  SDL_SetCursor(cursor);
  SDL_FreeSurface(surface);
}
