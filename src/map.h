/*
 * map.h - Map generators and map update functions
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

#ifndef _MAP_H
#define _MAP_H

#include "misc.h"
#include "random.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

typedef enum {
	DIR_NONE = -1,

	DIR_RIGHT = 0,
	DIR_DOWN_RIGHT,
	DIR_DOWN,
	DIR_LEFT,
	DIR_UP_LEFT,
	DIR_UP,

	DIR_UP_RIGHT,
	DIR_DOWN_LEFT
} dir_t;

#define DIR_REVERSE(dir)  ((dir_t)(((dir) + 3) % 6))

/* Extract col and row from map_pos_t */
#define MAP_POS_COL(pos)  ((pos) & game.map.col_mask)
#define MAP_POS_COL2(pos)  ((pos) & map->col_mask)
#define MAP_POS_ROW(pos)  (((pos)>>game.map.row_shift) & game.map.row_mask)
#define MAP_POS_ROW2(pos)  (((pos)>>map->row_shift) & map->row_mask)

/* Translate col, row coordinate to map_pos_t value. */
#define MAP_POS(x,y)  (((y)<<game.map.row_shift) | (x))
#define MAP_POS2(x,y)  (((y)<<map->row_shift) | (x))

/* Addition of two map positions. */
#define MAP_POS_ADD(pos,off)  MAP_POS(((MAP_POS_COL(pos) + MAP_POS_COL(off)) & game.map.col_mask), \
				      ((MAP_POS_ROW(pos) + MAP_POS_ROW(off)) & game.map.row_mask))
#define MAP_POS_ADD2(pos,off)  MAP_POS2(((MAP_POS_COL2(pos) + MAP_POS_COL2(off)) & map->col_mask), \
				      ((MAP_POS_ROW2(pos) + MAP_POS_ROW2(off)) & map->row_mask))

/* Movement of map position according to directions. */
#define MAP_MOVE(pos,dir)  MAP_POS_ADD((pos), game.map.dirs[(dir)])
#define MAP_MOVE2(pos,dir)  MAP_POS_ADD2((pos), map->dirs[(dir)])

#define MAP_MOVE_RIGHT(pos)  MAP_MOVE((pos), DIR_RIGHT)
#define MAP_MOVE_RIGHT2(pos)  MAP_MOVE2((pos), DIR_RIGHT)
#define MAP_MOVE_DOWN_RIGHT(pos)  MAP_MOVE((pos), DIR_DOWN_RIGHT)
#define MAP_MOVE_DOWN_RIGHT2(pos)  MAP_MOVE2((pos), DIR_DOWN_RIGHT)
#define MAP_MOVE_DOWN(pos)  MAP_MOVE((pos), DIR_DOWN)
#define MAP_MOVE_DOWN2(pos)  MAP_MOVE2((pos), DIR_DOWN)
#define MAP_MOVE_LEFT(pos)  MAP_MOVE((pos), DIR_LEFT)
#define MAP_MOVE_LEFT2(pos)  MAP_MOVE2((pos), DIR_LEFT)
#define MAP_MOVE_UP_LEFT(pos)  MAP_MOVE((pos), DIR_UP_LEFT)
#define MAP_MOVE_UP_LEFT2(pos)  MAP_MOVE2((pos), DIR_UP_LEFT)
#define MAP_MOVE_UP(pos)  MAP_MOVE((pos), DIR_UP)
#define MAP_MOVE_UP2(pos)  MAP_MOVE2((pos), DIR_UP)

#define MAP_MOVE_UP_RIGHT(pos)  MAP_MOVE((pos), DIR_UP_RIGHT)
#define MAP_MOVE_UP_RIGHT2(pos)  MAP_MOVE2((pos), DIR_UP_RIGHT)
#define MAP_MOVE_DOWN_LEFT(pos)  MAP_MOVE((pos), DIR_DOWN_LEFT)
#define MAP_MOVE_DOWN_LEFT2(pos)  MAP_MOVE2((pos), DIR_DOWN_LEFT)

