/*
 * map.c - Map generators and map update functions
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

/* Basically the map is constructed from a regular, square grid, with
   rows and columns, except that the grid is actually sheared like this:
   http://mathworld.wolfram.com/Polyrhomb.html
   This is the foundational 2D grid for the map, where each vertex can be
   identified by an integer col and row (commonly encoded as map_pos_t).

   Each tile has the shape of a rhombus:
      A ______ B
       /\    /
      /  \  /
   C /____\/ D

   but is actually composed of two triangles called "up" (a,c,d) and
   "down" (a,b,d). A serf can move on the perimeter of any of these
   triangles. Each vertex has various properties associated with it,
   among others a height value which means that the 3D landscape is
   defined by these points in (col, row, height)-space. */

#include "map.h"
#include "debug.h"

#include <string.h>

/* Facilitates quick lookup of offsets following a spiral pattern in the map data.
 The columns following the second are filled out by setup_spiral_pattern(). */
static int spiral_pattern[] = {
	0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	9, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	9, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	9, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	9, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	9, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	9, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	9, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	16, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	24, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	24, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int spiral_pattern_initialized = 0;

/* Initialize the global spiral_pattern. */
static void
init_spiral_pattern()
{
	if (spiral_pattern_initialized) {
		return;
	}

	static const int spiral_matrix[] = {
		1,  0,  0,  1,
		1,  1, -1,  0,
		0,  1, -1, -1,
		-1,  0,  0, -1,
		-1, -1,  1,  0,
		0, -1,  1,  1
	};

	for (int i = 0; i < 49; i++) {
		int x = spiral_pattern[2 + 12*i];
		int y = spiral_pattern[2 + 12*i + 1];

		for (int j = 0; j < 6; j++) {
			spiral_pattern[2+12*i+2*j] = x*spiral_matrix[4*j+0] + y*spiral_matrix[4*j+2];
			spiral_pattern[2+12*i+2*j+1] = x*spiral_matrix[4*j+1] + y*spiral_matrix[4*j+3];
		}
	}

	spiral_pattern_initialized = 1;
}

int *
get_spiral_pattern()
{
	return spiral_pattern;
}

/* Map map_obj_t to map_space_t. */
const map_space_t map_space_from_obj[] = {
	MAP_SPACE_OPEN,				// MAP_OBJ_NONE = 0,
	MAP_SPACE_FILLED,			// MAP_OBJ_FLAG,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_SMALL_BUILDING,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_LARGE_BUILDING,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_CASTLE,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,

	MAP_SPACE_FILLED,			// MAP_OBJ_TREE_0 = 8,
	MAP_SPACE_FILLED,			// MAP_OBJ_TREE_1,
	MAP_SPACE_FILLED,			// MAP_OBJ_TREE_2, /* 10 */
	MAP_SPACE_FILLED,			// MAP_OBJ_TREE_3,
	MAP_SPACE_FILLED,			// MAP_OBJ_TREE_4,
	MAP_SPACE_FILLED,			// MAP_OBJ_TREE_5,
	MAP_SPACE_FILLED,			// MAP_OBJ_TREE_6,
	MAP_SPACE_FILLED,			// MAP_OBJ_TREE_7, /* 15 */

	MAP_SPACE_FILLED,			// MAP_OBJ_PINE_0,
	MAP_SPACE_FILLED,			// MAP_OBJ_PINE_1,
	MAP_SPACE_FILLED,			// MAP_OBJ_PINE_2,
	MAP_SPACE_FILLED,			// MAP_OBJ_PINE_3,
	MAP_SPACE_FILLED,			// MAP_OBJ_PINE_4, /* 20 */
	MAP_SPACE_FILLED,			// MAP_OBJ_PINE_5,
	MAP_SPACE_FILLED,			// MAP_OBJ_PINE_6,
	MAP_SPACE_FILLED,			// MAP_OBJ_PINE_7,

	MAP_SPACE_FILLED,			// MAP_OBJ_PALM_0,
	MAP_SPACE_FILLED,			// MAP_OBJ_PALM_1, /* 25 */
	MAP_SPACE_FILLED,			// MAP_OBJ_PALM_2,
	MAP_SPACE_FILLED,			// MAP_OBJ_PALM_3,

	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_WATER_TREE_0,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_WATER_TREE_1,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_WATER_TREE_2, /* 30 */
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_WATER_TREE_3,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,
	MAP_SPACE_OPEN,

	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_STONE_0 = 72,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_STONE_1,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_STONE_2,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_STONE_3, /* 75 */
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_STONE_4,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_STONE_5,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_STONE_6,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_STONE_7,

	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_SANDSTONE_0, /* 80 */
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_SANDSTONE_1,
																				// 
	MAP_SPACE_FILLED,			// MAP_OBJ_CROSS,
	MAP_SPACE_OPEN,				// MAP_OBJ_STUB,

	MAP_SPACE_OPEN,				// MAP_OBJ_STONE,
	MAP_SPACE_OPEN,				// MAP_OBJ_SANDSTONE_3, /* 85 */

	MAP_SPACE_OPEN,				// MAP_OBJ_CADAVER_0,
	MAP_SPACE_OPEN,				// MAP_OBJ_CADAVER_1,

	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_WATER_STONE_0,
	MAP_SPACE_IMPASSABLE,		// MAP_OBJ_WATER_STONE_1,

	MAP_SPACE_FILLED,			// MAP_OBJ_CACTUS_0, /* 90 */
	MAP_SPACE_FILLED,			// MAP_OBJ_CACTUS_1,

	MAP_SPACE_FILLED,			// MAP_OBJ_DEAD_TREE,

	MAP_SPACE_FILLED,			// MAP_OBJ_FELLED_PINE_0,
	MAP_SPACE_FILLED,			// MAP_OBJ_FELLED_PINE_1,
	MAP_SPACE_FILLED,			// MAP_OBJ_FELLED_PINE_2, /* 95 */
	MAP_SPACE_FILLED,			// MAP_OBJ_FELLED_PINE_3,
	MAP_SPACE_OPEN,				// MAP_OBJ_FELLED_PINE_4,

	MAP_SPACE_FILLED,			// MAP_OBJ_FELLED_TREE_0,
	MAP_SPACE_FILLED,			// MAP_OBJ_FELLED_TREE_1,
	MAP_SPACE_FILLED,			// MAP_OBJ_FELLED_TREE_2, /* 100 */
	MAP_SPACE_FILLED,			// MAP_OBJ_FELLED_TREE_3,
	MAP_SPACE_OPEN,				// MAP_OBJ_FELLED_TREE_4,

	MAP_SPACE_FILLED,			// MAP_OBJ_NEW_PINE,
	MAP_SPACE_FILLED,			// MAP_OBJ_NEW_TREE,

	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_SEEDS_0, /* 105 */
	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_SEEDS_1,
	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_SEEDS_2,
	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_SEEDS_3,
	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_SEEDS_4,
	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_SEEDS_5, /* 110 */
	MAP_SPACE_OPEN,				// MAP_OBJ_FIELD_EXPIRED,

	MAP_SPACE_OPEN,				// MAP_OBJ_SIGN_LARGE_GOLD,
	MAP_SPACE_OPEN,				// MAP_OBJ_SIGN_SMALL_GOLD,
	MAP_SPACE_OPEN,				// MAP_OBJ_SIGN_LARGE_IRON,
	MAP_SPACE_OPEN,				// MAP_OBJ_SIGN_SMALL_IRON, /* 115 */
	MAP_SPACE_OPEN,				// MAP_OBJ_SIGN_LARGE_COAL,
	MAP_SPACE_OPEN,				// MAP_OBJ_SIGN_SMALL_COAL,
	MAP_SPACE_OPEN,				// MAP_OBJ_SIGN_LARGE_STONE,
	MAP_SPACE_OPEN,				// MAP_OBJ_SIGN_SMALL_STONE,

	MAP_SPACE_OPEN,				// MAP_OBJ_SIGN_EMPTY, /* 120 */

	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_FIELD_0,
	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_FIELD_1,
	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_FIELD_2,
	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_FIELD_3,
	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_FIELD_4, /* 125 */
	MAP_SPACE_SEMIPASSABLE,		// MAP_OBJ_FIELD_5,
	MAP_SPACE_OPEN,				// MAP_OBJ_127
};


/* Return a random map position.
   Returned as map_pos_t and also as col and row if not NULL. */
static map_pos_t
get_rnd_map_coord(map_t *map, int *col, int *row)
{
	int c = map_random_int(map) & map->col_mask;
	int r = map_random_int(map) & map->row_mask;

	if (col != NULL) *col = c;
	if (row != NULL) *row = r;
	return MAP_POS2(c, r);
}

/* Midpoint displacement map generator. This function initialises the height
   values in the corners of 16x16 squares. */
static void
init_map_heights_squares(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	for (uint y = 0; y < map->rows; y += 16) {
		for (uint x = 0; x < map->cols; x += 16) {
			int rnd = map_random_int(map) & 0xff;
			tiles[MAP_POS2(x,y)].height = MIN(rnd, 250);
		}
	}
}

static int
calc_height_displacement(map_t *map, int avg, int base, int offset)
{
	int r = map_random_int(map);
	int h = ((r * base) >> 16) - offset + avg;

	return MAX(0,MIN(h,250));
}

#define TERRAIN_SPIKYNESS  0x9999

/* Calculate height values of the subdivisions in the
   midpoint displacement algorithm. */
static void
init_map_heights_midpoints(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	/* This is the central part of the midpoint displacement algorithm.
	   The initial 16x16 squares are subdivided into 8x8 then 4x4 and so on,
	   until all positions in the map have a height value.

	   The random offset applied to the midpoints is based on r1 and r2.
	   The offset is a random value in [-r2; r1-r2). r1 controls the roughness of
	   the terrain; a larger value of r1 will result in rough terrain while a smaller
	   value will generate smoother terrain.

	   A high spikyness will result in sharp mountains and smooth valleys. A low
	   spikyness will result in smooth mountains and sharp valleys.
	*/

	int rnd = map_random_int(map);
	int r1 = 0x80 + (rnd & 0x7f);
	int r2 = (r1 * TERRAIN_SPIKYNESS) >> 16;

	for (int i = 8; i > 0; i >>= 1) {
		for (uint y = 0; y < map->rows; y += 2*i) {
			for (uint x = 0; x < map->cols; x += 2*i) {
				map_pos_t pos = MAP_POS2(x, y);
				int h = tiles[pos].height;

				map_pos_t pos_r = MAP_MOVE_RIGHT_N2(pos, 2*i);
				map_pos_t pos_mid_r = MAP_MOVE_RIGHT_N2(pos, i);
				int h_r = tiles[pos_r].height;

				if (map->preserve_bugs) {
					/* The intention was probably just to set h_r to the map height value,
					   but the upper bits of rnd must be preserved in h_r in the first
					   iteration to generate the same maps as the original game. */
					if (x == 0 && y == 0 && i == 8) h_r |= rnd & 0xff00;
				}

				tiles[pos_mid_r].height = calc_height_displacement(map, (h + h_r)/2, r1, r2);

				map_pos_t pos_d = MAP_MOVE_DOWN_N2(pos, 2*i);
				map_pos_t pos_mid_d = MAP_MOVE_DOWN_N2(pos, i);
				int h_d = tiles[pos_d].height;
				tiles[pos_mid_d].height = calc_height_displacement(map, (h+h_d)/2, r1, r2);

				map_pos_t pos_dr = MAP_MOVE_RIGHT_N2(MAP_MOVE_DOWN_N2(pos, 2*i), 2*i);
				map_pos_t pos_mid_dr = MAP_MOVE_RIGHT_N2(MAP_MOVE_DOWN_N2(pos, i), i);
				int h_dr = tiles[pos_dr].height;
				tiles[pos_mid_dr].height = calc_height_displacement(map, (h+h_dr)/2, r1, r2);
			}
		}

		r1 >>= 1;
		r2 >>= 1;
	}
}

static void
init_map_heights_diamond_square(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	/* This is the central part of the diamond-square algorithm.
	   The squares are first subdivided into four new squares and
	   the height of the midpoint is calculated by averaging the corners and
	   adding a random offset. Each "diamond" that appears is then processed
	   in the same way.

	   The random offset applied to the midpoints is based on r1 and r2.
	   The offset is a random value in [-r2; r1-r2). r1 controls the roughness of
	   the terrain; a larger value of r1 will result in rough terrain while a smaller
	   value will generate smoother terrain.

	   A high spikyness will result in sharp mountains and smooth valleys. A low
	   spikyness will result in smooth mountains and sharp valleys.
	*/

	int rnd = map_random_int(map);
	int r1 = 0x80 + (rnd & 0x7f);
	int r2 = (r1 * TERRAIN_SPIKYNESS) >> 16;

	for (int i = 8; i > 0; i >>= 1) {
		/* Diamond step */
		for (uint y = 0; y < map->rows; y += 2*i) {
			for (uint x = 0; x < map->cols; x += 2*i) {
				map_pos_t pos = MAP_POS2(x, y);
				int h = tiles[pos].height;

				map_pos_t pos_r = MAP_MOVE_RIGHT_N2(pos, 2*i);
				int h_r = tiles[pos_r].height;

				map_pos_t pos_d = MAP_MOVE_DOWN_N2(pos, 2*i);
				int h_d = tiles[pos_d].height;

				map_pos_t pos_dr = MAP_MOVE_RIGHT_N2(MAP_MOVE_DOWN_N2(pos, 2*i), 2*i);
				int h_dr = tiles[pos_dr].height;

				map_pos_t pos_mid_dr = MAP_MOVE_RIGHT_N2(MAP_MOVE_DOWN_N2(pos, i), i);
				int avg = (h + h_r + h_d + h_dr) / 4;
				tiles[pos_mid_dr].height = calc_height_displacement(map, avg, r1, r2);
			}
		}

		/* Square step */
		for (uint y = 0; y < map->rows; y += 2*i) {
			for (uint x = 0; x < map->cols; x += 2*i) {
				map_pos_t pos = MAP_POS2(x, y);
				int h = tiles[pos].height;

				map_pos_t pos_r = MAP_MOVE_RIGHT_N2(pos, 2*i);
				int h_r = tiles[pos_r].height;

				map_pos_t pos_d = MAP_MOVE_DOWN_N2(pos, 2*i);
				int h_d = tiles[pos_d].height;

				map_pos_t pos_ur = MAP_MOVE_RIGHT_N2(MAP_MOVE_DOWN_N2(pos, -i), i);
				int h_ur = tiles[pos_ur].height;

				map_pos_t pos_dr = MAP_MOVE_RIGHT_N2(MAP_MOVE_DOWN_N2(pos, i), i);
				int h_dr = tiles[pos_dr].height;

				map_pos_t pos_dl = MAP_MOVE_RIGHT_N2(MAP_MOVE_DOWN_N2(pos, i), -i);
				int h_dl = tiles[pos_dl].height;

				map_pos_t pos_mid_r = MAP_MOVE_RIGHT_N2(pos, i);
				int avg_r = (h + h_r + h_ur + h_dr) / 4;
				tiles[pos_mid_r].height = calc_height_displacement(map, avg_r, r1, r2);

				map_pos_t pos_mid_d = MAP_MOVE_DOWN_N2(pos, i);
				int avg_d = (h + h_d + h_dl + h_dr) / 4;
				tiles[pos_mid_d].height = calc_height_displacement(map, avg_d, r1, r2);
			}
		}

		r1 >>= 1;
		r2 >>= 1;
	}
}

static int
adjust_map_height(int h1, int h2, map_tile_t *tile)
{
	if (abs(h1 - h2) > 32) {
		tile->height = h1 + ((h1 < h2) ? 32 : -32);
		return 1;
	}

	return 0;
}

/* Ensure that map heights of adjacent fields are not too far apart. */
static void
clamp_map_heights(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	int changed = 1;
	while (changed) {
		changed = 0;
		for (uint y = 0; y < map->rows; y++) {
			for (uint x = 0; x < map->cols; x++) {
				map_pos_t pos = MAP_POS2(x, y);
				int h = tiles[pos].height;

				map_pos_t pos_d = MAP_MOVE_DOWN2(pos);
				int h_d = tiles[pos_d].height;
				changed |= adjust_map_height(h, h_d, &tiles[pos_d]);

				map_pos_t pos_dr = MAP_MOVE_DOWN_RIGHT2(pos);
				int h_dr = tiles[pos_dr].height;
				changed |= adjust_map_height(h, h_dr, &tiles[pos_dr]);

				map_pos_t pos_r = MAP_MOVE_RIGHT2(pos);
				int h_r = tiles[pos_r].height;
				changed |= adjust_map_height(h, h_r, &tiles[pos_r]);
			}
		}
	}
}

static int
map_expand_level_area(map_t *map, map_tile_t *tiles, map_pos_t pos, int limit, int r)
{
	int flag = 0;

	for (int d = DIR_RIGHT; d <= DIR_UP; d++) {
		map_pos_t new_pos = MAP_MOVE2(pos, d);
		if (tiles[new_pos].height < 254) {
			if (tiles[new_pos].height > limit) return r;
		} else if (tiles[new_pos].height == 255) {
			flag = 1;
		}
	}

	if (flag) {
		tiles[pos].height = 255;

		for (int d = DIR_RIGHT; d <= DIR_UP; d++) {
			map_pos_t new_pos = MAP_MOVE2(pos, d);
			if (tiles[new_pos].height != 255) tiles[new_pos].height = 254;
		}

		return 1;
	}

	return r;
}

static void
map_init_level_area(map_t *map, map_tile_t *tiles, map_pos_t pos)
{
	int limit = map->water_level;

	if (limit >= tiles[MAP_MOVE_RIGHT2(pos)].height &&
	    limit >= tiles[MAP_MOVE_DOWN_RIGHT2(pos)].height &&
	    limit >= tiles[MAP_MOVE_DOWN2(pos)].height &&
	    limit >= tiles[MAP_MOVE_LEFT2(pos)].height &&
	    limit >= tiles[MAP_MOVE_UP_LEFT2(pos)].height &&
	    limit >= tiles[MAP_MOVE_UP2(pos)].height) {
		tiles[pos].height = 255;
		tiles[MAP_MOVE_RIGHT2(pos)].height = 254;
		tiles[MAP_MOVE_DOWN_RIGHT2(pos)].height = 254;
		tiles[MAP_MOVE_DOWN2(pos)].height = 254;
		tiles[MAP_MOVE_LEFT2(pos)].height = 254;
		tiles[MAP_MOVE_UP_LEFT2(pos)].height = 254;
		tiles[MAP_MOVE_UP2(pos)].height = 254;

		for (int i = 0; i < map->max_lake_area; i++) {
			int flag = 0;

			map_pos_t new_pos = MAP_MOVE_RIGHT_N2(pos, i+1);
			for (int k = 0; k < 6; k++) {
				dir_t d = (dir_t)((k + DIR_DOWN) % 6);
				for (int j = 0; j <= i; j++) {
					flag = map_expand_level_area(map, tiles, new_pos, limit, flag);
					new_pos = MAP_MOVE2(new_pos, d);
				}
			}

			if (!flag) break;
		}

		if (tiles[pos].height > 253) tiles[pos].height -= 2;

		for (int i = 0; i < map->max_lake_area + 1; i++) {
			map_pos_t new_pos = MAP_MOVE_RIGHT_N2(pos, i+1);
			for (int k = 0; k < 6; k++) {
				dir_t d = (dir_t)((k + DIR_DOWN) % 6);
				for (int j = 0; j <= i; j++) {
					if (tiles[new_pos].height > 253) tiles[new_pos].height -= 2;
					new_pos = MAP_MOVE2(new_pos, d);
				}
			}
		}
	} else {
		tiles[pos].height = 0;
	}
}

/* Create level land that will later be filled with water.
   It is created in areas that are below a certain threshold.
   The areas are also limited in size. */
static void
map_init_sea_level(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	if (map->water_level < 0) return;

	for (int h = 0; h <= map->water_level; h++) {
		for (uint y = 0; y < map->rows; y++) {
			for (uint x = 0; x < map->cols; x++) {
				map_pos_t pos = MAP_POS2(x, y);
				if (tiles[pos].height == h) {
					map_init_level_area(map, tiles, pos);
				}
			}
		}
	}

	/* Map positions are marked in the previous loop.
	   0: Above water level.
	   252: Land at water level.
	   253: Water. */

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);
			int h = tiles[pos].height;
			switch (h) {
				case 0:
					tiles[pos].height = map->water_level + 1;
					break;
				case 252:
					tiles[pos].height = (uint8_t)map->water_level;
					break;
				case 253:
					tiles[pos].height = map->water_level - 1;
					tiles[pos].resource = map_random_int(map) & 7; /* Fish */
					break;
			}
		}
	}
}

