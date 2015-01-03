/*
 * pathfinder.h - Path finder functions
 *
 * Copyright (C) 2012  Jon Lund Steffensen <jonlst@gmail.com>
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

#ifndef _PATHFINDER_H
#define _PATHFINDER_H

#ifndef _MSC_VER
extern "C" {
#endif
  #include "map.h"
#ifndef _MSC_VER
}
#endif

dir_t *pathfinder_map(map_pos_t start, map_pos_t end, uint *length, map_t *map);

#endif /* !_PATHFINDER_H */