#define MAP_MOVE_RIGHT_N(pos,n)  MAP_POS_ADD((pos), game.map.dirs[DIR_RIGHT]*(n))
#define MAP_MOVE_RIGHT_N2(pos,n)  MAP_POS_ADD2((pos), map->dirs[DIR_RIGHT]*(n))
#define MAP_MOVE_DOWN_N(pos,n)  MAP_POS_ADD((pos), game.map.dirs[DIR_DOWN]*(n))
#define MAP_MOVE_DOWN_N2(pos,n)  MAP_POS_ADD2((pos), map->dirs[DIR_DOWN]*(n))


/* Extractors for map data. */
#define MAP_PATHS(pos)  ((uint)(game.map.tiles[(pos)].paths & 0x3f))
#define MAP_PATHS2(pos)  ((uint)(map->tiles[(pos)].paths & 0x3f))

#define MAP_HAS_OWNER(pos)  ((uint)((game.map.tiles[(pos)].height >> 7) & 1))
#define MAP_HAS_OWNER2(pos)  ((uint)((map->tiles[(pos)].height >> 7) & 1))
#define MAP_OWNER(pos)  ((uint)((game.map.tiles[(pos)].height >> 5) & 3))
#define MAP_OWNER2(pos)  ((uint)((map->tiles[(pos)].height >> 5) & 3))
#define MAP_HEIGHT(pos)  ((uint)(game.map.tiles[(pos)].height & 0x1f))
#define MAP_HEIGHT2(pos)  ((uint)(map->tiles[(pos)].height & 0x1f))

#define MAP_TYPE_UP(pos)  ((uint)((game.map.tiles[(pos)].type >> 4) & 0xf))
#define MAP_TYPE_UP2(pos)  ((uint)((map->tiles[(pos)].type >> 4) & 0xf))
#define MAP_TYPE_DOWN(pos)  ((uint)(game.map.tiles[(pos)].type & 0xf))
#define MAP_TYPE_DOWN2(pos)  ((uint)(map->tiles[(pos)].type & 0xf))

#define MAP_OBJ(pos)  ((map_obj_t)(game.map.tiles[(pos)].obj & 0x7f))
#define MAP_OBJ2(pos)  ((map_obj_t)(map->tiles[(pos)].obj & 0x7f))
#define MAP_IDLE_SERF(pos)  ((uint)((game.map.tiles[(pos)].obj >> 7) & 1))
#define MAP_IDLE_SERF2(pos)  ((uint)((map->tiles[(pos)].obj >> 7) & 1))

#define MAP_OBJ_INDEX(pos)  ((uint)game.map.tiles[(pos)].obj_index)
#define MAP_OBJ_INDEX2(pos)  ((uint)map->tiles[(pos)].obj_index)
#define MAP_RES_TYPE(pos)  ((ground_deposit_t)((game.map.tiles[(pos)].resource >> 5) & 7))
#define MAP_RES_TYPE2(pos)  ((ground_deposit_t)((map->tiles[(pos)].resource >> 5) & 7))
#define MAP_RES_AMOUNT(pos)  ((uint)(game.map.tiles[(pos)].resource & 0x1f))
#define MAP_RES_AMOUNT2(pos)  ((uint)(map->tiles[(pos)].resource & 0x1f))
#define MAP_RES_FISH(pos)  ((uint)game.map.tiles[(pos)].resource)
#define MAP_SERF_INDEX(pos)  ((uint)game.map.tiles[(pos)].serf)


#define MAP_HAS_FLAG(pos)  (MAP_OBJ(pos) == MAP_OBJ_FLAG)
#define MAP_HAS_FLAG2(pos)  (MAP_OBJ2(pos) == MAP_OBJ_FLAG)
#define MAP_HAS_BUILDING(pos)  (MAP_OBJ(pos) >= MAP_OBJ_SMALL_BUILDING && \
				MAP_OBJ(pos) <= MAP_OBJ_CASTLE)



/* Whether any of the two up/down tiles at this pos are water. */
#define MAP_WATER_TILE(pos)				\
	(MAP_TYPE_DOWN(pos) < 4 &&			\
	 MAP_TYPE_UP(pos) < 4)
#define MAP_WATER_TILE2(pos)				\
	(MAP_TYPE_DOWN2(pos) < 4 &&			\
	MAP_TYPE_UP2(pos) < 4)