/* Adjust heights so zero height is sea level. */
static void
map_heights_rebase(map_t *map)
{
	map_tile_t *tiles = map->tiles;
	int h = map->water_level - 1;

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			tiles[MAP_POS2(x, y)].height -= h;
		}
	}
}

static int
calc_map_type(int h_sum)
{
	if (h_sum < 3) return 0;
	else if (h_sum < 384) return 5;
	else if (h_sum < 416) return 6;
	else if (h_sum < 448) return 11;
	else if (h_sum < 480) return 12;
	else if (h_sum < 528) return 13;
	else if (h_sum < 560) return 14;
	return 15;
}

/* Set type of map fields based on the height value. */
static void
init_map_types(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);
			int h1 = tiles[pos].height;
			int h2 = tiles[MAP_MOVE_RIGHT2(pos)].height;
			int h3 = tiles[MAP_MOVE_DOWN_RIGHT2(pos)].height;
			int h4 = tiles[MAP_MOVE_DOWN2(pos)].height;
			tiles[pos].type = (calc_map_type(h1 + h3 + h4) << 4) | calc_map_type(h1 + h2 + h3);
		}
	}
}

static void
init_map_types_2_sub(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			tiles[MAP_POS2(x, y)].obj = 0;
		}
	}
}

static void
init_map_types_2(map_t *map)
{
	init_map_types_2_sub(map);

	map_tile_t *tiles = map->tiles;

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);

			if (tiles[pos].height > 0) {
				tiles[pos].obj = 1;

				int num = 0;
				int changed = 1;
				while (changed) {
					changed = 0;
					for (uint y = 0; y < map->rows; y++) {
						for (uint x = 0; x < map->cols; x++) {
							map_pos_t pos = MAP_POS2(x, y);

							if (tiles[pos].obj == 1) {
								num += 1;
								tiles[pos].obj = 2;

								int flags = 0;
								if (tiles[pos].type & 0xc) flags |= 3;
								if (tiles[pos].type & 0xc0) flags |= 6;
								if (tiles[MAP_MOVE_LEFT2(pos)].type & 0xc) flags |= 0xc;
								if (tiles[MAP_MOVE_UP_LEFT2(pos)].type & 0xc0) flags |= 0x18;
								if (tiles[MAP_MOVE_UP_LEFT2(pos)].type & 0xc) flags |= 0x30;
								if (tiles[MAP_MOVE_UP2(pos)].type & 0xc0) flags |= 0x21;

								for (int d = DIR_RIGHT; d <= DIR_UP; d++) {
									if (BIT_TEST(flags, d)) {
										if (tiles[MAP_MOVE2(pos, d)].obj == 0) {
											tiles[MAP_MOVE2(pos, d)].obj = 1;
											changed = 1;
										}
									}
								}
							}
						}
					}
				}

				if (4*num >= (int)map->tile_count) goto break_loop;
			}
		}
	}

	break_loop:

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);

			if (tiles[pos].height > 0 && tiles[pos].obj == 0) {
				tiles[pos].height = 0;
				tiles[pos].type = 0;

				tiles[MAP_MOVE_LEFT2(pos)].type &= 0xf0;
				tiles[MAP_MOVE_UP_LEFT2(pos)].type = 0;
				tiles[MAP_MOVE_UP2(pos)].type &= 0xf;
			}
		}
	}

	init_map_types_2_sub(map);
}

