/*
 * ai_road.c - Computer player road builder, ported from the Amiga original
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

/* The road builder works on a 19x19 grid of map positions centered on
   the AI cursor (grid index = row * 19 + col, center 180). Each cell
   holds a byte:
     0x00..0x3f  cost of walking over the cell,
     0x40..0x4f  target: a flag (category 0),
     0x50..0x7e  target: a path tile or shore tile where a flag can
                 be built (category 1),
     0x7f        blocked, 0xff unreachable / not evaluated.
   A layered shortest path search (at most 21 steps) records the best
   path to each target; the roads are then built in order of cost. */

#include "ai_internal.h"
#include "misc.h"
#include "log.h"

#include <string.h>

#define GRID_SIZE    19
#define GRID_CELLS   (GRID_SIZE*GRID_SIZE)
#define GRID_CENTER  180
#define MAX_TARGETS  63

/* Amiga player->build bit 4: build water roads. */
#define AI_BUILD_WATER(player)  (((player)->build >> 4) & 1)

/* Grid index offsets of the six directions (right, down right,
   down, left, up left, up). */
static const int grid_dir_offset[6] = { 1, 20, 19, -1, -20, -19 };

/* Extra cost around own farms (7x7, @0x2b152) and foresters
   (13x13, @0x2b183). */
static const uint8_t cost_farm[7*7] = {
	7, 7, 7, 7, 0, 0, 0,
	7, 8, 8, 8, 7, 0, 0,
	7, 8, 9, 9, 8, 7, 0,
	7, 8, 9, 9, 9, 8, 7,
	0, 7, 8, 9, 9, 8, 7,
	0, 0, 7, 8, 8, 8, 7,
	0, 0, 0, 7, 7, 7, 7
};

static const uint8_t cost_forester[13*13] = {
	4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0,
	4, 5, 5, 5, 5, 5, 5, 4, 0, 0, 0, 0, 0,
	4, 5, 6, 6, 6, 6, 6, 5, 4, 0, 0, 0, 0,
	4, 5, 6, 7, 7, 7, 7, 6, 5, 4, 0, 0, 0,
	4, 5, 6, 7, 8, 8, 8, 7, 6, 5, 4, 0, 0,
	4, 5, 6, 7, 8, 9, 9, 8, 7, 6, 5, 4, 0,
	4, 5, 6, 7, 8, 9, 9, 9, 8, 7, 6, 5, 4,
	0, 4, 5, 6, 7, 8, 9, 9, 8, 7, 6, 5, 4,
	0, 0, 4, 5, 6, 7, 8, 8, 8, 7, 6, 5, 4,
	0, 0, 0, 4, 5, 6, 7, 7, 7, 7, 6, 5, 4,
	0, 0, 0, 0, 4, 5, 6, 6, 6, 6, 6, 5, 4,
	0, 0, 0, 0, 0, 4, 5, 5, 5, 5, 5, 5, 4,
	0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4
};

/* Work tables of ai_build_road (Amiga: inside game->land_influence,
   the cost grid itself is ai_game.land_influence[0..360]). */
static map_pos_t grid_pos[GRID_CELLS];		/* +0x16c */
static uint16_t grid_best[GRID_CELLS];		/* +0x710 */
static uint64_t target_path[MAX_TARGETS+1];	/* +0x9e2 */
static uint16_t target_cost[MAX_TARGETS+1];	/* +0xbda */

/* Search queue entry (Amiga 12 bytes at game+0x120/0x124). The path is
   a list of 3-bit direction codes (dir + 1), first step in the most
   significant used group. */
typedef struct {
	uint16_t cost;
	int index;
	uint64_t path;
} road_queue_t;

#define ROAD_QUEUE_SIZE  (GRID_CELLS*6)
static road_queue_t road_queue[2][ROAD_QUEUE_SIZE];

#define FLAG_QUEUE_SIZE  1024
static flag_t *flag_queue[2][FLAG_QUEUE_SIZE];


/* Move the AI cursor to pos and evaluate it (determine_map_cursor_type
   @0x19368 with ptr+0xfc/0xfe). */
static void
ai_cursor_at(player_t *player, map_pos_t pos)
{
	AI_SET_CURSOR(player, pos);
	ai_determine_map_cursor_type(player);
}

/* Amiga obj bit 7 (map_mark_water_tiles): the vertex' own up or down
   triangle is water. legacy keeps no marker. */
static int
ai_water_marker(map_pos_t pos)
{
	return MAP_TYPE_UP(pos) < 4 || MAP_TYPE_DOWN(pos) < 4;
}

/* Amiga paths bit 6 ("blocked": lake water, kept impassable objects,
   buildings). */
static int
ai_tile_blocked(map_pos_t pos)
{
	return MAP_BLOCKED(pos);
}

static int
ai_owned_by(map_pos_t pos, const player_t *player)
{
	return MAP_HAS_OWNER(pos) && MAP_OWNER(pos) == player->player_num;
}

/* ai_road_tile_cost @0x2b4d8: evaluate grid cell index at map pos in
   ring ring. Clears *found when the cell became passable. */
