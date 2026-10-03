/*
 * video-sdl.cc - SDL graphics rendering
 *
 * Copyright (C) 2013-2015  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "src/video-sdl.h"

#include <sstream>

#include <SDL3/SDL.h>

SDL_Exception::SDL_Exception(const std::string &description) throw()
  : Video_Exception(description) {
  sdl_error = SDL_GetError();
}

SDL_Exception::~SDL_Exception() throw() {
}

const char *
SDL_Exception::get_description() const {
  std::stringstream str;
  str << Video_Exception::get_description();
  str << "(" << sdl_error << ")";
  full_description = str.str();
  return full_description.c_str();
}

const char *
SDL_Exception::get_platform() const {
  return "SDL";
}

int video_sdl_t::bpp = 32;
Uint32 video_sdl_t::Rmask = 0x0000FF00;
Uint32 video_sdl_t::Gmask = 0x00FF0000;
Uint32 video_sdl_t::Bmask = 0xFF000000;
Uint32 video_sdl_t::Amask = 0x000000FF;
SDL_PixelFormat video_sdl_t::pixel_format = SDL_PIXELFORMAT_RGBA8888;

video_sdl_t::video_sdl_t() throw(Video_Exception) {
  screen = NULL;
  screen_texture = NULL;
  cursor = NULL;
  fullscreen = false;
  zoom_factor = 1.f;

  /* Initialize defaults and Video subsystem */
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    throw SDL_Exception("Unable to initialize SDL video");
  }

  /* Create window and renderer */
  window = SDL_CreateWindow("freeserf", 800, 600, SDL_WINDOW_RESIZABLE);
  if (window == NULL) {
    throw SDL_Exception("Unable to create SDL window");
  }

  /* Create renderer for window */
  renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    throw SDL_Exception("Unable to create SDL renderer");
  }

  /* Determine optimal pixel format for current window */
  const SDL_PixelFormat *formats = static_cast<const SDL_PixelFormat *>(
    SDL_GetPointerProperty(SDL_GetRendererProperties(renderer),
                           SDL_PROP_RENDERER_TEXTURE_FORMATS_POINTER, NULL));
  for (int i = 0; formats != NULL &&
                  formats[i] != SDL_PIXELFORMAT_UNKNOWN; i++) {
    SDL_PixelFormat format = formats[i];
    int bpp = SDL_BITSPERPIXEL(format);
    if (32 == bpp) {
      pixel_format = format;
      break;
    }
  }
  SDL_GetMasksForPixelFormat(pixel_format, &bpp,
                             &Rmask, &Gmask, &Bmask, &Amask);

  /* Set scaling mode */
  SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_LINEAR);
}

video_sdl_t::~video_sdl_t() {
  if (screen != NULL) {
    delete screen;
    screen = NULL;
  }
  set_cursor(NULL, 0, 0);
  SDL_Quit();
}

video_t *
video_t::get_instance() {
  if (instance == NULL) {
    instance = new video_sdl_t();
  }
  return instance;
}

SDL_Surface *
video_sdl_t::create_surface(int width, int height) {
  SDL_Surface *surf = SDL_CreateSurface(width, height, pixel_format);
  if (surf == NULL) {
    throw SDL_Exception("Unable to create SDL surface");
  }

  return surf;
}