/* Rescale height values to be in [0;31]. */
static void
map_heights_rescale(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);
			tiles[pos].height = (tiles[pos].height + 6) >> 3;
		}
	}
}

static void
init_map_types_shared_sub(map_t *map, int old, int seed, int new_)
{
	map_tile_t *tiles = map->tiles;

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);

			if (MAP_TYPE_UP2(pos) == old &&
			    (seed == MAP_TYPE_DOWN2(MAP_MOVE_UP_LEFT2(pos)) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_UP_LEFT2(pos)) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_UP2(pos)) ||
			     seed == MAP_TYPE_DOWN2(MAP_MOVE_LEFT2(pos)) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_LEFT2(pos)) ||
			     seed == MAP_TYPE_DOWN2(pos) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_RIGHT2(pos)) ||
			     seed == MAP_TYPE_DOWN2(MAP_MOVE_DOWN_LEFT2(pos)) ||
			     seed == MAP_TYPE_DOWN2(MAP_MOVE_DOWN2(pos)) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_DOWN2(pos)) ||
			     seed == MAP_TYPE_DOWN2(MAP_MOVE_DOWN_RIGHT2(pos)) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_DOWN_RIGHT2(pos)))) {
				tiles[pos].type = (new_ << 4) | (tiles[pos].type & 0xf);
			}

			if (MAP_TYPE_DOWN2(pos) == old &&
			    (seed == MAP_TYPE_DOWN2(MAP_MOVE_UP_LEFT2(pos)) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_UP_LEFT2(pos)) ||
			     seed == MAP_TYPE_DOWN2(MAP_MOVE_UP2(pos)) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_UP2(pos)) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_UP_RIGHT2(pos)) ||
			     seed == MAP_TYPE_DOWN2(MAP_MOVE_LEFT2(pos)) ||
			     seed == MAP_TYPE_UP2(pos) ||
			     seed == MAP_TYPE_DOWN2(MAP_MOVE_RIGHT2(pos)) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_RIGHT2(pos)) ||
			     seed == MAP_TYPE_DOWN2(MAP_MOVE_DOWN2(pos)) ||
			     seed == MAP_TYPE_DOWN2(MAP_MOVE_DOWN_RIGHT2(pos)) ||
			     seed == MAP_TYPE_UP2(MAP_MOVE_DOWN_RIGHT2(pos)))) {
				tiles[pos].type = (tiles[pos].type & 0xf0) | new_;
			}
		}
	}
}