static void
ai_road_tile_cost(player_t *player, int index, map_pos_t pos, int ring,
		  int *found)
{
	uint8_t *grid = ai_game.land_influence;

	/* Only cells next to an already passable cell. */
	if (grid[index-1] >= 0x40 && grid[index-19] >= 0x40 &&
	    grid[index-20] >= 0x40 && grid[index+1] >= 0x40 &&
	    grid[index+19] >= 0x40 && grid[index+20] >= 0x40) {
		goto out;
	}

	if (!ai_owned_by(pos, player)) goto out;

	if (MAP_HAS_FLAG(pos)) {
		/* Flag: a category 0 target. */
		if ((int16_t)player->ai.u_1a4 < 0) {
			/* Connecting a flag (ai_find_flag_connection): skip
			   flags of its own network; with u_1a4 = -2 only
			   those found in the first three search layers. */
			flag_t *flag = game_get_flag(MAP_OBJ_INDEX(pos));
			if ((int16_t)player->ai.u_1a4 == -1 ||
			    flag->search_dir == 0) {
				if ((flag->search_num & 0xffff) ==
				    (player->ai.u_1a6 & 0xffff)) {
					goto out;
				}
			}
		}

		if (ai_game.g_24a == 0x50) goto out;
		grid[index] = ai_game.g_24a;
		ai_game.g_24a += 1;
		goto out;
	}

	if (AI_BUILD_WATER(player)) {
		/* Water road. */
		if (MAP_PATHS(pos) != 0) goto out;
		if (MAP_OBJ(pos) != 0) goto out;

		/* Original bug: the test of the UP neighbour's up triangle
		   (@0x2b57a) loads the type into D7 and tests the stale D6,
		   so it always passes. Test all six triangles. */
		if (MAP_IN_WATER(pos)) {
			grid[index] = 2;
			*found = 0;
			goto out;
		}

		/* Shore: a category 1 target where a flag can be built. */
		if (ring < 3) goto out;
		ai_cursor_at(player, pos);
		if ((player->ai.map_cursor_type == AI_CURSOR_CLEAR ||
		     player->ai.map_cursor_type == AI_CURSOR_PATH) &&
		    player->ai.panel_btn_type == AI_CAN_BUILD_FLAG &&
		    PLAYER_ALLOW_FLAG(player) &&
		    ai_game.g_24c != 0x7f) {
			grid[index] = ai_game.g_24c;
			ai_game.g_24c += 1;
		}
		goto out;
	}

	if (ai_tile_blocked(pos)) goto out;
	if (MAP_OBJ(pos) != 0 &&
	    map_space_from_obj[MAP_OBJ(pos)] >= MAP_SPACE_SEMIPASSABLE) {
		goto out;
	}

	if (MAP_PATHS(pos) != 0) {
		/* Path: a category 1 target (a flag splits the road). */
		if ((int16_t)player->ai.u_1a4 < 0) goto out;
		ai_cursor_at(player, pos);
		if (player->ai.map_cursor_type == AI_CURSOR_PATH &&
		    player->ai.panel_btn_type == AI_CAN_BUILD_FLAG &&
		    PLAYER_ALLOW_FLAG(player) &&
		    ai_game.g_24c != 0x7f) {
			grid[index] = ai_game.g_24c;
			ai_game.g_24c += 1;
		}
		goto out;
	}

	/* Free tile. */
	ai_cursor_at(player, pos);
	if (player->ai.map_cursor_type >= AI_CURSOR_CLEAR_BY_FLAG &&
	    player->ai.panel_btn_type >= AI_CAN_BUILD_MINE) {
		/* Keep building sites free (mine and large sites more). */
		if (player->ai.panel_btn_type >= AI_CAN_BUILD_LARGE) {
			grid[index] = 20;
		} else if (player->ai.panel_btn_type >= AI_CAN_BUILD_SMALL) {
			grid[index] = 3;
		} else {
			grid[index] = 20;
		}
	} else if (ai_water_marker(pos) ||
		   ai_water_marker(MAP_MOVE_UP_LEFT(pos))) {
		grid[index] = 4;
	} else if (ai_owned_by(MAP_MOVE_UP_LEFT(pos), player) &&
		   ai_owned_by(MAP_MOVE_UP(pos), player) &&
		   ai_owned_by(MAP_MOVE_RIGHT(pos), player) &&
		   ai_owned_by(MAP_MOVE_DOWN_RIGHT(pos), player) &&
		   ai_owned_by(MAP_MOVE_DOWN(pos), player) &&
		   ai_owned_by(MAP_MOVE_LEFT(pos), player)) {
		/* Inside own land. */
		grid[index] = 0;
	} else {
		/* At the border. */
		grid[index] = 3;
	}
	*found = 0;

out:
	grid_pos[index] = pos;
}

/* Number of steps of a path. */
static int
ai_road_path_length(uint64_t path)
{
	int length = 0;
	while (path != 0) {
		path >>= 3;
		length += 1;
	}
	return length;
}

/* ai_road_path_step @0x2b44a: decode the direction list. */
static int
ai_road_path_dirs(uint64_t path, dir_t dirs[])
{
	int length = ai_road_path_length(path);
	for (int i = 0; i < length; i++) {
		dirs[i] = (dir_t)(((path >> (3*(length-1-i))) & 7) - 1);
	}
	return length;
}

