/*
 * video.cc - Video graphics rendering
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

#include "video.h"
#include "data.h"

image_t::image_t(sprite_t *sprite)
{
  delta_x = sprite->delta_x;
  delta_y = sprite->delta_y;
  offset_x = sprite->offset_x;
  offset_y = sprite->offset_y;
  width = sprite->width;
  height = sprite->height;
}