static void
init_map_lakes(map_t *map)
{
	init_map_types_shared_sub(map, 0, 5, 3);
	init_map_types_shared_sub(map, 0, 3, 2);
	init_map_types_shared_sub(map, 0, 2, 1);
}

static void
init_map_types4(map_t *map)
{
	init_map_types_shared_sub(map, 5, 3, 4);
}

/* Use spiral pattern to lookup a new position based on col, row. */
static map_pos_t
lookup_pattern(map_t *map, int col, int row, int index)
{
	return MAP_POS_ADD2(MAP_POS2(col, row),
			   map->spiral_pos_pattern[index]);
}


static int
init_map_desert_sub1(map_t *map, map_pos_t pos)
{
	int type_d = MAP_TYPE_DOWN2(pos);
	int type_u = MAP_TYPE_UP2(pos);

	if (type_d != 5 && type_d != 10) return -1;
	if (type_u != 5 && type_u != 10) return -1;

	type_d = MAP_TYPE_DOWN2(MAP_MOVE_LEFT2(pos));
	if (type_d != 5 && type_d != 10) return -1;

	type_d = MAP_TYPE_DOWN2(MAP_MOVE_DOWN2(pos));
	if (type_d != 5 && type_d != 0xa) return -1;

	return 0;
}

static int
init_map_desert_sub2(map_t *map, map_pos_t pos)
{
	int type_d = MAP_TYPE_DOWN2(pos);
	int type_u = MAP_TYPE_UP2(pos);

	if (type_d != 5 && type_d != 10) return -1;
	if (type_u != 5 && type_u != 10) return -1;

	type_u = MAP_TYPE_UP2(MAP_MOVE_RIGHT2(pos));
	if (type_u != 5 && type_u != 10) return -1;

	type_u = MAP_TYPE_UP2(MAP_MOVE_UP2(pos));
	if (type_u != 5 && type_u != 10) return -1;

	return 0;
}