/* Whether the position is completely surrounded by water. */
#define MAP_IN_WATER(pos)				\
	(MAP_WATER_TILE(pos) &&				\
	 MAP_WATER_TILE(MAP_MOVE_UP_LEFT(pos)) &&	\
	 MAP_TYPE_DOWN(MAP_MOVE_LEFT(pos)) < 4 &&	\
	 MAP_TYPE_UP(MAP_MOVE_UP(pos)) < 4)
#define MAP_IN_WATER2(pos)				\
	(MAP_WATER_TILE2(pos) &&				\
	MAP_WATER_TILE2(MAP_MOVE_UP_LEFT2(pos)) &&	\
	MAP_TYPE_DOWN2(MAP_MOVE_LEFT2(pos)) < 4 &&	\
	MAP_TYPE_UP2(MAP_MOVE_UP2(pos)) < 4)


typedef enum {
	MAP_OBJ_NONE = 0,
	MAP_OBJ_FLAG,
	MAP_OBJ_SMALL_BUILDING,
	MAP_OBJ_LARGE_BUILDING,
	MAP_OBJ_CASTLE,

	MAP_OBJ_TREE_0 = 8,
	MAP_OBJ_TREE_1,
	MAP_OBJ_TREE_2, /* 10 */
	MAP_OBJ_TREE_3,
	MAP_OBJ_TREE_4,
	MAP_OBJ_TREE_5,
	MAP_OBJ_TREE_6,
	MAP_OBJ_TREE_7, /* 15 */

	MAP_OBJ_PINE_0,
	MAP_OBJ_PINE_1,
	MAP_OBJ_PINE_2,
	MAP_OBJ_PINE_3,
	MAP_OBJ_PINE_4, /* 20 */
	MAP_OBJ_PINE_5,
	MAP_OBJ_PINE_6,
	MAP_OBJ_PINE_7,

	MAP_OBJ_PALM_0,
	MAP_OBJ_PALM_1, /* 25 */
	MAP_OBJ_PALM_2,
	MAP_OBJ_PALM_3,

	MAP_OBJ_WATER_TREE_0,
	MAP_OBJ_WATER_TREE_1,
	MAP_OBJ_WATER_TREE_2, /* 30 */
	MAP_OBJ_WATER_TREE_3,

	MAP_OBJ_STONE_0 = 72,
	MAP_OBJ_STONE_1,
	MAP_OBJ_STONE_2,
	MAP_OBJ_STONE_3, /* 75 */
	MAP_OBJ_STONE_4,
	MAP_OBJ_STONE_5,
	MAP_OBJ_STONE_6,
	MAP_OBJ_STONE_7,

	MAP_OBJ_SANDSTONE_0, /* 80 */
	MAP_OBJ_SANDSTONE_1,

	MAP_OBJ_CROSS,
	MAP_OBJ_STUB,

	MAP_OBJ_STONE,
	MAP_OBJ_SANDSTONE_3, /* 85 */

	MAP_OBJ_CADAVER_0,
	MAP_OBJ_CADAVER_1,

	MAP_OBJ_WATER_STONE_0,
	MAP_OBJ_WATER_STONE_1,

	MAP_OBJ_CACTUS_0, /* 90 */
	MAP_OBJ_CACTUS_1,

	MAP_OBJ_DEAD_TREE,

	MAP_OBJ_FELLED_PINE_0,
	MAP_OBJ_FELLED_PINE_1,
	MAP_OBJ_FELLED_PINE_2, /* 95 */
	MAP_OBJ_FELLED_PINE_3,
	MAP_OBJ_FELLED_PINE_4,

	MAP_OBJ_FELLED_TREE_0,
	MAP_OBJ_FELLED_TREE_1,
	MAP_OBJ_FELLED_TREE_2, /* 100 */
	MAP_OBJ_FELLED_TREE_3,
	MAP_OBJ_FELLED_TREE_4,

	MAP_OBJ_NEW_PINE,
	MAP_OBJ_NEW_TREE,

	MAP_OBJ_SEEDS_0, /* 105 */
	MAP_OBJ_SEEDS_1,
	MAP_OBJ_SEEDS_2,
	MAP_OBJ_SEEDS_3,
	MAP_OBJ_SEEDS_4,
	MAP_OBJ_SEEDS_5, /* 110 */
	MAP_OBJ_FIELD_EXPIRED,

	MAP_OBJ_SIGN_LARGE_GOLD,
	MAP_OBJ_SIGN_SMALL_GOLD,
	MAP_OBJ_SIGN_LARGE_IRON,
	MAP_OBJ_SIGN_SMALL_IRON, /* 115 */
	MAP_OBJ_SIGN_LARGE_COAL,
	MAP_OBJ_SIGN_SMALL_COAL,
	MAP_OBJ_SIGN_LARGE_STONE,
	MAP_OBJ_SIGN_SMALL_STONE,

	MAP_OBJ_SIGN_EMPTY, /* 120 */

	MAP_OBJ_FIELD_0,
	MAP_OBJ_FIELD_1,
	MAP_OBJ_FIELD_2,
	MAP_OBJ_FIELD_3,
	MAP_OBJ_FIELD_4, /* 125 */
	MAP_OBJ_FIELD_5,
	MAP_OBJ_127
} map_obj_t;