/* ai_build_road_along_path @0x2b22c: build the road from the cursor
   along path. category (game+0x248) is 1 if the end is a path or
   shore tile that needs a flag first. The original writes the path
   bits itself (ai_road_finish @0x2b424 only redraws the tiles,
   redraw_map_pos_panels @0x1d20e); here legacy game_build_road
   builds the road. Returns < 0 on failure. */
static int
ai_build_road_along_path(player_t *player, uint64_t path, int category)
{
	dir_t dirs[22];
	int length = ai_road_path_dirs(path, dirs);
	map_pos_t source = AI_CURSOR_POS(player);

	/* The tiles between the ends must be free of paths. */
	map_pos_t pos = source;
	for (int i = 0; i < length; i++) {
		pos = MAP_MOVE(pos, dirs[i]);
		if (i < length-1 && MAP_PATHS(pos) != 0) return -1;
	}
	map_pos_t dest = pos;

	/* Not in the original: legacy checks the segments (objects, owner,
	   water/land changes) more strictly than the AI cost grid. Check
	   the part that does not depend on the end flag before anything
	   is changed. */
	if (category == 0) {
		if (game_can_build_road(source, dirs, length, player,
					NULL, NULL) <= 0) {
			return -1;
		}
	} else if (length > 1) {
		if (game_can_build_road(source, dirs, length-1, player,
					NULL, NULL) <= 0) {
			return -1;
		}
	}

	if (category != 0) {
		/* Place the end flag. */
		ai_game.g_24a = player->ai.cursor_col;
		ai_game.g_24c = player->ai.cursor_row;
		ai_cursor_at(player, dest);

		int ok = 0;
		if (player->ai.map_cursor_type == AI_CURSOR_PATH) {
			if (player->ai.panel_btn_type >= AI_CAN_BUILD_FLAG &&
			    PLAYER_ALLOW_FLAG(player)) {
				game_build_flag(dest, player);
				ok = 1;
			}
		} else if (AI_BUILD_WATER(player) &&
			   player->ai.map_cursor_type == AI_CURSOR_CLEAR &&
			   player->ai.panel_btn_type >= AI_CAN_BUILD_FLAG &&
			   PLAYER_ALLOW_FLAG(player)) {
			game_build_flag(dest, player);
			ai_pull_roads_through_flag(player);
			ok = 1;
		}

		player->ai.cursor_col = ai_game.g_24a;
		player->ai.cursor_row = ai_game.g_24c;
		if (!ok) return -1;
	}

	/* Adjust the limits for the next roads of this call. */
	if ((int16_t)player->ai.u_1a4 > 0) {
		player->ai.u_1a4 -= 1;
		if (player->ai.u_1a4 == 0) {
			player->ai.u_1ba = 70;
			player->ai.u_19c = 10;
		}
	} else {
		if ((player->ai.u_1ba & 0xffff) == 0) {
			int limit = (ai_game.g_24e * 2) & 0xffff;
			if (limit >= 70) limit = 70;
			player->ai.u_1ba = limit;
		}
		if ((player->ai.u_19c & 0xffff) == 0) {
			player->ai.u_19c = player->ai.u_1a8;
		}
	}

	/* The original cannot fail here; legacy may still refuse the last
	   segment (the end flag stays then). */
	if (game_build_road(source, dirs, length, player) < 0) {
		LOGD("ai", "player %i: road from %i failed",
		     player->player_num, source);
		return -1;
	}

	return 0;
}

/* ai_build_road @0x2a8f2: build roads from the flag at the AI cursor.
   Parameters:
     u_1ba  max. cost of a road (0 = no limit; set from the first road),
     u_19c  max. average cost per step (0 = no limit),
     u_1a8  value for u_19c after the first road if it was 0,
     u_1a4  > 0: number of roads until the limits become 70/10,
            -1/-2: connecting a flag (ai_find_flag_connection, the
            flag search id is in u_1a6),
     u_19e  set to 0 when a road was built (callers set -1),
     player->build bit 4: water roads.
   Returns u_19e (< 0: no road built). */