/* Create deserts on the map. */
static void
init_map_desert(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	for (int i = 0; i < map->regions; i++) {
		for (int try_ = 0; try_ < 200; try_++) {
			int col, row;
			map_pos_t rnd_pos = get_rnd_map_coord(map, &col, &row);

			if (MAP_TYPE_UP2(rnd_pos) == 5 &&
			    MAP_TYPE_DOWN2(rnd_pos) == 5) {
				for (int index = 255; index >= 0; index--) {
					map_pos_t pos = lookup_pattern(map, col, row, index);

					int r = init_map_desert_sub1(map, pos);
					if (r == 0) tiles[pos].type = (10 << 4) | (tiles[pos].type & 0xf);

					r = init_map_desert_sub2(map, pos);
					if (r == 0) tiles[pos].type = (tiles[pos].type & 0xf0) | 10;
				}
				break;
			}
		}
	}
}

static void
init_map_desert_2_sub(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);
			int type_d = MAP_TYPE_DOWN2(pos);
			int type_u = MAP_TYPE_UP2(pos);

			if (type_d >= 7 && type_d < 10) type_d = 5;
			if (type_u >= 7 && type_u < 10) type_u = 5;

			tiles[pos].type = (type_u << 4) | type_d;
		}
	}
}

static void
init_map_desert_2(map_t *map)
{
	init_map_types_shared_sub(map, 10, 5, 7);
	init_map_types_shared_sub(map, 10, 7, 8);
	init_map_types_shared_sub(map, 10, 8, 9);

	init_map_desert_2_sub(map);

	init_map_types_shared_sub(map, 5, 10, 9);
	init_map_types_shared_sub(map, 5, 9, 8);
	init_map_types_shared_sub(map, 5, 8, 7);
}

/* Put crosses on top of mountains. */
static void
init_map_crosses(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);
			uint h = MAP_HEIGHT2(pos);
			if (h >= 26 &&
			    h >= MAP_HEIGHT2(MAP_MOVE_RIGHT2(pos)) &&
			    h >= MAP_HEIGHT2(MAP_MOVE_DOWN_RIGHT2(pos)) &&
			    h >= MAP_HEIGHT2(MAP_MOVE_DOWN2(pos)) &&
			    h > MAP_HEIGHT2(MAP_MOVE_LEFT2(pos)) &&
			    h > MAP_HEIGHT2(MAP_MOVE_UP_LEFT2(pos)) &&
			    h > MAP_HEIGHT2(MAP_MOVE_UP2(pos))) {
				tiles[pos].obj = MAP_OBJ_CROSS;
			}
		}
	}
}

/* Check whether the hexagon at pos has triangles of types
   between min and max. Return -1 if not all triangles are
   in this range. */
static int
init_map_objects_shared_sub1(map_t *map, map_pos_t pos, int min, int max)
{
	int type_d = MAP_TYPE_DOWN2(pos);
	int type_u = MAP_TYPE_UP2(pos);

	if (type_d < min || type_d >= max) return -1;
	if (type_u < min || type_u >= max) return -1;

	type_d = MAP_TYPE_DOWN2(MAP_MOVE_LEFT2(pos));
	if (type_d < min || type_d >= max) return -1;

	type_d = MAP_TYPE_DOWN2(MAP_MOVE_UP_LEFT2(pos));
	type_u = MAP_TYPE_UP2(MAP_MOVE_UP_LEFT2(pos));
	if (type_d < min || type_d >= max) return -1;
	if (type_u < min || type_u >= max) return -1;

	/* Should be checkeing the up tri type. */
	if (map->preserve_bugs) {
		type_d = MAP_TYPE_DOWN2(MAP_MOVE_UP2(pos));
		if (type_d < min || type_d >= max) return -1;
	} else {
		type_u = MAP_TYPE_UP2(MAP_MOVE_UP2(pos));
		if (type_u < min || type_u >= max) return -1;
	}

	return 0;
}

/* Get a random position in the spiral pattern based at col, row. */
static map_pos_t
lookup_rnd_pattern(map_t *map, int col, int row, int mask)
{
	return lookup_pattern(map, col, row, map_random_int(map) & mask);
}

static void
init_map_objects_shared(map_t *map, int num_clusters, int objs_in_cluster, int pos_mask,
			int type_min, int type_max, int obj_base, int obj_mask)
{
	map_tile_t *tiles = map->tiles;

	for (int i = 0; i < num_clusters; i++) {
		for (int try_ = 0; try_ < 100; try_++) {
			int col, row;
			map_pos_t rnd_pos = get_rnd_map_coord(map, &col, &row);
			int r = init_map_objects_shared_sub1(map, rnd_pos, type_min, type_max);
			if (r == 0) {
				for (int j = 0; j < objs_in_cluster; j++) {
					map_pos_t pos = lookup_rnd_pattern(map, col, row, pos_mask);
					int r = init_map_objects_shared_sub1(map, pos, type_min, type_max);
					if (r == 0 && MAP_OBJ2(pos) == MAP_OBJ_NONE) {
						tiles[pos].obj = (map_random_int(map) & obj_mask) + obj_base;
					}
				}
				break;
			}
		}
	}
}