void
video_sdl_t::set_resolution(unsigned int width, unsigned int height,
                            bool fullscreen) throw(Video_Exception) {
  /* Set fullscreen mode */
  /* Fullscreen without a display mode is desktop fullscreen. */
  if (!SDL_SetWindowFullscreen(window, fullscreen)) {
    throw SDL_Exception("Unable to set window fullscreen");
  }

  if (screen == NULL) {
    screen = new video_frame_t();
  }

  /* Allocate new screen surface and texture */
  if (screen->texture != NULL) {
    SDL_DestroyTexture(screen->texture);
  }
  screen->texture = create_texture(width, height);

  if (screen_texture != NULL) {
    SDL_DestroyTexture(screen_texture);
  }
  screen_texture = SDL_CreateTexture(renderer, pixel_format,
                                     SDL_TEXTUREACCESS_STREAMING,
                                     width, height);
  if (screen_texture == NULL) {
    throw SDL_Exception("Unable to create SDL texture");
  }

  /* Set logical size of screen. The logical presentation belongs to the
     current render target, so select the window first. */
  SDL_SetRenderTarget(renderer, NULL);
  if (!SDL_SetRenderLogicalPresentation(renderer, width, height,
                                        SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
    throw SDL_Exception("Unable to set logical size");
  }

  this->fullscreen = fullscreen;
}

void
video_sdl_t::get_resolution(unsigned int *width, unsigned int *height) {
  int w = 0;
  int h = 0;
  SDL_GetRenderOutputSize(renderer, &w, &h);
  if (width != NULL) {
    *width = w;
  }
  if (height != NULL) {
    *height = h;
  }
}

void
video_sdl_t::set_fullscreen(bool enable) throw(Video_Exception) {
  int width = 0;
  int height = 0;
  SDL_GetRenderOutputSize(renderer, &width, &height);
  set_resolution(width, height, enable);
}

bool
video_sdl_t::is_fullscreen() {
  return fullscreen;
}

video_frame_t *
video_sdl_t::get_screen_frame() {
  return screen;
}

video_frame_t *
video_sdl_t::create_frame(unsigned int width, unsigned int height) {
  video_frame_t *frame = new video_frame_t;
  frame->texture = create_texture(width, height);
  return frame;
}

void
video_sdl_t::destroy_frame(video_frame_t *frame) {
  SDL_DestroyTexture(frame->texture);
  delete frame;
}

video_image_t *
video_sdl_t::create_image(void *data, unsigned int width, unsigned int height) {
  video_image_t *image = new video_image_t();
  image->w = width;
  image->h = height;
  image->texture = create_texture_from_data(data, width, height);
  return image;
}

void
video_sdl_t::destroy_image(video_image_t *image) {
  SDL_DestroyTexture(image->texture);
  delete image;
}

void
video_sdl_t::warp_mouse(int x, int y) {
  SDL_WarpMouseInWindow(window, static_cast<float>(x),
                        static_cast<float>(y));
}

SDL_Surface *
video_sdl_t::create_surface_from_data(void *data, int width, int height) {
  /* Create sprite surface */
  SDL_Surface *surf = SDL_CreateSurfaceFrom(width, height,
                                            SDL_PIXELFORMAT_ARGB8888,
                                            data, 4 * width);
  if (surf == NULL) {
    throw SDL_Exception("Unable to create sprite surface");
  }

  /* Covert to screen format */
  SDL_Surface *surf_screen = SDL_ConvertSurface(surf, pixel_format);
  if (surf_screen == NULL) {
    throw SDL_Exception("Unable to convert sprite surface");
  }

  SDL_DestroySurface(surf);

  return surf_screen;
}

SDL_Texture *
video_sdl_t::create_texture(int width, int height) {
  SDL_Texture *texture = SDL_CreateTexture(renderer, pixel_format,
                                           SDL_TEXTUREACCESS_TARGET,
                                           width, height);

  if (texture == NULL) {
    throw SDL_Exception("Unable to create SDL texture");
  }

  SDL_SetRenderTarget(renderer, texture);
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(renderer);

  return texture;
}

SDL_Texture *
video_sdl_t::create_texture_from_data(void *data, int width, int height) {
  SDL_Surface *surf = create_surface_from_data(data, width, height);
  if (surf == NULL) {
    return NULL;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
  if (texture == NULL) {
    throw SDL_Exception("Unable to create SDL texture from data");
  }

  SDL_DestroySurface(surf);

  return texture;
}

void
video_sdl_t::draw_image(const video_image_t *image, int x, int y, int y_offset,
                        video_frame_t *dest) {
  SDL_FRect dest_rect = { static_cast<float>(x),
                          static_cast<float>(y + y_offset),
                          static_cast<float>(image->w),
                          static_cast<float>(image->h - y_offset) };
  SDL_FRect src_rect = { 0.f, static_cast<float>(y_offset),
                         static_cast<float>(image->w),
                         static_cast<float>(image->h - y_offset) };

  /* Blit sprite */
  SDL_SetRenderTarget(renderer, dest->texture);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  if (!SDL_RenderTexture(renderer, image->texture, &src_rect, &dest_rect)) {
    throw SDL_Exception("RenderTexture error");
  }
}

void
video_sdl_t::draw_frame(int dx, int dy, video_frame_t *dest, int sx, int sy,
                        video_frame_t *src, int w, int h) {
  SDL_FRect dest_rect = { static_cast<float>(dx), static_cast<float>(dy),
                          static_cast<float>(w), static_cast<float>(h) };
  SDL_FRect src_rect = { static_cast<float>(sx), static_cast<float>(sy),
                         static_cast<float>(w), static_cast<float>(h) };

  SDL_SetRenderTarget(renderer, dest->texture);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  if (!SDL_RenderTexture(renderer, src->texture, &src_rect, &dest_rect)) {
    throw SDL_Exception("RenderTexture error");
  }
}

void
video_sdl_t::draw_rect(int x, int y, unsigned int width, unsigned int height,
                       const video_color_t color, video_frame_t *dest) {
  /* Draw rectangle. */
  fill_rect(x, y, width, 1, color, dest);
  fill_rect(x, y+height-1, width, 1, color, dest);
  fill_rect(x, y, 1, height, color, dest);
  fill_rect(x+width-1, y, 1, height, color, dest);
}

void
video_sdl_t::fill_rect(int x, int y, unsigned int width, unsigned int height,
                       const video_color_t color, video_frame_t *dest) {
  SDL_FRect rect = { static_cast<float>(x), static_cast<float>(y),
                     static_cast<float>(width), static_cast<float>(height) };

  /* Fill rectangle */
  SDL_SetRenderTarget(renderer, dest->texture);
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xff);
  if (!SDL_RenderFillRect(renderer, &rect)) {
    throw SDL_Exception("RenderFillRect error");
  }
}

void
video_sdl_t::swap_buffers() {
  SDL_SetRenderTarget(renderer, NULL);
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
  SDL_RenderClear(renderer);
  SDL_RenderTexture(renderer, screen->texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void
video_sdl_t::set_cursor(void *data, unsigned int width, unsigned int height) {
  if (cursor != NULL) {
    SDL_SetCursor(SDL_GetDefaultCursor());
    SDL_DestroyCursor(cursor);
    cursor = NULL;
  }

  if (data == NULL) return;

  SDL_Surface *surface = create_surface_from_data(data, width, height);
  cursor = SDL_CreateColorCursor(surface, 8, 8);
  SDL_DestroySurface(surface);
  SDL_SetCursor(cursor);
}

bool
video_sdl_t::set_zoom_factor(float factor) {
  if ((factor < 0.2f) || (factor > 1.f)) {
    return false;
  }

  unsigned int width = 0;
  unsigned int height = 0;
  get_resolution(&width, &height);
  zoom_factor = factor;

  width = (unsigned int)(static_cast<float>(width) * zoom_factor);
  height = (unsigned int)(static_cast<float>(height) * zoom_factor);
  set_resolution(width, height, is_fullscreen());

  return true;
}