int
ai_build_road(player_t *player)
{
	uint8_t *grid = ai_game.land_influence;

	memset(grid, 0xff, GRID_CELLS);
	/* The original keeps these tables in a scratch area shared with
	   other routines; start from a defined state so that a loaded
	   game continues exactly like the saved one. */
	memset(grid_pos, 0, sizeof(grid_pos));
	memset(grid_best, 0, sizeof(grid_best));
	memset(target_path, 0, sizeof(target_path));
	memset(target_cost, 0, sizeof(target_cost));

	ai_game.g_24e = player->ai.cursor_col;
	ai_game.g_246 = player->ai.cursor_row;

	map_pos_t pos = AI_CURSOR_POS(player);
	int index = GRID_CENTER;

	ai_game.g_24a = 0x40;
	ai_game.g_24c = 0x50;
	grid[index] = 0;
	grid_pos[index] = pos;

	/* Evaluate rings around the cursor while they have passable
	   cells (at most 8 rings). */
	for (int ring = 0; ring < 8; ring++) {
		int found = -1;
		pos = MAP_MOVE_RIGHT(pos);
		index += 1;

		for (int i = 0; i <= ring; i++) {
			ai_road_tile_cost(player, index, pos, ring, &found);
			pos = MAP_MOVE_DOWN(pos);
			index += 19;
		}
		for (int i = 0; i <= ring; i++) {
			ai_road_tile_cost(player, index, pos, ring, &found);
			pos = MAP_MOVE_LEFT(pos);
			index -= 1;
		}
		for (int i = 0; i <= ring; i++) {
			ai_road_tile_cost(player, index, pos, ring, &found);
			pos = MAP_MOVE_UP_LEFT(pos);
			index -= 20;
		}
		for (int i = 0; i <= ring; i++) {
			ai_road_tile_cost(player, index, pos, ring, &found);
			pos = MAP_MOVE_UP(pos);
			index -= 19;
		}
		for (int i = 0; i <= ring; i++) {
			ai_road_tile_cost(player, index, pos, ring, &found);
			pos = MAP_MOVE_RIGHT(pos);
			index += 1;
		}
		for (int i = 0; i <= ring; i++) {
			ai_road_tile_cost(player, index, pos, ring, &found);
			pos = MAP_MOVE_DOWN_RIGHT(pos);
			index += 20;
		}

		if (found < 0) break;
	}

	/* The start is blocked, its neighbours (where no flag can be
	   placed anyway) get 30 more. */
	grid[GRID_CENTER] = 0x7f;
	for (int d = 0; d < 6; d++) {
		static const int order[6] = { 1, 20, 19, -1, -20, -19 };
		uint8_t *v = &grid[GRID_CENTER + order[d]];
		if (*v < 0x80) {
			int c = *v + 30;
			if (c >= 0x40) c = 0x3f;
			*v = c;
		}
	}

	/* Extra cost around own foresters and farms. */
	int start_col = ai_game.g_24e;
	int start_row = ai_game.g_246;
	for (int y = -6; y < 25; y++) {
		for (int x = -6; x < 25; x++) {
			map_pos_t p = MAP_POS((start_col + x - 9) & game.map.col_mask,
					      (start_row + y - 9) & game.map.row_mask);
			const uint8_t *table;
			int width;

			if (MAP_OBJ(p) == MAP_OBJ_SMALL_BUILDING) {
				building_t *b = game_get_building(MAP_OBJ_INDEX(p));
				/* Original bug: compares (type<<2|player) & 0x3f
				   with 0x24 and then the player, so only player 0
				   ever matched. */
				if (BUILDING_TYPE(b) != BUILDING_FORESTER ||
				    BUILDING_PLAYER(b) != (int)player->player_num) {
					continue;
				}
				table = cost_forester;
				width = 13;
			} else if (MAP_OBJ(p) == MAP_OBJ_LARGE_BUILDING) {
				building_t *b = game_get_building(MAP_OBJ_INDEX(p));
				/* Original bug: same as above (0x30). */
				if (BUILDING_TYPE(b) != BUILDING_FARM ||
				    BUILDING_PLAYER(b) != (int)player->player_num) {
					continue;
				}
				table = cost_farm;
				width = 7;
			} else {
				continue;
			}

			int half = width/2;
			for (int ty = 0; ty < width; ty++) {
				int gy = y - half + ty;
				if (gy < 0 || gy >= GRID_SIZE) continue;
				for (int tx = 0; tx < width; tx++) {
					int gx = x - half + tx;
					if (gx < 0 || gx >= GRID_SIZE) continue;
					uint8_t *v = &grid[gy*GRID_SIZE + gx];
					if (*v < 0x40) {
						int c = *v + table[ty*width + tx];
						if (c >= 0x40) c = 0x3f;
						*v = c;
					}
				}
			}
		}
	}

	/* Shortest paths, one layer per step, at most 21 steps. */
	for (int i = 0; i < GRID_CELLS; i++) grid_best[i] = 0xffff;
	memset(target_cost, 0, sizeof(target_cost));

	int sel = 0;
	int count = 1;
	road_queue[0][0].cost = 0xffff;
	road_queue[0][0].index = GRID_CENTER;
	road_queue[0][0].path = 0;

	int layers = 21;
	while (1) {
		road_queue_t *in = road_queue[sel];
		road_queue_t *out = road_queue[!sel];
		int out_count = 0;

		for (int e = 0; e < count; e++) {
			int idx = in[e].index;
			if (grid_best[idx] != in[e].cost) continue;

			uint16_t cost = in[e].cost + 3;
			uint64_t path = in[e].path << 3;
			int h = MAP_HEIGHT(grid_pos[idx]);

			for (int d = 0; d < 6; d++) {
				int nb = idx + grid_dir_offset[d];
				uint8_t c = grid[nb];
				if (c >= 0x7f) continue;
				if (cost >= grid_best[nb]) continue;

				uint16_t new_cost = cost + c;
				int dh = (int)MAP_HEIGHT(grid_pos[nb]) - h;
				if (dh < 0) dh = -dh;
				if (dh == 2) new_cost += 1;
				else if (dh == 3) new_cost += 3;
				else if (dh > 3) new_cost += 8;

				if (new_cost >= grid_best[nb]) continue;
				grid_best[nb] = new_cost;

				uint64_t new_path = path | (d+1);
				if (c >= 0x40) {
					target_cost[c-0x40] = new_cost;
					target_path[c-0x40] = new_path;
				} else if (out_count < ROAD_QUEUE_SIZE) {
					out[out_count].cost = new_cost;
					out[out_count].index = nb;
					out[out_count].path = new_path;
					out_count += 1;
				}
			}
		}

		sel = !sel;
		layers -= 1;
		if (layers == 0) break;
		count = out_count;
		if (count == 0) break;
	}
	ai_game.g_24a = -1;
	ai_game.g_248 = layers;

	player->ai.cursor_col = ai_game.g_24e;
	player->ai.cursor_row = ai_game.g_246;

	/* Candidates: cost minus target code, path/shore targets 1.5x. */
	uint16_t cand_cost[MAX_TARGETS];
	int cand_cat[MAX_TARGETS];
	uint64_t cand_path[MAX_TARGETS];
	int cand_count = 0;
	for (int k = 0; k < MAX_TARGETS; k++) {
		if (target_cost[k] == 0) continue;
		uint16_t cost = target_cost[k] - (0x40 + k);
		int cat = 0;
		if (0x40 + k >= 0x50) {
			cost += cost >> 1;
			cat = 1;
		}
		cand_cost[cand_count] = cost;
		cand_cat[cand_count] = cat;
		cand_path[cand_count] = target_path[k];
		cand_count += 1;
	}

	/* Build the roads, cheapest first. */
	while (1) {
		int best = -1;
		uint16_t best_cost = 0xffff;
		for (int i = 0; i < cand_count; i++) {
			if (cand_cost[i] & 0x8000) break; /* end marker */
			if (cand_cost[i] == 0) continue;
			if (cand_cost[i] < best_cost) {
				best_cost = cand_cost[i];
				best = i;
			}
		}
		if (best < 0 || (best_cost & 0x8000)) break;

		if ((player->ai.u_1ba & 0xffff) != 0 &&
		    best_cost >= (player->ai.u_1ba & 0xffff)) {
			break;
		}

		if ((player->ai.u_19c & 0xffff) != 0) {
			int length = ai_road_path_length(cand_path[best]);
			uint16_t limit = (player->ai.u_19c * length) & 0xffff;
			if (best_cost >= limit) {
				/* Too much detour. */
				cand_cost[best] = 0;
				continue;
			}
		}

		ai_game.g_24e = best_cost;
		cand_cost[best] = 0;
		ai_game.g_248 = cand_cat[best];
		if (ai_build_road_along_path(player, cand_path[best],
					     cand_cat[best]) >= 0) {
			player->ai.u_19e = 0;
		}
	}

	return (int16_t)player->ai.u_19e;
}