static void
init_map_trees_1(map_t *map)
{
	/* Add either tree or pine. */
	init_map_objects_shared(map, map->regions << 3, 10, 0xff, 5, 7, MAP_OBJ_TREE_0, 0xf);
}

static void
init_map_trees_2(map_t *map)
{
	/* Add only trees. */
	init_map_objects_shared(map, map->regions, 45, 0x3f, 5, 7, MAP_OBJ_TREE_0, 0x7);
}

static void
init_map_trees_3(map_t *map)
{
	/* Add only pines. */
	init_map_objects_shared(map, map->regions, 30, 0x3f, 4, 7, MAP_OBJ_PINE_0, 0x7);
}

static void
init_map_trees_4(map_t *map)
{
	/* Add either tree or pine. */
	init_map_objects_shared(map, map->regions, 20, 0x7f, 5, 7, MAP_OBJ_TREE_0, 0xf);
}

static void
init_map_stone_1(map_t *map)
{
	init_map_objects_shared(map, map->regions, 40, 0x3f, 5, 7, MAP_OBJ_STONE_0, 0x7);
}

static void
init_map_stone_2(map_t *map)
{
	init_map_objects_shared(map, map->regions, 15, 0xff, 5, 7, MAP_OBJ_STONE_0, 0x7);
}

static void
init_map_dead_trees(map_t *map)
{
	init_map_objects_shared(map, map->regions, 2, 0xff, 5, 7, MAP_OBJ_DEAD_TREE, 0);
}

static void
init_map_large_boulders(map_t *map)
{
	init_map_objects_shared(map, map->regions, 6, 0xff, 5, 7, MAP_OBJ_SANDSTONE_0, 0x1);
}

static void
init_map_water_trees(map_t *map)
{
	init_map_objects_shared(map, map->regions, 50, 0x7f, 2, 4, MAP_OBJ_WATER_TREE_0, 0x3);
}

static void
init_map_stubs(map_t *map)
{
	init_map_objects_shared(map, map->regions, 5, 0xff, 5, 7, MAP_OBJ_STUB, 0);
}

static void
init_map_small_boulders(map_t *map)
{
	init_map_objects_shared(map, map->regions, 10, 0xff, 5, 7, MAP_OBJ_STONE, 0x1);
}

static void
init_map_cadavers(map_t *map)
{
	init_map_objects_shared(map, map->regions, 2, 0xf, 10, 11, MAP_OBJ_CADAVER_0, 0x1);
}

static void
init_map_cacti(map_t *map)
{
	init_map_objects_shared(map, map->regions, 6, 0x7f, 8, 11, MAP_OBJ_CACTUS_0, 0x1);
}

static void
init_map_water_stones(map_t *map)
{
	init_map_objects_shared(map, map->regions, 8, 0x7f, 0, 3, MAP_OBJ_WATER_STONE_0, 0x1);
}

static void
init_map_palms(map_t *map)
{
	init_map_objects_shared(map, map->regions, 6, 0x3f, 10, 11, MAP_OBJ_PALM_0, 0x3);
}

static void
init_map_resources_shared_sub(map_t *map, map_tile_t *tiles, int iters, int col, int row, int *index,
			      int amount, ground_deposit_t type)
{
	for (int i = 0; i < iters; i++) {
		map_pos_t pos = lookup_pattern(map, col, row, *index);
		*index += 1;

		int res = tiles[pos].resource;
		if (res == 0 || (res & 0x1f) < amount) {
			tiles[pos].resource = (type << 5) + amount;
		}
	}
}

static void
init_map_resources_shared(map_t *map, int num_clusters, ground_deposit_t type, int min, int max)
{
	map_tile_t *tiles = map->tiles;

	for (int i = 0; i < num_clusters; i++) {
		for (int try_ = 0; try_ < 100; try_++) {
			int col, row;
			map_pos_t pos = get_rnd_map_coord(map, &col, &row);

			if (init_map_objects_shared_sub1(map, pos, min, max) == 0) {
				int index = 0;
				int amount = 8 + (map_random_int(map) & 0xc);
				init_map_resources_shared_sub(map, tiles, 1, col, row, &index, amount, type);
				amount -= 4;
				if (amount == 0) break;

				init_map_resources_shared_sub(map, tiles, 6, col, row, &index, amount, type);
				amount -= 4;
				if (amount == 0) break;

				init_map_resources_shared_sub(map, tiles, 12, col, row, &index, amount, type);
				amount -= 4;
				if (amount == 0) break;

				init_map_resources_shared_sub(map, tiles, 18, col, row, &index, amount, type);
				amount -= 4;
				if (amount == 0) break;

				init_map_resources_shared_sub(map, tiles, 24, col, row, &index, amount, type);
				amount -= 4;
				if (amount == 0) break;

				init_map_resources_shared_sub(map, tiles, 30, col, row, &index, amount, type);
				break;
			}
		}
	}
}

/* Initialize resources in the ground. */
static void
init_map_resources(map_t *map)
{
	init_map_resources_shared(map, map->regions * 9, GROUND_DEPOSIT_COAL, 11, 15);
	init_map_resources_shared(map, map->regions * 4, GROUND_DEPOSIT_IRON, 11, 15);
	init_map_resources_shared(map, map->regions * 2, GROUND_DEPOSIT_GOLD, 11, 15);
	init_map_resources_shared(map, map->regions * 2, GROUND_DEPOSIT_STONE, 11, 15);
}

static void
init_map_clean_up(map_t *map)
{
	map_tile_t *tiles = map->tiles;

	/* Make sure that it is always possible to walk around
	   any impassable objects. This also clears water obstacles
	   except in certain positions near the shore. */
	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);
			if (map_space_from_obj[MAP_OBJ2(pos)] >= MAP_SPACE_IMPASSABLE) {
				for (int d = DIR_LEFT; d <= DIR_UP; d++) {
					map_pos_t other_pos = MAP_MOVE2(pos, d);
					map_space_t s = map_space_from_obj[MAP_OBJ2(other_pos)];
					if (MAP_IN_WATER2(other_pos) ||
					    s >= MAP_SPACE_IMPASSABLE) {
						tiles[pos].obj &= 0x80;
						break;
					}
				}
			}
		}
	}
}

