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

#include "video-sdl.h"
#include "data.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
#ifndef _MSC_VER
}
#endif

video_t *
video_create()
{
  return new sdl_video_t();
}

int sdl_video_t::bpp = 32;
Uint32 sdl_video_t::Rmask = 0xFF000000;
Uint32 sdl_video_t::Gmask = 0x00FF0000;
Uint32 sdl_video_t::Bmask = 0x0000FF00;
Uint32 sdl_video_t::Amask = 0x000000FF;
Uint32 sdl_video_t::pixel_format = SDL_PIXELFORMAT_RGBA8888;

sdl_video_t::sdl_video_t()
{
  screen = NULL;

  /* Initialize defaults and Video subsystem */
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    LOGE("sdl-video", "Unable to initialize SDL: %s.", SDL_GetError());
    return;
  }

  /* Create window and renderer */
  window = SDL_CreateWindow("freeserf",
          SDL_WINDOWPOS_UNDEFINED,
          SDL_WINDOWPOS_UNDEFINED,
          800, 600, SDL_WINDOW_RESIZABLE);
  if (window == NULL) {
    LOGE("sdl-video", "Unable to create SDL window: %s.", SDL_GetError());
    return;
  }

  /* Create renderer for window */
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  if (renderer == NULL) {
    LOGE("sdl-video", "Unable to create SDL renderer: %s.", SDL_GetError());
    return;
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
}

sdl_video_t::~sdl_video_t()
{
  set_cursor(NULL);
  SDL_Quit();
}

bool
sdl_video_t::set_resolution(unsigned int width, unsigned int height, bool fullscreen)
{
  /* Set fullscreen mode */
  int r = SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
  if (r < 0) {
    LOGE("sdl-video", "Unable to set window fullscreen: %s.", SDL_GetError());
    return false;
  }

  /* Allocate new screen surface and texture */
  if (screen != NULL) {
    screen->set_size(width, height);
  }
  else {
    screen = new sdl_frame_t(width, height, this);
  }

  /* Set logical size of screen */
  r = SDL_RenderSetLogicalSize(renderer, width, height);
  if (r < 0) {
    LOGE("sdl-video", "Unable to set logical size: %s.", SDL_GetError());
    return false;
  }

  this->fullscreen = fullscreen;

  return true;
}

void
sdl_video_t::get_resolution(unsigned int &width, unsigned int &height)
{
  int w = 0;
  int h = 0;
  SDL_GL_GetDrawableSize(window, &w, &h);
  width = w;
  height = h;
}

bool
sdl_video_t::is_fullscreen_possible()
{
  return true;
}

bool
sdl_video_t::set_fullscreen(bool enable)
{
  int width = 0;
  int height = 0;
  SDL_GL_GetDrawableSize(window, &width, &height);
  return set_resolution(width, height, enable);
}

bool
sdl_video_t::is_fullscreen()
{
  return this->fullscreen;
}

video_frame_t *
sdl_video_t::get_screen_frame()
{
  if (screen == NULL) {
    screen = new sdl_frame_t(100, 100, this);
  }

  return screen;
}

sdl_frame_t::sdl_frame_t(unsigned int width, unsigned int height, sdl_video_t *video)
{
  texture = NULL;
  this->video = video;

  set_size(width, height);
}

sdl_frame_t::~sdl_frame_t()
{
  if (texture != NULL) {
    SDL_DestroyTexture(texture);
  }
}

void
sdl_frame_t::set_size(unsigned int width, unsigned int height)
{
  if (texture != NULL) {
    SDL_DestroyTexture(texture);
  }

  this->width = width;
  this->height = height;

  texture = SDL_CreateTexture(video->get_renderer(), video->get_pixel_format(),
                              SDL_TEXTUREACCESS_TARGET,
                              width, height);

  if (texture == NULL) {
    LOGE("sdl-video", "Unable to create SDL texture: %s.", SDL_GetError());
  }
}

video_frame_t *
sdl_video_t::frame_create(unsigned int width, unsigned int height)
{
  return new sdl_frame_t(width, height, this);
}

void
sdl_video_t::warp_mouse(int x, int y)
{
  SDL_WarpMouseInWindow(NULL, x, y);
}

SDL_Surface *
sdl_image_t::create_surface_from_sprite(const sprite_t *sprite, Uint32 pixel_format)
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

sdl_image_t::sdl_image_t(sprite_t *sprite, sdl_video_t *video)
  : image_t(sprite)
{
  this->video = video;

  /* Create sprite surface */
  SDL_Surface *surf = create_surface_from_sprite(sprite, video->get_pixel_format());

  texture = SDL_CreateTextureFromSurface(video->get_renderer(), surf);
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
sdl_video_t::image_from_sprite(sprite_t *sprite)
{
  return new sdl_image_t(sprite, this);
}

void
sdl_frame_t::draw(image_t *image, int x, int y, int y_offset)
{
  SDL_Rect dest_rect = { x, y + y_offset, (int)image->get_width(), (int)image->get_height() - y_offset};
  SDL_Rect src_rect = { 0, y_offset, (int)image->get_width(), (int)image->get_height() - y_offset};

  /* Blit sprite */
  SDL_SetRenderTarget(video->get_renderer(), texture);
  int r = SDL_RenderCopy(video->get_renderer(), ((sdl_image_t*)image)->get_texture(), &src_rect, &dest_rect);
  if (r < 0) {
    LOGE("sdl-video", "RenderCopy error: %s.", SDL_GetError());
  }
}

void
sdl_frame_t::draw(int dx, int dy, video_frame_t *dest, int sx, int sy, int w, int h)
{
  SDL_Rect dest_rect = { dx, dy, w, h };
  SDL_Rect src_rect = { sx, sy, w, h };

  SDL_SetRenderTarget(video->get_renderer(), ((sdl_frame_t*)dest)->texture);
  int r = SDL_RenderCopy(video->get_renderer(), texture, &src_rect, &dest_rect);
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
  SDL_SetRenderTarget(video->get_renderer(), texture);
  SDL_SetRenderDrawColor(video->get_renderer(), color->r, color->g, color->b, 0xff);
  int r = SDL_RenderFillRect(video->get_renderer(), &rect);
  if (r < 0) {
    LOGE("sdl-video", "RenderFillRect error: %s.", SDL_GetError());
  }
}

void
sdl_video_t::swap_buffers()
{
  SDL_SetRenderTarget(renderer, NULL);
  SDL_RenderCopy(renderer, screen->get_texture(), NULL, NULL);
  SDL_RenderPresent(renderer);
}

void
sdl_video_t::set_cursor(const sprite_t *sprite)
{
  if (cursor != NULL) {
    SDL_SetCursor(NULL);
    SDL_FreeCursor(cursor);
    cursor = NULL;
  }

  if (sprite == NULL) return;

  SDL_Surface *surface = sdl_image_t::create_surface_from_sprite(sprite, pixel_format);
  cursor = SDL_CreateColorCursor(surface, 8, 8);
  SDL_SetCursor(cursor);
  SDL_FreeSurface(surface);
}