/* ai_find_flag_connection @0x29316: connect the flag at the AI cursor
   to another flag. A flag search over the land roads first looks for
   an inventory within the network (three layers, then layer by
   layer). Found: road to a flag outside the near part of the network
   (u_1a4 = -2, limits 100/20). Not found: road to a flag of another
   network (u_1a4 = -1).
   The original gets the flag in A0 and its position in D0/D1 and sets
   the cursor to it; here the caller sets the cursor to the flag.
   Returns 1 if the flag already has all six paths (nothing done, the
   caller's D7 is unchanged), 0 if a road was built (the original sets
   D7 = 0), -1 if not (the original subtracts 600 from D7). */
int
ai_find_flag_connection(player_t *player)
{
	map_pos_t pos = AI_CURSOR_POS(player);
	flag_t *flag = game_get_flag(MAP_OBJ_INDEX(pos));

	if (FLAG_PATHS(flag) == 0x3f) return 1;

	flag_search_t search;
	flag_search_init(&search);
	int id = search.id;

	int sel = 0;
	int count = 1;
	flag_queue[0][0] = flag;

	int layers = 2;
	dir_t layer_dir = DIR_RIGHT;	/* 0 */
	int found = 0;
	int connected = 0;

	while (1) {
		flag_t **in = flag_queue[sel];
		flag_t **out = flag_queue[!sel];
		int out_count = 0;

		for (int i = 0; i < count; i++) {
			flag_t *f = in[i];
			f->search_dir = layer_dir;
			for (int d = DIR_UP; d >= DIR_RIGHT; d--) {
				if (!BIT_TEST(f->endpoint, d)) continue;
				flag_t *other = f->other_endpoint.f[d];
				if (other->search_num == id) continue;
				if (FLAG_HAS_INVENTORY(other)) found = 1;
				other->search_num = id;
				out[out_count++] = other;
			}
			if (out_count > 0x3e2) break;
		}

		sel = !sel;
		layers -= 1;
		if (layers < 0) {
			/* Original stores 0xff; only != 0 is tested. */
			layer_dir = DIR_NONE;
			if (found) {
				connected = 1;
				break;
			}
		}

		count = out_count;
		if (count == 0) break;
	}

	if (connected) {
		player->ai.u_1ba = 100;
		player->ai.u_19c = 20;
		player->ai.u_1a4 = -2;
		player->ai.u_1a6 = id;
		player->ai.u_19e = 0;
	} else {
		player->ai.u_1ba = 0;
		player->ai.u_19c = 0;
		player->ai.u_1a8 = 12;
		player->ai.u_1a4 = -1;
		player->ai.u_1a6 = id;
		player->ai.u_19e = -1;
	}

	AI_SET_CURSOR(player, pos);
	player->build &= ~BIT(4);
	if (ai_build_road(player) < 0) return -1;
	return 0;
}