static void
init_map_sub(map_t *map)
{
	init_map_lakes(map);
	init_map_types4(map);
	init_map_desert(map);
	init_map_desert_2(map);
	init_map_crosses(map);
	init_map_trees_1(map);
	init_map_trees_2(map);
	init_map_trees_3(map);
	init_map_trees_4(map);
	init_map_stone_1(map);
	init_map_stone_2(map);
	init_map_dead_trees(map);
	init_map_large_boulders(map);
	init_map_water_trees(map);
	init_map_stubs(map);
	init_map_small_boulders(map);
	init_map_cadavers(map);
	init_map_cacti(map);
	init_map_water_stones(map);
	init_map_palms(map);
	init_map_resources(map);
	init_map_clean_up(map);
}

/* Initialize global count of gold deposits. */
static void
init_map_ground_gold_deposit(map_t *map)
{
	int total_gold = 0;

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);
			if (MAP_RES_TYPE2(pos) == GROUND_DEPOSIT_GOLD) {
				total_gold += MAP_RES_AMOUNT2(pos);
			}
		}
	}

	map->gold_deposit = total_gold;
}

/* Initialize minimap data. */
void
map_init_minimap(map_t *map)
{
	static const int color_offset[] = {
		0, 85, 102, 119, 17, 17, 17, 17,
		34, 34, 34, 51, 51, 51, 68, 68
	};

	static const int colors[] = {
		 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
		31, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
		63, 63, 62, 61, 61, 60, 59, 59, 58, 57, 57, 56, 55, 55, 54, 53, 53,
		61, 61, 60, 60, 59, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
		47, 47, 46, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
		 9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
		11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11
	};

	if (map->minimap != NULL) free(map->minimap);
	map->minimap = (uint8_t*)malloc(map->rows * map->cols);
	if (map->minimap == NULL) abort();

	map_tile_t *tiles = map->tiles;
	uint8_t *minimap = map->minimap;

	for (uint y = 0; y < map->rows; y++) {
		for (uint x = 0; x < map->cols; x++) {
			map_pos_t pos = MAP_POS2(x, y);
			int type_off = color_offset[tiles[pos].type >> 4];

			pos = MAP_MOVE_RIGHT2(pos);
			int h1 = MAP_HEIGHT2(pos);

			pos = MAP_MOVE_DOWN_LEFT2(pos);
			int h2 = MAP_HEIGHT2(pos);

			int h_off = h2 - h1 + 8;
			*(minimap++) = colors[type_off + h_off];
		}
	}
}

/* Set all map fields except cols/rows and col/row_size
   which must be set. */
void
map_init_dimensions(map_t *map)
{
	map->tile_count = map->cols * map->rows;

	map->col_mask = (1 << map->col_size) - 1;
	map->row_mask = (1 << map->row_size) - 1;
	map->row_shift = map->col_size;

	/* Setup direction offsets. */
	map->dirs[DIR_RIGHT] = 1 & map->col_mask;
	map->dirs[DIR_LEFT] = -1 & map->col_mask;
	map->dirs[DIR_DOWN] = (1 & map->row_mask) << map->row_shift;
	map->dirs[DIR_UP] = (-1 & map->row_mask) << map->row_shift;

	map->dirs[DIR_DOWN_RIGHT] = map->dirs[DIR_RIGHT] | map->dirs[DIR_DOWN];
	map->dirs[DIR_UP_RIGHT] = map->dirs[DIR_RIGHT] | map->dirs[DIR_UP];
	map->dirs[DIR_DOWN_LEFT] = map->dirs[DIR_LEFT] | map->dirs[DIR_DOWN];
	map->dirs[DIR_UP_LEFT] = map->dirs[DIR_LEFT] | map->dirs[DIR_UP];

	/* Allocate map */
	if (map->tiles != NULL) free(map->tiles);
	map->tiles = (map_tile_t*)calloc(map->tile_count, sizeof(map_tile_t));
	if (map->tiles == NULL) abort();
}

/* Initialize spiral_pos_pattern from spiral_pattern. */
static void
init_spiral_pos_pattern(map_t *map)
{
	if (map->spiral_pos_pattern == NULL) {
		map->spiral_pos_pattern = (map_pos_t*)malloc(295*sizeof(map_pos_t));
		if (map->spiral_pos_pattern == NULL) abort();
	}

	for (int i = 0; i < 295; i++) {
		int x = spiral_pattern[2*i] & map->col_mask;
		int y = spiral_pattern[2*i+1] & map->row_mask;

		map->spiral_pos_pattern[i] = MAP_POS2(x, y);
	}
}

void
map_init(map_t *map, uint size)
{
	/* Initialize global lookup tables */
	init_spiral_pattern();

	map->minimap = NULL;
	map->spiral_pos_pattern = NULL;
	map->tiles = NULL;

	map->max_lake_area = 14;
	map->water_level = 20;

	map->update_map_last_tick = 0;
	map->update_map_counter = 0;
	map->update_map_16_loop = 0;
	map->update_map_initial_pos = 0;

	map->size = size;

	map->col_size = 5 + map->size/2;
	map->row_size = 5 + (map->size - 1)/2;
	map->cols = 1 << map->col_size;
	map->rows = 1 << map->row_size;

	map_init_dimensions(map);

	map->regions = (map->cols >> 5) * (map->rows >> 5);

	init_spiral_pos_pattern(map);
}

void
map_generate(map_t *map, int generator, random_state_t *rnd)
{
	map->rnd = rnd;

	map_random_int(map);
	map_random_int(map);

	init_map_heights_squares(map);
	switch (generator) {
	case 0:
		init_map_heights_midpoints(map); /* Midpoint displacement algorithm */
		break;
	case 1:
		init_map_heights_diamond_square(map); /* Diamond square algorithm */
		break;
	}

	clamp_map_heights(map);
	map_init_sea_level(map);
	map_heights_rebase(map);
	init_map_types(map);
	init_map_types_2(map);
	map_heights_rescale(map);
	init_map_sub(map);
	init_map_ground_gold_deposit(map);

	map_init_minimap(map);
}

void
map_deinit(map_t *map)
{
	if (map->tiles != NULL) {
		free(map->tiles);
		map->tiles = NULL;
	}

	if (map->minimap != NULL) {
		free(map->minimap);
		map->minimap = NULL;
	}
}

/* Change the height of a map position. */
void
map_set_height(map_t *map, map_pos_t pos, int height)
{
	map_tile_t *tiles = map->tiles;
	tiles[pos].height = (tiles[pos].height & 0xe0) | (height & 0x1f);

	/* Mark landscape dirty */
	if (map->update_map_height_cb != NULL) {
		for (int d = DIR_RIGHT; d <= DIR_UP; d++) {
			map->update_map_height_cb(MAP_MOVE2(pos, d),
						  map->update_map_height_data);
		}
	}
}

/* Change the object at a map position. If index is non-negative
   also change this. The index should be reset to zero when a flag or
   building is removed. */