/* A map space can be OPEN which means that
   a building can be constructed in the space.
   A FILLED space can be passed by a serf, but
   nothing can be built in this space except roads.
   A SEMIPASSABLE space is like FILLED but no roads
   can be built. A IMPASSABLE space can neither be
   used for contructions nor passed by serfs. */
typedef enum {
	MAP_SPACE_OPEN = 0,
	MAP_SPACE_FILLED,
	MAP_SPACE_SEMIPASSABLE,
	MAP_SPACE_IMPASSABLE,
} map_space_t;

typedef enum {
	GROUND_DEPOSIT_NONE = 0,
	GROUND_DEPOSIT_GOLD,
	GROUND_DEPOSIT_IRON,
	GROUND_DEPOSIT_COAL,
	GROUND_DEPOSIT_STONE,
} ground_deposit_t;

typedef struct {
	uint8_t paths;
	uint8_t height;
	uint8_t type;
	uint8_t obj;
	uint16_t obj_index;
	uint8_t resource;
	uint16_t serf;
} map_tile_t;

/* map_pos_t is a compact composition of col and row values that
   uniquely identifies a vertex in the map space. It is also used
   directly as index to map data arrays. */
typedef uint map_pos_t;

typedef void game_update_map_height_func(map_pos_t pos, void *data);

typedef struct {
	/* Fundamentals */
	uint16_t size;
	map_tile_t *tiles;
	uint col_size, row_size;

	/* Derived */
	map_pos_t dirs[8];
	uint tile_count;
	uint cols, rows;
	uint col_mask, row_mask;
	uint row_shift;

	uint8_t *minimap;

	int16_t water_level;
	int16_t max_lake_area;

	int preserve_bugs;

	uint16_t regions;

	uint32_t gold_deposit;

	random_state_t rnd;

	int16_t update_map_16_loop;
	uint16_t update_map_last_tick;
	int16_t update_map_counter;
	map_pos_t update_map_initial_pos;

	/* Callback for map height changes */
	game_update_map_height_func *update_map_height_cb;
	void *update_map_height_data;

	map_pos_t *spiral_pos_pattern;
} map_t;


/* Mapping from map_obj_t to map_space_t. */
extern const map_space_t map_space_from_obj[128];


void map_set_height(map_t *map, map_pos_t pos, int height);
void map_set_object(map_t *map, map_pos_t pos, map_obj_t obj, int index);
void map_remove_ground_deposit(map_t *map, map_pos_t pos, int amount);
void map_remove_fish(map_t *map, map_pos_t pos, int amount);
void map_set_serf_index(map_t *map, map_pos_t pos, int index);

void map_init_dimensions(map_t *map);
void map_init_minimap(map_t *map);

void map_init(map_t *map, uint size);
void map_generate(map_t *map, int generator, random_state_t *rnd);
void map_deinit(map_t *map);
void map_update(map_t *map, uint tick);

int map_road_segment_valid(map_pos_t pos, dir_t dir, map_t *map);

uint16_t map_random_int(map_t *map);

int *get_spiral_pattern();

#endif /* _MAP_H */