/* Static copies of legacy game.c helpers (wake_transporter_*,
   fill_path_serf_info, restore_path_serf_info and the tail of
   build_flag_split_path), Amiga 0x26ad8, 0x268d8, 0x276e0, 0x274a8. */

typedef struct {
	int path_len;
	int serf_count;
	int flag_index;
	dir_t flag_dir;
	int serfs[16];
} ai_path_info_t;

static int
ai_wake_transporter_at_flag(map_pos_t pos)
{
	for (uint i = 1; i < game.max_serf_index; i++) {
		if (SERF_ALLOCATED(i)) {
			serf_t *serf = game_get_serf(i);
			if (serf->pos == pos &&
			    (serf->state == SERF_STATE_WAKE_AT_FLAG ||
			     serf->state == SERF_STATE_WAKE_ON_PATH ||
			     serf->state == SERF_STATE_WAIT_IDLE_ON_PATH ||
			     serf->state == SERF_STATE_IDLE_ON_PATH)) {
				serf_log_state_change(serf, SERF_STATE_WAKE_AT_FLAG);
				serf->state = SERF_STATE_WAKE_AT_FLAG;
				return SERF_INDEX(serf);
			}
		}
	}

	return -1;
}

/* wake_idle_serfs_at_pos @0x26ad8. */
static int
ai_wake_transporter_on_path(map_pos_t pos)
{
	for (uint i = 1; i < game.max_serf_index; i++) {
		if (SERF_ALLOCATED(i)) {
			serf_t *serf = game_get_serf(i);
			if (serf->pos != pos) continue;

			switch (serf->state) {
			case SERF_STATE_IDLE_ON_PATH:
			case SERF_STATE_WAIT_IDLE_ON_PATH:
				serf_log_state_change(serf, SERF_STATE_WAKE_ON_PATH);
				serf->state = SERF_STATE_WAKE_ON_PATH;
				return SERF_INDEX(serf);
			case SERF_STATE_WAKE_ON_PATH:
				return SERF_INDEX(serf);
			case SERF_STATE_WAKE_AT_FLAG:
				return -1;
			default:
				break;
			}
		}
	}

	return -1;
}

/* fill_path_serf_info @0x268d8. */
static void
ai_fill_path_serf_info(map_pos_t pos, dir_t dir, ai_path_info_t *data)
{
	if (MAP_IDLE_SERF(pos)) ai_wake_transporter_at_flag(pos);

	int serf_count = 0;
	int path_len = 0;

	/* Handle first position. */
	if (MAP_SERF_INDEX(pos) != 0) {
		serf_t *serf = game_get_serf(MAP_SERF_INDEX(pos));
		if (serf->state == SERF_STATE_TRANSPORTING &&
		    serf->s.walking.wait_counter != -1) {
			int d = serf->s.walking.dir;
			if (d < 0) d += 6;

			if (dir == d) {
				serf->s.walking.wait_counter = 0;
				data->serfs[serf_count++] = SERF_INDEX(serf);
			}
		}
	}

	/* Trace along the path to the flag at the other end. */
	while (1) {
		path_len += 1;
		pos = MAP_MOVE(pos, dir);
		int paths = MAP_PATHS(pos);
		paths &= ~BIT(DIR_REVERSE(dir));

		if (MAP_HAS_FLAG(pos)) break;

		for (int d = DIR_RIGHT; d <= DIR_UP; d++) {
			if (BIT_TEST(paths, d)) {
				dir = (dir_t)d;
				break;
			}
		}

		if (MAP_IDLE_SERF(pos)) {
			int index = ai_wake_transporter_on_path(pos);
			if (index >= 0) data->serfs[serf_count++] = index;
		}

		if (MAP_SERF_INDEX(pos) != 0) {
			serf_t *serf = game_get_serf(MAP_SERF_INDEX(pos));
			if (serf->state == SERF_STATE_TRANSPORTING &&
			    serf->s.walking.wait_counter != -1) {
				serf->s.walking.wait_counter = 0;
				data->serfs[serf_count++] = SERF_INDEX(serf);
			}
		}
	}

	/* Handle last position. */
	if (MAP_SERF_INDEX(pos) != 0) {
		serf_t *serf = game_get_serf(MAP_SERF_INDEX(pos));
		if ((serf->state == SERF_STATE_TRANSPORTING &&
		     serf->s.walking.wait_counter != -1) ||
		    serf->state == SERF_STATE_DELIVERING) {
			int d = serf->s.walking.dir;
			if (d < 0) d += 6;

			if (d == DIR_REVERSE(dir)) {
				serf->s.walking.wait_counter = 0;
				data->serfs[serf_count++] = SERF_INDEX(serf);
			}
		}
	}

	data->path_len = path_len;
	data->serf_count = serf_count;
	data->flag_index = MAP_OBJ_INDEX(pos);
	data->flag_dir = DIR_REVERSE(dir);
}