void
map_set_object(map_t *map, map_pos_t pos, map_obj_t obj, int index)
{
	map_tile_t *tiles = map->tiles;
	tiles[pos].obj = (tiles[pos].obj & 0x80) | (obj & 0x7f);
	if (index >= 0) tiles[pos].obj_index = index;
}

/* Remove resources from the ground at a map position. */
void
map_remove_ground_deposit(map_t *map, map_pos_t pos, int amount)
{
	map_tile_t *tiles = map->tiles;
	tiles[pos].resource -= amount;

	if (MAP_RES_AMOUNT2(pos) == 0) {
		/* Also sets the ground deposit type to none. */
		tiles[pos].resource = 0;
	}
}

/* Remove fish at a map position (must be water). */
void
map_remove_fish(map_t *map, map_pos_t pos, int amount)
{
	map_tile_t *tiles = map->tiles;
	tiles[pos].resource -= amount;
}

/* Set the index of the serf occupying map position. */
void
map_set_serf_index(map_t *map, map_pos_t pos, int index)
{
	map_tile_t *tiles = map->tiles;
	tiles[pos].serf = index;

	/* TODO Mark dirty in viewport. */
}

/* Update public parts of the map data. */
static void
map_update_public(map_t *map, map_pos_t pos)
{
	/* Update other map objects */
	int r;
	switch (MAP_OBJ2(pos)) {
	case MAP_OBJ_STUB:
		if ((map_random_int(map) & 3) == 0) map_set_object(map, pos, MAP_OBJ_NONE, -1);
		break;
	case MAP_OBJ_FELLED_PINE_0: case MAP_OBJ_FELLED_PINE_1:
	case MAP_OBJ_FELLED_PINE_2: case MAP_OBJ_FELLED_PINE_3:
	case MAP_OBJ_FELLED_PINE_4:
	case MAP_OBJ_FELLED_TREE_0: case MAP_OBJ_FELLED_TREE_1:
	case MAP_OBJ_FELLED_TREE_2: case MAP_OBJ_FELLED_TREE_3:
	case MAP_OBJ_FELLED_TREE_4:
		map_set_object(map, pos, MAP_OBJ_STUB, -1);
		break;
	case MAP_OBJ_NEW_PINE:
		r = map_random_int(map);
		if ((r & 0x300) == 0) map_set_object(map, pos, (map_obj_t)(MAP_OBJ_PINE_0 + (r & 7)), -1);
		break;
	case MAP_OBJ_NEW_TREE:
		r = map_random_int(map);
		if ((r & 0x300) == 0) map_set_object(map, pos, (map_obj_t)(MAP_OBJ_TREE_0 + (r & 7)), -1);
		break;
	case MAP_OBJ_SEEDS_0: case MAP_OBJ_SEEDS_1:
	case MAP_OBJ_SEEDS_2: case MAP_OBJ_SEEDS_3:
	case MAP_OBJ_SEEDS_4:
	case MAP_OBJ_FIELD_0: case MAP_OBJ_FIELD_1:
	case MAP_OBJ_FIELD_2: case MAP_OBJ_FIELD_3:
	case MAP_OBJ_FIELD_4:
		map_set_object(map, pos, (map_obj_t)(MAP_OBJ2(pos) + 1), -1);
		break;
	case MAP_OBJ_SEEDS_5:
		map_set_object(map, pos, MAP_OBJ_FIELD_0, -1);
		break;
	case MAP_OBJ_FIELD_EXPIRED:
		map_set_object(map, pos, MAP_OBJ_NONE, -1);
		break;
	case MAP_OBJ_SIGN_LARGE_GOLD: case MAP_OBJ_SIGN_SMALL_GOLD:
	case MAP_OBJ_SIGN_LARGE_IRON: case MAP_OBJ_SIGN_SMALL_IRON:
	case MAP_OBJ_SIGN_LARGE_COAL: case MAP_OBJ_SIGN_SMALL_COAL:
	case MAP_OBJ_SIGN_LARGE_STONE: case MAP_OBJ_SIGN_SMALL_STONE:
	case MAP_OBJ_SIGN_EMPTY:
		if (map->update_map_16_loop == 0) {
			map_set_object(map, pos, MAP_OBJ_NONE, -1);
		}
		break;
	case MAP_OBJ_FIELD_5:
		map_set_object(map, pos, MAP_OBJ_FIELD_EXPIRED, -1);
		break;
	default:
		break;
	}
}

/* Update hidden parts of the map data. */
static void
map_update_hidden(map_t *map, map_pos_t pos)
{
	map_tile_t *tiles = map->tiles;

	/* Update fish resources in water */
	if (MAP_IN_WATER2(pos) &&
	    tiles[pos].resource > 0) {
		int r = map_random_int(map);

		if (tiles[pos].resource < 10 && (r & 0x3f00)) {
			/* Spawn more fish. */
			tiles[pos].resource += 1;
		}

		/* Move in a random direction of: right, down right, left, up left */
		map_pos_t adj_pos = pos;
		switch ((r >> 2) & 3) {
		case 0: adj_pos = MAP_MOVE_RIGHT2(adj_pos); break;
		case 1: adj_pos = MAP_MOVE_DOWN_RIGHT2(adj_pos); break;
		case 2: adj_pos = MAP_MOVE_LEFT2(adj_pos); break;
		case 3: adj_pos = MAP_MOVE_UP_LEFT2(adj_pos); break;
		default: NOT_REACHED(); break;
		}

		if (MAP_IN_WATER2(adj_pos)) {
			/* Migrate a fish to adjacent water space. */
			tiles[pos].resource -= 1;
			tiles[adj_pos].resource += 1;
		}
	}
}

/* Update map data as part of the game progression. */
void
map_update(map_t *map, uint tick)
{
	uint16_t delta = tick - map->update_map_last_tick;
	map->update_map_last_tick = tick;
	map->update_map_counter -= delta;

	int iters = 0;
	while (map->update_map_counter < 0) {
		iters += map->regions;
		map->update_map_counter += 20;
	}

	map_pos_t pos = map->update_map_initial_pos;

	for (int i = 0; i < iters; i++) {
		map->update_map_16_loop -= 1;
		if (map->update_map_16_loop < 0) map->update_map_16_loop = 16;

		/* Test if moving 23 positions right crosses map boundary. */
		if (MAP_POS_COL2(pos) + 23 < map->cols) {
			pos = MAP_MOVE_RIGHT_N2(pos, 23);
		} else {
			pos = MAP_MOVE_RIGHT_N2(pos, 23);
			pos = MAP_MOVE_DOWN2(pos);
		}

		/* Update map at position. */
		map_update_hidden(map, pos);
		map_update_public(map, pos);
	}

	map->update_map_initial_pos = pos;
}

uint16_t
map_random_int(map_t *map)
{
	return random_int(map->rnd);
}