/* restore_path_serf_info @0x276e0. */
static void
ai_restore_path_serf_info(flag_t *flag, dir_t dir, ai_path_info_t *data)
{
	const int max_path_serfs[] = { 1, 2, 3, 4, 6, 8, 11, 15 };

	flag_t *other_flag = game_get_flag(data->flag_index);
	dir_t other_dir = data->flag_dir;

	flag->path_con |= BIT(dir);
	flag->endpoint &= ~BIT(dir);

	if (!FLAG_IS_WATER_PATH(other_flag, other_dir)) {
		flag->endpoint |= BIT(dir);
	}

	other_flag->transporter &= ~BIT(other_dir);
	flag->transporter &= ~BIT(dir);

	int len = get_road_length_value(data->path_len);

	flag->length[dir] = len << 4;
	other_flag->length[other_dir] = (0x80 & other_flag->length[other_dir]) | (len << 4);

	if (FLAG_SERF_REQUESTED(other_flag, other_dir)) {
		flag->length[dir] |= BIT(7);
	}

	flag->other_end_dir[dir] = (flag->other_end_dir[dir] & 0xc7) | (other_dir << 3);
	other_flag->other_end_dir[other_dir] = (other_flag->other_end_dir[other_dir] & 0xc7) | (dir << 3);

	flag->other_endpoint.f[dir] = other_flag;
	other_flag->other_endpoint.f[other_dir] = flag;

	int max_serfs = max_path_serfs[len];
	if (FLAG_SERF_REQUESTED(flag, dir)) max_serfs -= 1;

	if (data->serf_count > max_serfs) {
		for (int i = 0; i < data->serf_count - max_serfs; i++) {
			serf_t *serf = game_get_serf(data->serfs[i]);
			if (serf->state != SERF_STATE_WAKE_ON_PATH) {
				serf->s.walking.wait_counter = -1;
				if (serf->s.walking.res != 0) {
					resource_type_t res = (resource_type_t)(serf->s.walking.res-1);
					serf->s.walking.res = 0;

					game_cancel_transported_resource(res, serf->s.walking.dest);
					game_lose_resource(res);
				}
			} else {
				serf_log_state_change(serf, SERF_STATE_WAKE_AT_FLAG);
				serf->state = SERF_STATE_WAKE_AT_FLAG;
			}
		}
	}

	if (min(data->serf_count, max_serfs) > 0) {
		flag->transporter |= BIT(dir);
		other_flag->transporter |= BIT(other_dir);

		flag->length[dir] |= min(data->serf_count, max_serfs);
		other_flag->length[other_dir] |= min(data->serf_count, max_serfs);
	}
}

/* Turn the walking direction of a serf at pos from old_dir to new_dir
   (also the negative "waiting" encoding, dir - 6). */
static void
ai_redirect_serf(map_pos_t pos, int old_dir, int new_dir)
{
	if (MAP_SERF_INDEX(pos) == 0) return;

	serf_t *serf = game_get_serf(MAP_SERF_INDEX(pos));
	/* Original bug: the dir - 6 case is applied to serfs in any state
	   (byte +0xe of the state union, @0x27402). Only walking and
	   transporting serfs are changed here. */
	if (serf->state != SERF_STATE_WALKING &&
	    serf->state != SERF_STATE_TRANSPORTING) {
		return;
	}

	if (serf->s.walking.dir == old_dir) {
		serf->s.walking.dir = new_dir;
	} else if (serf->s.walking.dir == old_dir - 6) {
		serf->s.walking.dir = new_dir - 6;
	}
}

/* ai_reroute_path_corner @0x2738c: a road passes the flag at pos
   between its neighbours in direction dir and dir + 1. Lead it
   through the flag and split it there. */
static void
ai_reroute_path_corner(map_pos_t pos, int dir)
{
	map_tile_t *tiles = game.map.tiles;
	int dir_1 = dir;
	int dir_2 = (dir + 1) % 6;

	tiles[pos].paths |= BIT(dir_1) | BIT(dir_2);

	/* First neighbour: the corner segment goes to dir + 2. */
	map_pos_t pos_1 = MAP_MOVE(pos, dir_1);
	int old_dir = (dir_1 + 2) % 6;
	int new_dir = (old_dir + 1) % 6;
	tiles[pos_1].paths &= ~BIT(old_dir);
	tiles[pos_1].paths |= BIT(new_dir);
	if (MAP_IDLE_SERF(pos_1)) ai_wake_transporter_on_path(pos_1);
	ai_redirect_serf(pos_1, old_dir, new_dir);

	/* Second neighbour: the corner segment goes to dir - 1. */
	map_pos_t pos_2 = MAP_MOVE(pos, dir_2);
	old_dir = (dir_2 + 4) % 6;
	new_dir = (old_dir + 5) % 6;
	tiles[pos_2].paths &= ~BIT(old_dir);
	tiles[pos_2].paths |= BIT(new_dir);
	if (MAP_IDLE_SERF(pos_2)) ai_wake_transporter_on_path(pos_2);
	ai_redirect_serf(pos_2, old_dir, new_dir);

	/* Split the road at the flag (as build_flag_split_path). */
	ai_path_info_t path_1_data;
	ai_path_info_t path_2_data;

	ai_fill_path_serf_info(pos, (dir_t)dir_1, &path_1_data);
	ai_fill_path_serf_info(pos, (dir_t)dir_2, &path_2_data);

	flag_t *flag_2 = game_get_flag(path_2_data.flag_index);
	dir_t flag_dir_2 = path_2_data.flag_dir;

	int select = -1;
	if (FLAG_SERF_REQUESTED(flag_2, flag_dir_2)) {
		for (uint i = 1; i < game.max_serf_index; i++) {
			if (SERF_ALLOCATED(i)) {
				serf_t *serf = game_get_serf(i);

				if (serf->state == SERF_STATE_WALKING) {
					if (serf->s.walking.dest == path_1_data.flag_index &&
					    serf->s.walking.res == path_1_data.flag_dir) {
						select = 0;
						break;
					} else if (serf->s.walking.dest == path_2_data.flag_index &&
						   serf->s.walking.res == path_2_data.flag_dir) {
						select = 1;
						break;
					}
				} else if (serf->state == SERF_STATE_READY_TO_LEAVE_INVENTORY) {
					if (serf->s.ready_to_leave_inventory.dest == path_1_data.flag_index &&
					    serf->s.ready_to_leave_inventory.mode == path_1_data.flag_dir) {
						select = 0;
						break;
					} else if (serf->s.ready_to_leave_inventory.dest == path_2_data.flag_index &&
						   serf->s.ready_to_leave_inventory.mode == path_2_data.flag_dir) {
						select = 1;
						break;
					}
				} else if ((serf->state == SERF_STATE_READY_TO_LEAVE ||
					    serf->state == SERF_STATE_LEAVING_BUILDING) &&
					   serf->s.leaving_building.next_state == SERF_STATE_WALKING) {
					if (serf->s.leaving_building.dest == path_1_data.flag_index &&
					    serf->s.leaving_building.field_B == path_1_data.flag_dir) {
						select = 0;
						break;
					} else if (serf->s.leaving_building.dest == path_2_data.flag_index &&
						   serf->s.leaving_building.field_B == path_2_data.flag_dir) {
						select = 1;
						break;
					}
				}
			}
		}

		/* The original defaults to the path 2 end (@0x2753e). */
		ai_path_info_t *path_data = &path_2_data;
		if (select == 1) path_data = &path_1_data;

		flag_t *selected_flag = game_get_flag(path_data->flag_index);
		selected_flag->length[path_data->flag_dir] &= ~BIT(7);
	}

	flag_t *flag = game_get_flag(MAP_OBJ_INDEX(pos));

	ai_restore_path_serf_info(flag, (dir_t)dir_1, &path_1_data);
	ai_restore_path_serf_info(flag, (dir_t)dir_2, &path_2_data);
}

/* ai_pull_roads_through_flag @0x271d4: roads passing the flag at the
   AI cursor around a corner (between two neighbours, where the flag
   has no paths) are led through the flag (the same test as
   flag_has_road_corner @0x270fe). Roads that now start and end at
   this flag are removed. Returns 0 if a road was rerouted, -1 if not
   (or no flag at the cursor). */
int
ai_pull_roads_through_flag(player_t *player)
{
	/* Per corner: neighbour direction and the path direction of the
	   corner segment there. */
	static const dir_t corner_nb[6] = {
		DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_LEFT, DIR_UP, DIR_RIGHT
	};
	static const dir_t corner_path[6] = {
		DIR_DOWN, DIR_RIGHT, DIR_DOWN_RIGHT, DIR_UP, DIR_LEFT, DIR_UP_LEFT
	};

	map_pos_t pos = AI_CURSOR_POS(player);
	int r = -1;

	if (!MAP_HAS_FLAG(pos)) return -1;

	for (int d = 0; d < 6; d++) {
		int mask = BIT(d) | BIT((d + 1) % 6);
		if (MAP_PATHS(pos) & mask) continue;

		map_pos_t nb = MAP_MOVE(pos, corner_nb[d]);
		if (BIT_TEST(MAP_PATHS(nb), corner_path[d])) {
			ai_reroute_path_corner(pos, d);
			r = 0;
		}
	}

	if (r < 0) return -1;

	flag_t *flag = game_get_flag(MAP_OBJ_INDEX(pos));
	for (int d = DIR_UP; d >= DIR_RIGHT; d--) {
		if (FLAG_HAS_PATH(flag, d) &&
		    flag->other_endpoint.v[d] == (void *)flag) {
			/* Original: demolish_road @0x26532 without checks. */
			game_demolish_road(MAP_MOVE(pos, d), player);
		}
	}

	return 0;
}
