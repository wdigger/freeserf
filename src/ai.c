/*
 * ai.c - Computer player core, ported from the Amiga original
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

/* Port of the Amiga AI core: the phase machine ai_update (0x28ee2) with
   its slot dispatch, the random site scan (0x2d162), the computer
   player's map cursor evaluation (0x19368), the settings/priority
   updates, the building statistics and the build step (0x2a5c4).
   All values are unsigned 16-bit as in the original: W() wraps, sadd()
   saturates (add + bcs -> 0xffff). */

#include <stddef.h>

#include "ai_internal.h"
#include "resource.h"
#include "misc.h"

ai_game_t ai_game;

#define W(x)  ((unsigned)(x) & 0xffff)

/* Saturating unsigned 16-bit add. */
static unsigned
sadd(unsigned a, unsigned b)
{
	unsigned r = W(a) + W(b);
	return r > 0xffff ? 0xffff : r;
}

/* Unsigned 16-bit subtract clamped at 0 (sub + bcs -> 0). */
static unsigned
ssub(unsigned a, unsigned b)
{
	return W(a) < W(b) ? 0 : W(a) - W(b);
}


/* ---- Amiga data ---- */

/* map_space_from_obj of the original (0x1f42): 0 open, 1 filled,
   2 semipassable/impassable, 3 flag, 4/5/6 small/large building/castle,
   0xff for object 127. Its encoding differs from legacy's table. */
static const uint8_t amiga_space_from_obj[128] = {
	0, 3, 4, 5, 6, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 1, 0, 0, 0, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1,
	1, 0, 1, 1, 1, 1, 0, 1, 1, 2, 2, 2, 2, 2, 2, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 255
};

/* Site categories allowed per panel_btn_type 1..5 (0x2d234). */
static const uint32_t site_categories[5] = {
	0x1, 0x20001e0, 0x8a1e, 0xfffe1e, 0x1000000
};

/* Knight occupation table of ai_adjust_flags (0x2a4b0): 17 rows of
   occupation values for threat levels 3, 2, 1, 0. */
static const uint8_t occupation_rows[17][4] = {
	{4, 4, 4, 4}, {4, 4, 4, 3}, {4, 4, 4, 2}, {4, 4, 3, 2},
	{4, 4, 3, 1}, {4, 4, 2, 1}, {4, 3, 2, 1}, {4, 3, 2, 0},
	{4, 3, 1, 0}, {4, 3, 0, 0}, {4, 2, 0, 0}, {4, 1, 0, 0},
	{3, 1, 0, 0}, {2, 1, 0, 0}, {2, 0, 0, 0}, {1, 0, 0, 0},
	{0, 0, 0, 0}
};

/* Knights per occupation value for hut, tower, fortress (0x2a4f4). */
static const uint8_t knights_per_level[3][5] = {
	{1, 1, 2, 2, 3}, {1, 2, 3, 4, 6}, {1, 3, 6, 9, 12}
};


/* ---- Access to the statistics words ---- */

/* Word at Amiga pointer-relative offset off (0x33c..0x3ce): poi[4]
   (used as statistics array) and u_394..u_3ce. Same layout as
   ai_want.c reads. */
static int *
ai_stat(player_t *player, int off)
{
	static const size_t u_offset[] = {
		offsetof(player_ai_t, u_394), offsetof(player_ai_t, u_396),
		offsetof(player_ai_t, u_398), offsetof(player_ai_t, u_39a),
		offsetof(player_ai_t, u_39c), offsetof(player_ai_t, u_39e),
		offsetof(player_ai_t, u_3a0), offsetof(player_ai_t, u_3a2),
		offsetof(player_ai_t, u_3a4), offsetof(player_ai_t, u_3a6),
		offsetof(player_ai_t, u_3a8), offsetof(player_ai_t, u_3aa),
		offsetof(player_ai_t, u_3ac), offsetof(player_ai_t, u_3ae),
		offsetof(player_ai_t, u_3b0), offsetof(player_ai_t, u_3b2),
		offsetof(player_ai_t, u_3b4), offsetof(player_ai_t, u_3b6),
		offsetof(player_ai_t, u_3b8), offsetof(player_ai_t, u_3ba),
		offsetof(player_ai_t, u_3bc), offsetof(player_ai_t, u_3be),
		offsetof(player_ai_t, u_3c0), offsetof(player_ai_t, u_3c2),
		offsetof(player_ai_t, u_3c4), offsetof(player_ai_t, u_3c6),
		offsetof(player_ai_t, u_3c8), offsetof(player_ai_t, u_3ca),
		offsetof(player_ai_t, u_3cc), offsetof(player_ai_t, u_3ce)
	};
	ai_poi_t *p = &player->ai.poi[4];
	int w;

	if (off >= 0x394) {
		return (int *)((char *)&player->ai +
			       u_offset[(off - 0x394) / 2]);
	}

	w = (off - 0x33c) / 2;
	if (w >= 5 && w < 30) return &p->bld_count[w - 5];
	if (w >= 33 && w < 37) return &p->deposit[w - 33];
	switch (w) {
	case 0: return &p->field_0;
	case 1: return &p->field_2;
	case 2: return &p->field_4;
	case 3: return &p->field_6;
	case 4: return &p->field_8;
	case 30: return &p->trees;
	case 31: return &p->field_3e;
	case 32: return &p->stones;
	case 37: return &p->field_4a;
	case 38: return &p->field_4c;
	case 39: return &p->field_4e;
	case 40: return &p->field_50;
	case 41: return &p->field_52;
	case 42: return &p->field_54;
	default: return &p->field_56;
	}
}

/* ptr+0x33c + 2*slot: stock fill ratio. */
#define FILL(slot)   (*ai_stat(player, 0x33c + 2*(slot)))
/* ptr+0x366 + 2*serf_type: serfs idle in inventories. */
#define IDLE(type)   (*ai_stat(player, 0x366 + 2*(type)))
/* ptr+0x39c + 2*resource: resources in all inventories. */
#define STOCK(res)   (*ai_stat(player, 0x39c + 2*(res)))

/* Pending road connections (ptr+0x1bc..0x1da): 8 x {col, row}, empty
   when col has bit 15 set (the original's tst.l bmi). Filled by the
   game when a knight of the AI player occupies an enemy building. */
static int *
ai_pending(player_t *player, int i, int row)
{
	player_ai_t *ai = &player->ai;
	int *tab[16] = {
		&ai->u_1bc, &ai->u_1be, &ai->u_1c0, &ai->u_1c2,
		&ai->u_1c4, &ai->u_1c6, &ai->u_1c8, &ai->u_1ca,
		&ai->u_1cc, &ai->u_1ce, &ai->u_1d0, &ai->u_1d2,
		&ai->u_1d4, &ai->u_1d6, &ai->u_1d8, &ai->u_1da
	};
	return tab[2*i + row];
}


/* ---- Map helpers (Amiga tile bytes) ---- */

#define SPACE(pos)   (amiga_space_from_obj[MAP_OBJ(pos)])
#define OWNERBITS(pos)  ((unsigned)(game.map.tiles[(pos)].height & 0xe0))
#define TYPEBYTE(pos)   ((unsigned)game.map.tiles[(pos)].type)
#define PATHBITS(pos)   ((unsigned)(game.map.tiles[(pos)].paths & 0x3f))

static map_pos_t
spiral(map_pos_t pos, int i)
{
	return MAP_POS_ADD(pos, game.spiral_pos_pattern[i]);
}

/* Owner pattern of the height byte: bit 7 has owner, bits 5-6 owner. */
static unsigned
own_bits(const player_t *player)
{
	return 0x80 | ((player->player_num & 3) << 5);
}


/* ---- Map cursor type (0x1932e..0x19af8) ---- */

/* determine_map_cursor_type_sub @0x196d8: classify a triangle. */
static int
triangle_class(unsigned type)
{
	if (type < 4) return 2;
	if (type < 8) return 0;
	if (type >= 11 && type < 15) return 1;
	return 2;
}

/* determine_possible_building @0x19660. own = owner pattern (D3). */
static void
determine_possible_building(player_t *player, map_pos_t pos, unsigned own)
{
	int has_castle = PLAYER_HAS_CASTLE(player);

	for (int i = 1; i <= 6; i++) {
		if (OWNERBITS(spiral(pos, i)) != own) return;
	}

	map_pos_t p4 = spiral(pos, 4), p5 = spiral(pos, 5), p6 = spiral(pos, 6);
	int cls = triangle_class(TYPEBYTE(pos) >> 4) |
		triangle_class(TYPEBYTE(pos) & 0xf) |
		triangle_class(TYPEBYTE(p4) & 0xf) |
		triangle_class(TYPEBYTE(p5) >> 4) |
		triangle_class(TYPEBYTE(p5) & 0xf) |
		triangle_class(TYPEBYTE(p6) >> 4);
	if (cls >= 2) return;
	if (cls == 1) {
		if (has_castle) player->ai.panel_btn_type = AI_CAN_BUILD_MINE;
		return;
	}
	if (has_castle) player->ai.panel_btn_type = AI_CAN_BUILD_SMALL;

	/* Military buildings in the second shell (only 7..18; the
	   original does not look at the center and the first shell). */
	player->build &= ~BIT(0);
	for (int i = 7; i <= 18; i++) {
		map_pos_t p = spiral(pos, i);
		int obj = MAP_OBJ(p);
		if (obj >= MAP_OBJ_SMALL_BUILDING && obj <= MAP_OBJ_CASTLE) {
			building_t *b = game_get_building(MAP_OBJ_INDEX(p));
			int t = BUILDING_TYPE(b);
			if (t == BUILDING_HUT || t == BUILDING_TOWER ||
			    t == BUILDING_FORTRESS || t == BUILDING_CASTLE) {
				player->build |= BIT(0);
				break;
			}
		}
	}

	for (int i = 1; i <= 6; i++) {
		unsigned s = SPACE(spiral(pos, i));
		if (s >= 2 && s != 3) return;
	}
	for (int i = 7; i <= 18; i++) {
		if (SPACE(spiral(pos, i)) >= 5) return;
	}

	if (TYPEBYTE(pos) != 0x55) return;
	if ((TYPEBYTE(p4) & 0xf) != 5) return;
	if (TYPEBYTE(p5) != 0x55) return;
	if ((TYPEBYTE(p6) & 0xf0) != 0x50) return;

	unsigned h_min = 31, h_max = 0;
	for (int i = 7; i <= 18; i++) {
		unsigned h = MAP_HEIGHT(spiral(pos, i));
		if (h <= h_min) h_min = h;
		if (h > h_max) h_max = h;
	}
	for (int i = 19; i <= 36; i++) {
		map_pos_t p = spiral(pos, i);
		if (MAP_OBJ(p) != MAP_OBJ_LARGE_BUILDING) continue;
		building_t *b = game_get_building(MAP_OBJ_INDEX(p));
		if (BUILDING_IS_DONE(b) || b->progress != 0) continue;
		unsigned h = b->u.level & 0xff;
		if (h <= h_min) h_min = h;
		if (h > h_max) h_max = h;
	}
	if (W(h_max - h_min) >= 9) return;

	/* The leveling height (ptr+0x102) is not kept: legacy
	   game_build_building computes it itself. */
	player->ai.panel_btn_type = has_castle ? AI_CAN_BUILD_LARGE :
		AI_CAN_BUILD_CASTLE;
}

/* Tail of determine_map_cursor_type from 0x194ee: building and flag
   possibilities once map_cursor_type is known. */
static void
determine_build_possibility(player_t *player, map_pos_t pos, unsigned own)
{
	player_ai_t *ai = &player->ai;

	if (SPACE(pos) != 0) return;

	/* All six triangles water? */
	if (!(TYPEBYTE(pos) & 0xcc) &&
	    !(TYPEBYTE(spiral(pos, 4)) & 0x0c) &&
	    !(TYPEBYTE(spiral(pos, 5)) & 0xcc) &&
	    !(TYPEBYTE(spiral(pos, 6)) & 0xc0)) {
		return;
	}

	int flag_near = 0;
	for (int i = 1; i <= 6; i++) {
		if (SPACE(spiral(pos, i)) == 3) {
			flag_near = 1;
			break;
		}
	}
	if (flag_near) {
		if (ai->map_cursor_type == AI_CURSOR_PATH) return;
	} else {
		player->build &= ~BIT(1);
		if (PLAYER_HAS_CASTLE(player)) {
			ai->panel_btn_type = AI_CAN_BUILD_FLAG;
		}
		if (ai->map_cursor_type == AI_CURSOR_PATH) return;
	}

	for (int i = 1; i <= 6; i++) {
		if (SPACE(spiral(pos, i)) >= 4) return;
	}
	if (ai->map_cursor_type != AI_CURSOR_CLEAR_BY_FLAG &&
	    SPACE(spiral(pos, 2)) != 0) {
		return;
	}

	/* Flags near the building flag. */
	static const int flag_ring[] = { 7, 8, 14, 1, 3 };
	for (int i = 0; i < 5; i++) {
		if (SPACE(spiral(pos, flag_ring[i])) == 3) return;
	}

	/* Triangles around the building flag must be land. */
	if (!(TYPEBYTE(spiral(pos, 1)) & 0xc0) ||
	    !(TYPEBYTE(spiral(pos, 3)) & 0x0c) ||
	    !(TYPEBYTE(spiral(pos, 2)) & 0xc0) ||
	    !(TYPEBYTE(spiral(pos, 2)) & 0x0c)) {
		return;
	}

	determine_possible_building(player, pos, own);
}

/* get_map_cursor_type @0x194ba (cursor on a free tile). */
static void
get_map_cursor_type(player_t *player, map_pos_t pos, unsigned own)
{
	map_pos_t p2 = spiral(pos, 2);

	if (SPACE(p2) == 3) {
		player->ai.map_cursor_type = AI_CURSOR_CLEAR_BY_FLAG;
	} else if (PATHBITS(p2) != 0) {
		player->ai.map_cursor_type = AI_CURSOR_CLEAR_BY_PATH;
	} else {
		player->ai.map_cursor_type = AI_CURSOR_CLEAR;
	}
	determine_build_possibility(player, pos, own);
}

/* determine_map_cursor_type @0x19368 at pos. */
static void
determine_map_cursor_type_at(player_t *player, map_pos_t pos)
{
	player_ai_t *ai = &player->ai;
	unsigned own = PLAYER_HAS_CASTLE(player) ? own_bits(player) : 0;

	player->build |= BIT(1);
	ai->map_cursor_type = AI_CURSOR_NONE;
	ai->panel_btn_type = AI_CAN_BUILD_NONE;

	if (OWNERBITS(pos) != own) return;

	unsigned s = SPACE(pos);
	if (s == 3) {
		/* Flag */
		if ((game.map.tiles[pos].paths & 0x10) &&
		    SPACE(spiral(pos, 5)) >= 4) {
			ai->map_cursor_type = AI_CURSOR_FLAG;
			return;
		}
		if (PATHBITS(pos) == 0) {
			ai->map_cursor_type = AI_CURSOR_REMOVABLE_FLAG;
			return;
		}

		flag_t *flag = game_get_flag(MAP_OBJ_INDEX(pos));
		flag_t *other = NULL;
		int paths = 0;
		for (int d = 5; d >= 0; d--) {
			if (!(flag->path_con & BIT(d))) continue;
			if (!(flag->endpoint & BIT(d))) {
				/* Water path */
				ai->map_cursor_type = AI_CURSOR_FLAG;
				return;
			}
			paths += 1;
			if (other == NULL) {
				other = flag->other_endpoint.f[d];
			} else if (other == flag->other_endpoint.f[d]) {
				ai->map_cursor_type = AI_CURSOR_FLAG;
				return;
			}
		}
		ai->map_cursor_type = (paths == 2) ? AI_CURSOR_REMOVABLE_FLAG :
			AI_CURSOR_FLAG;
		return;
	} else if (s >= 4) {
		/* Original bug: object 127 (space 0xff) is taken for a
		   building and its obj_index read as building index. */
		if (s == 6 || s == 0xff) return;
		building_t *b = game_get_building(MAP_OBJ_INDEX(pos));
		if (BUILDING_IS_BURNING(b)) return;
		ai->map_cursor_type = AI_CURSOR_BUILDING;
		determine_possible_building(player, pos, own);
		return;
	}

	unsigned paths = PATHBITS(pos);
	if (paths == 0) {
		get_map_cursor_type(player, pos, own);
		return;
	}
	if (paths == BIT(DIR_DOWN_RIGHT) || paths == BIT(DIR_UP_LEFT)) return;
	ai->map_cursor_type = AI_CURSOR_PATH;
	determine_build_possibility(player, pos, own);
}

/* determine_map_cursor_type @0x19368 for the AI cursor. */
void
ai_determine_map_cursor_type(player_t *player)
{
	determine_map_cursor_type_at(player, AI_CURSOR_POS(player));
}

/* ai_get_map_cursor_type_at @0x1932e: get_map_cursor_type for pos
   without the checks of the center (the caller player_ai made them);
   the owner pattern own is the caller's D3. The cursor is not moved. */
static void
ai_get_map_cursor_type_at(player_t *player, map_pos_t pos, unsigned own)
{
	player->build |= BIT(1);
	player->ai.map_cursor_type = AI_CURSOR_NONE;
	player->ai.panel_btn_type = AI_CAN_BUILD_NONE;
	get_map_cursor_type(player, pos, own);
}


/* ---- Site scan and start ---- */

/* player_ai @0x2d162: map_regions * 8 random positions. The first own
   free site that allows a building is rated as building site, the
   first finished, active enemy military building of state 3 near own
   land as attack target; then the scan ends. */
void
ai_scan_sites(player_t *player)
{
	player_ai_t *ai = &player->ai;
	int n = W(game.map_regions * 8);
	unsigned own = PLAYER_HAS_CASTLE(player) ? own_bits(player) : 0;

	for (int i = 0; i < n; i++) {
		uint32_t r = (uint32_t)game_random_int() << 16;
		r |= game_random_int();
		map_pos_t pos = MAP_POS((r >> 2) & game.map.col_mask,
					(r >> (game.map.row_shift + 3)) &
					game.map.row_mask);

		unsigned owner = OWNERBITS(pos);
		if (owner == own) {
			if (SPACE(pos) != 0 || PATHBITS(pos) != 0) continue;

			ai_get_map_cursor_type_at(player, pos, own);
			if (ai->map_cursor_type < AI_CURSOR_CLEAR_BY_FLAG) continue;
			if (ai->panel_btn_type == AI_CAN_BUILD_NONE) continue;

			AI_SET_CURSOR(player, pos);
			uint32_t cat = site_categories[ai->panel_btn_type - 1];
			if (player->build & BIT(0)) cat &= ~0x600800; /* no military */
			if (player->build & BIT(1)) cat &= ~1; /* no flag */
			ai_scan_points_of_interest(player);
			ai_rate_building_site(player, cat);
			return;
		}

		if (!(owner & 0x80)) continue;
		int obj = MAP_OBJ(pos);
		if (obj < MAP_OBJ_SMALL_BUILDING || obj > MAP_OBJ_CASTLE) continue;

		building_t *b = game_get_building(MAP_OBJ_INDEX(pos));
		int t = BUILDING_TYPE(b);
		if (!BUILDING_IS_DONE(b) ||
		    (t != BUILDING_HUT && t != BUILDING_TOWER &&
		     t != BUILDING_FORTRESS && t != BUILDING_CASTLE)) {
			continue;
		}
		if (BUILDING_STATE(b) != 3 || !BUILDING_IS_ACTIVE(b)) continue;

		int near_own = 0;
		for (int j = 7; j < 7 + 0x102; j++) {
			if (OWNERBITS(spiral(b->pos, j)) == own_bits(player)) {
				near_own = 1;
				break;
			}
		}
		if (!near_own) continue;

		ai->panel_btn_type = AI_CAN_BUILD_NONE;
		AI_SET_CURSOR(player, pos);
		ai_scan_points_of_interest(player);
		ai_rate_attack_target(player);
		return;
	}
}

/* ai_wait @0x2d082: phase 1. */
static void
ai_wait(player_t *player)
{
	ai_scan_sites(player);
	player->ai.counter = W(player->ai.counter - 1);
	if (player->ai.counter == 0) {
		player->ai.phase = 2;
		player->ai.counter = 0xffff;
		player->ai.build_threshold = 0;
	}
}

/* Best stored site of a category: highest value (first one wins),
   NULL if all are 0. */
static ai_location_t *
best_location(player_t *player, int category)
{
	ai_location_t *best = NULL;
	unsigned value = 0;
	for (int i = 0; i < AI_LOCATIONS_PER_CATEGORY; i++) {
		ai_location_t *loc = &player->ai.locations[category][i];
		if (W(loc->value) > value) {
			value = W(loc->value);
			best = loc;
		}
	}
	return best;
}

/* ai_place_castle @0x2d09e: phase 0. */
static void
ai_place_castle(player_t *player)
{
	ai_scan_sites(player);

	unsigned tick = W(game.tick);
	if (tick < 2000) return;
	if (tick < 10000) {
		unsigned r = game_random_int();
		if (tick < 6000) {
			if (r & 0x3f) return;
		} else if (tick < 9000) {
			if (r & 0x1f) return;
		} else {
			if (r & 0xf) return;
		}
	}

	while (1) {
		ai_location_t *loc = best_location(player, BUILDING_CASTLE);
		if (loc == NULL) return;

		ai_game.some_location = loc;
		loc->value = 0;
		player->ai.cursor_col = loc->col;
		player->ai.cursor_row = loc->row;
		ai_determine_map_cursor_type(player);
		if (player->ai.panel_btn_type != AI_CAN_BUILD_CASTLE) continue;

		ai_scan_points_of_interest(player);
		unsigned rating = W(ai_rate_site_castle(player));
		if (rating < W(ai_game.some_location->value)) {
			/* Never taken: the value was just cleared. */
			int better = 0;
			for (int i = 0; i < AI_LOCATIONS_PER_CATEGORY; i++) {
				if (rating < W(player->ai.locations[BUILDING_CASTLE][i].value)) {
					better = 1;
					break;
				}
			}
			if (better) {
				ai_game.some_location->value = rating;
				continue;
			}
		}

		player->ai.phase = 1;
		player->ai.counter = 24;
		game_build_castle(AI_CURSOR_POS(player), player);
		return;
	}
}


/* ---- Build damping (0xb094) ---- */

/* ai_update_build_damping @0xb0c0: lower the build threshold by the
   elapsed ticks (but not below a value from the land/building ratio)
   and let the damping of every building type recover. */
static void
ai_update_build_damping(player_t *player, unsigned elapsed)
{
	/* Shift of the elapsed ticks per building type 1..25
	   (-1 = doubled). */
	static const int shift[25] = {
		3, 2, 4, 3, 1, 1, 1, 1, 2, 3, -1, 2, 2, 2, 2, 2,
		2, 2, 3, 2, 2, 2, 2, -1, 2
	};
	player_ai_t *ai = &player->ai;

	if (!PLAYER_IS_AI(player)) return;

	ai->build_threshold = ssub(ai->build_threshold, elapsed);

	if (player->total_building_score != 0) {
		unsigned div = player->total_building_score & 0xffff;
		uint32_t land = (uint32_t)player->total_land_area << 7;
		/* Original bug: divu.w traps on a zero low word and leaves
		   the dividend on overflow; both are skipped here. */
		if (div != 0 && land / div <= 0xffff) {
			unsigned q = land / div;
			if (q < 0x400) {
				unsigned v = W((q ^ 0x3ff) << 6);
				if (v >= W(ai->build_threshold)) {
					ai->build_threshold = v;
				}
			}
		}
	}

	for (int i = 0; i < 25; i++) {
		unsigned inc = shift[i] < 0 ? W(elapsed << 1) :
			W(elapsed) >> shift[i];
		ai->build_damp[i] = sadd(ai->build_damp[i], inc);
	}
}

/* ai_update_build_damping_all @0xb094 (scheduler slot 32). */
void
ai_update_build_damping_all(void)
{
	unsigned elapsed = W(ai_game.ticks_288); /* game+0x28a */
	ai_game.ticks_288 = 0;

	for (int i = 0; i < 4; i++) {
		player_t *player = game.player[i];
		if (PLAYER_IS_ACTIVE(player)) {
			ai_update_build_damping(player, elapsed);
		}
	}
}


/* ---- Settings (slot 0) ---- */

/* ai_adjust_flags @0x2a238: send-strongest flag, knight cycling,
   promotion of serfs to knights, castle knights and the knight
   occupation of the military buildings. */
static void
ai_adjust_flags(player_t *player)
{
	player_ai_t *ai = &player->ai;

	if (game_random_int() < W(player->ai_value_4)) {
		player->flags |= BIT(1);
	} else {
		player->flags &= ~BIT(1);
	}

	if (ai->u_1b0 == 0 && player->knight_cycle_counter == 0) {
		unsigned k0 = W(player->serf_count[SERF_KNIGHT_0]);
		unsigned k1 = W(player->serf_count[SERF_KNIGHT_1]);
		unsigned k2 = W(player->serf_count[SERF_KNIGHT_2]);
		unsigned v, d0, d1, d2;

		/* Knights 2..4 idle in inventories. */
		v = W(IDLE(SERF_KNIGHT_4)) << 1;
		if (v <= 0xffff) v += W(IDLE(SERF_KNIGHT_3));
		if (v <= 0xffff) v <<= 1;
		if (v <= 0xffff) v += W(IDLE(SERF_KNIGHT_2));
		d0 = v > 0xffff ? 0xffff : v;

		/* Knights 0..2 outside the inventories. */
		v = W(k0 - W(IDLE(SERF_KNIGHT_0))) << 1;
		if (v <= 0xffff) v += k1;
		if (v <= 0xffff) {
			v = W(W(v - W(IDLE(SERF_KNIGHT_1))) << 1) + k2;
			if (v <= 0xffff) v = W(v - W(IDLE(SERF_KNIGHT_2)));
		}
		d1 = v > 0xffff ? 0xffff : v;

		/* Capacity of the military buildings. */
		v = W(player->completed_building_count[BUILDING_FORTRESS]) << 1;
		if (v <= 0xffff) v += W(player->completed_building_count[BUILDING_TOWER]);
		if (v <= 0xffff) v <<= 1;
		if (v <= 0xffff) v += W(player->completed_building_count[BUILDING_HUT]);
		if (v <= 0xffff) v <<= 1;
		d2 = v > 0xffff ? 0xffff : v;

		if (d2 < d1 && d2 < d0) {
			/* Start cycling the knights. */
			player->flags |= BIT(2) | BIT(4);
			player->knight_cycle_counter = 1200;
			ai->u_1b0 = 15000;
		}
	}

	/* Promote generic serfs if swords and shields are there. */
	unsigned generic = 0, armed = 0;
	for (uint i = 0; i < game.max_inventory_index; i++) {
		if (!INVENTORY_ALLOCATED(i)) continue;
		inventory_t *inv = game_get_inventory(i);
		if (inv->player_num != (int)player->player_num) continue;
		unsigned n = W(inv->generic_count);
		generic = W(generic + n);
		if (n >= W(inv->resources[RESOURCE_SWORD])) n = W(inv->resources[RESOURCE_SWORD]);
		if (n >= W(inv->resources[RESOURCE_SHIELD])) n = W(inv->resources[RESOURCE_SHIELD]);
		armed = W(armed + n);
	}
	if (armed != 0 && generic >= 10) {
		generic = W(generic - armed);
		if (generic >= 10) {
			player_promote_serfs_to_knights(player, armed);
		} else {
			generic = 10 - generic;
			if (armed > generic) {
				player_promote_serfs_to_knights(player, armed - generic);
			}
		}
	}

	/* Military buildings by type (hut, tower, fortress) and state
	   (game+0x12c scratch in the original). */
	unsigned count[3][4] = {{0}};
	for (uint i = 1; i < game.max_building_index; i++) {
		/* Original bug: the bitmap bit is tested with D1 while D7
		   (left from game_random_int) is counted down, so the wrong
		   allocation bits are read. */
		if (!BUILDING_ALLOCATED(i)) continue;
		building_t *b = game_get_building(i);
		if (BUILDING_PLAYER(b) != (int)player->player_num) continue;
		/* Type byte & 0xfc: unfinished buildings never match. */
		if (!BUILDING_IS_DONE(b)) continue;
		int k;
		switch (BUILDING_TYPE(b)) {
		case BUILDING_HUT: k = 0; break;
		case BUILDING_TOWER: k = 1; break;
		case BUILDING_FORTRESS: k = 2; break;
		default: continue;
		}
		count[k][BUILDING_STATE(b)] = W(count[k][BUILDING_STATE(b)] + 1);
	}

	/* Knights wanted in the castle. */
	unsigned knights = W(player->serf_count[SERF_KNIGHT_0] +
			     player->serf_count[SERF_KNIGHT_1] +
			     player->serf_count[SERF_KNIGHT_2] +
			     player->serf_count[SERF_KNIGHT_3] +
			     player->serf_count[SERF_KNIGHT_4]);
	ai->u_1ac = knights;
	unsigned wanted = (knights >> 2) + 3;
	if (wanted >= 30) {
		wanted = (wanted >> 1) + 15;
		if (wanted >= 50) wanted = (wanted >> 1) + 25;
	}
	if (wanted >= 100) wanted = 99;
	player->castle_knights_wanted = wanted;
	knights = ssub(knights, wanted);
	knights -= knights >> 3;

	/* Highest occupation row the remaining knights can fill. */
	int level = 0;
	for (int r = 0; r < 16; r++) {
		unsigned need = 0;
		for (int k = 0; k < 4; k++) {
			int threat = 3 - k;
			int v = occupation_rows[r][k];
			need += knights_per_level[0][v] * count[0][threat];
			need += knights_per_level[1][v] * count[1][threat];
			need += knights_per_level[2][v] * count[2][threat];
			need = W(need);
		}
		if (knights >= need) {
			level = 16 - r;
			break;
		}
	}
	ai->u_1aa = level;
	if (W(player->ai_value_0) < (unsigned)level) level = W(player->ai_value_0);

	const uint8_t *row = occupation_rows[16 - level];
	for (int i = 3; i >= 0; i--) {
		int v = row[3 - i];
		int max = (v << 4) & 0xff;
		int min = v + i - 4;
		if (min < 0) min = 0;
		if (i == 3 && max == 0x30) min = 1;
		player->knight_occupation[i] = max | min;
	}
}

/* Tool priority from the stock of one tool (part of 0x2c906). */
static unsigned
tool_value(unsigned base, int shift, unsigned stock)
{
	if (W(stock) == 0) return 16;
	return ssub((base >> shift) + 1, stock);
}

/* ai_adjust_priorities @0x2c906: tool, planks, steel, coal, wheat
   and food distribution. */
static void
ai_adjust_priorities(player_t *player)
{
	static const int tool_shift[9] = { 2, 0, 2, 3, 2, 2, 3, 1, 3 };
	unsigned d0, d1, d2, d3, d4;

	d0 = player->total_land_area >= 0x1000 ? 0xfff :
		W(player->total_land_area);
	if (d0 < 0x400) d0 = 0x400;
	d0 >>= 8;

	d2 = 0;
	for (int i = 0; i < 9; i++) {
		unsigned v = tool_value(d0, tool_shift[i],
					STOCK(RESOURCE_SHOVEL + i));
		if (i == 0 || v > d2) d2 = v;
		player->tool_prio[i] = W(v * 0xfff);
	}

	d1 = W(d2 * 0xfff);
	player->planks_toolmaker = d1;
	player->steel_toolmaker = d1;
	d1 = W(~d1);
	d3 = sadd(d1, 0xfa0);

	d2 = 0xffff;
	switch (player->knight_occupation[3] & 0xf0) {
	case 0x40: d0 = 0; break;
	case 0x30: d0 = 0x7530; d2 = 0x7530; break;
	case 0x20: d0 = 0xc350; d2 = 0x4e20; break;
	case 0x10: d0 = 0xea60; d2 = 0x2710; break;
	default: d0 = 0xffff; d2 = 0x1388; break;
	}
	if (d1 >= d0) d0 = d1;
	player->steel_weaponsmith = d0;
	if (d3 >= d2) d2 = d3;
	if (W(STOCK(RESOURCE_STEEL)) >= 10) d2 = 0xffff;
	player->coal_goldsmelter = d2;
	d4 = d2;
	player->coal_weaponsmith = 0xafc8;

	d0 = W(STOCK(RESOURCE_STEEL));
	if (d0 >= 0x80) d0 = 0x7f;
	player->coal_steelsmelter = W(((0x7f - d0) << 8) + 0x8000);

	d0 = sadd(sadd(STOCK(RESOURCE_FISH), STOCK(RESOURCE_MEAT)),
		  STOCK(RESOURCE_BREAD));
	if (d0 >= 0x80) d0 = 0x7f;
	d0 = W(d0 << 9);
	if (d0 < 0x8000) {
		player->wheat_mill = 0xffff;
		player->wheat_pigfarm = W(d0 + 0x8000);
	} else {
		player->wheat_pigfarm = 0xffff;
		player->wheat_mill = W(W(~d0) + 0x8000);
	}

	d1 = 0xffff;
	d2 = W(ssub(8, STOCK(RESOURCE_BOAT)) << 11);
	d0 = W(player->planks_toolmaker);
	if (d0 >= 0xc000) {
		d2 = 0;
		d1 = W(~W((d0 - 0xc000) << 1));
	}
	player->planks_construction = d1;
	player->planks_boatbuilder = d2;

	/* Food for the mines: iron ore vs. gold ore (coal read but
	   overwritten in the original). */
	d1 = W(STOCK(RESOURCE_IRONORE));
	d0 = d1 >> 2;
	d1 = W(W(d1 << 1) - d0);
	d1 = sadd(d1, STOCK(RESOURCE_GOLDORE));
	d2 = W(STOCK(RESOURCE_STEEL));
	d2 = W(d2 - (d2 >> 2));
	d1 = W(d1 + d2);
	if (d1 >= d0) {
		d1 -= d0;
		if (d1 >= 0xdc) d1 = 0xdb;
		d1 = W(~(d1 << 8));
		d0 = 0xffff;
	} else {
		d0 -= d1;
		if (d0 >= 0xdc) d0 = 0xdb;
		d0 = W(~(d0 << 8));
		d1 = 0xffff;
	}

	d2 = W(STOCK(RESOURCE_STONE));
	if (d2 >= 0x18) d2 = 0x17;
	d2 = W(~(d2 << 11));
	player->food_stonemine = d2;
	if (d2 >= 0xafc8) {
		d0 >>= 1;
		d1 >>= 1;
		if (d2 >= 0xea60) {
			d0 >>= 1;
			d1 >>= 1;
		}
	}
	player->food_coalmine = d0;
	player->food_ironmine = d1;
	player->food_goldmine = d4 < d1 ? d4 : d1;
}

/* ai_update_settings @0x2c8fc: slot 0. */
static void
ai_update_settings(player_t *player)
{
	ai_adjust_flags(player);
	ai_adjust_priorities(player);
}


/* ---- Statistics (slots 1, 5, 9, 13) ---- */

/* ai_stock*_fill_* @0x2cef2..0x2cf7e: requested + 2 * available. */
static unsigned
stock_fill(const building_t *b, int i)
{
	return W(b->stock[i].requested + 2 * b->stock[i].available);
}

/* ai_update_building_stats @0x2ccc8. */
static void
ai_update_building_stats(player_t *player)
{
	/* Fill slot per building type: stock 0 and stock 1
	   (-1 none), capacity of stock 1 for the military buildings. */
	static const struct { int slot0, slot1, cap1; } bld_slots[24] = {
		{-1, -1, 0},	/* NONE */
		{-1, -1, 0},	/* FISHER */
		{-1, -1, 0},	/* LUMBERJACK */
		{0, -1, 0},	/* BOATBUILDER */
		{-1, -1, 0},	/* STONECUTTER */
		{1, -1, 0},	/* STONEMINE */
		{2, -1, 0},	/* COALMINE */
		{3, -1, 0},	/* IRONMINE */
		{4, -1, 0},	/* GOLDMINE */
		{-1, -1, 0},	/* FORESTER */
		{-1, -1, 0},	/* STOCK */
		{-1, 5, 4},	/* HUT */
		{-1, -1, 0},	/* FARM */
		{7, -1, 0},	/* BUTCHER */
		{6, -1, 0},	/* PIGFARM */
		{8, -1, 0},	/* MILL */
		{9, -1, 0},	/* BAKER */
		{-1, 10, 16},	/* SAWMILL */
		{15, 16, 16},	/* STEELSMELTER */
		{11, 12, 16},	/* TOOLMAKER */
		{14, 13, 16},	/* WEAPONSMITH */
		{-1, 5, 8},	/* TOWER */
		{-1, 5, 16},	/* FORTRESS */
		{17, 18, 16}	/* GOLDSMELTER */
	};
	unsigned fill[21] = {0}, cap[21] = {0};

	for (uint i = 1; i < game.max_building_index; i++) {
		if (!BUILDING_ALLOCATED(i)) continue;
		building_t *b = game_get_building(i);
		if (BUILDING_PLAYER(b) != (int)player->player_num) continue;

		if (!BUILDING_IS_DONE(b)) {
			/* Construction material: planks, stone. */
			fill[19] = W(fill[19] + stock_fill(b, 0));
			cap[19] = W(cap[19] + ((2 * b->stock[0].maximum) & 0xff));
			fill[20] = W(fill[20] + stock_fill(b, 1));
			cap[20] = W(cap[20] + ((2 * b->stock[1].maximum) & 0xff));
			continue;
		}

		int t = BUILDING_TYPE(b);
		if (t >= 24) continue;
		if (bld_slots[t].slot0 >= 0) {
			int s = bld_slots[t].slot0;
			fill[s] = W(fill[s] + stock_fill(b, 0));
			cap[s] = W(cap[s] + 16);
		}
		if (bld_slots[t].slot1 >= 0) {
			int s = bld_slots[t].slot1;
			fill[s] = W(fill[s] + stock_fill(b, 1));
			cap[s] = W(cap[s] + bld_slots[t].cap1);
		}
	}

	for (int s = 0; s < 21; s++) {
		unsigned v;
		if (fill[s] == cap[s]) {
			v = 0xffff;
		} else if (cap[s] == 0 || fill[s] > cap[s]) {
			/* Original bug: divu.w traps on 0 and on overflow
			   (fill > capacity) leaves 0. Taken as full here. */
			v = 0xffff;
		} else {
			v = ((uint32_t)fill[s] << 16) / cap[s];
		}
		FILL(s) = v;
	}

	/* Serfs idle in inventories by type. */
	for (int t = 0; t < 27; t++) IDLE(t) = 0;
	for (uint i = 1; i < game.max_serf_index; i++) {
		if (!SERF_ALLOCATED(i)) continue;
		serf_t *serf = game_get_serf(i);
		if (serf->state != SERF_STATE_IDLE_IN_STOCK) continue;
		if (SERF_PLAYER(serf) != (int)player->player_num) continue;
		IDLE(SERF_TYPE(serf)) = W(IDLE(SERF_TYPE(serf)) + 1);
	}

	/* Resources in all inventories, plus the emergency reserve. */
	for (int r = 0; r < 26; r++) STOCK(r) = 0;
	/* Original bug: the original adds these to build_want[7] and
	   build_want[9] (A1 already past the cleared array). */
	STOCK(RESOURCE_PLANK) = W(STOCK(RESOURCE_PLANK) + (player->extra_planks & 0xff));
	STOCK(RESOURCE_STONE) = W(STOCK(RESOURCE_STONE) + (player->extra_stone & 0xff));
	for (uint i = 0; i < game.max_inventory_index; i++) {
		if (!INVENTORY_ALLOCATED(i)) continue;
		inventory_t *inv = game_get_inventory(i);
		if (inv->player_num != (int)player->player_num) continue;
		for (int r = 25; r >= 0; r--) {
			STOCK(r) = sadd(STOCK(r), inv->resources[r]);
		}
	}
}


/* ---- Building ---- */

/* ai_check_construction_limit @0x2a504: < 0 if no builder can be
   had or too many buildings are under construction. */
int
ai_check_construction_limit(player_t *player)
{
	unsigned transporters = W(IDLE(SERF_TRANSPORTER));
	unsigned generic = W(IDLE(SERF_GENERIC));

	if (W(IDLE(SERF_BUILDER)) != 0) {
		if (W(transporters + generic) < 2) return -1;
	} else {
		if (W(STOCK(RESOURCE_HAMMER)) == 0) return -1;
		if (generic == 0) return -1;
		if (W(generic + transporters) < 3) return -1;
	}

	unsigned incomplete = 0;
	for (int t = BUILDING_FISHER; t <= BUILDING_GOLDSMELTER; t++) {
		incomplete = W(incomplete + player->incomplete_building_count[t]);
	}

	unsigned limit = W((W(player->completed_building_count[BUILDING_STOCK]) + 3) << 2);
	unsigned v = (W(STOCK(RESOURCE_PLANK)) >> 2) + 6;
	if (v < limit) limit = v;
	v = (W(STOCK(RESOURCE_STONE)) >> 1) + 8;
	if (v < limit) limit = v;

	return limit < incomplete ? -1 : 0;
}

/* Road builder parameters set before ai_build_road. */
static void
road_params(player_t *player, int u_1ba, int u_19c, int u_1a8, int u_1a4,
	    int u_19e)
{
	player->ai.u_1ba = u_1ba;
	player->ai.u_19c = u_19c;
	player->ai.u_1a8 = u_1a8;
	player->ai.u_1a4 = u_1a4;
	player->ai.u_19e = u_19e;
}

/* Move the AI cursor by (dc, dr) with wrap-around. */
static void
move_cursor(player_t *player, int dc, int dr)
{
	player->ai.cursor_col = (player->ai.cursor_col + dc) & game.map.col_mask;
	player->ai.cursor_row = (player->ai.cursor_row + dr) & game.map.row_mask;
}

/* ai_build_building @0x2a5c4: build ai_game.build_building_type at
   the best stored site that is still possible. Type 25 builds only a
   flag (category 0), type 24 sends a geologist (category 25) once the
   castle exists. */
void
ai_build_building(player_t *player)
{
	player_ai_t *ai = &player->ai;
	int mode = 2;		/* D5: -1 flag, 0 geologist, 2/3 building */
	unsigned allowed = 0x18;	/* D6: panel_btn_type bits */
	int type = ai_game.build_building_type;

	if ((0x3ff7400 >> (type & 31)) & 1) {
		if (type >= 24) {
			if (type == 25) {
				ai_game.build_building_type = 0;
				mode = -1;
				allowed = BIT(AI_CAN_BUILD_FLAG);
			} else if (PLAYER_HAS_CASTLE(player)) {
				ai_game.build_building_type = 25;
				mode = 0;
				allowed = BIT(AI_CAN_BUILD_MINE);
			} else {
				mode = 3;
				allowed = BIT(AI_CAN_BUILD_LARGE);
			}
		} else {
			mode = 3;
			allowed = BIT(AI_CAN_BUILD_LARGE);
		}
	} else if (type >= 5 && type < 9) {
		allowed = BIT(AI_CAN_BUILD_MINE);
	}
	ai_game.g_24a = mode;
	ai_game.g_24c = allowed;

	unsigned rating;
	while (1) {
		int category = ai_game.build_building_type;
		ai_location_t *loc = best_location(player, category);
		if (loc == NULL) return;

		ai_game.some_location = loc;
		loc->value = 0;
		ai->cursor_col = loc->col;
		ai->cursor_row = loc->row;
		ai_determine_map_cursor_type(player);
		if (ai->map_cursor_type < AI_CURSOR_CLEAR_BY_FLAG) continue;
		if (!(allowed & BIT(ai->panel_btn_type & 7))) continue;
		if ((player->build & BIT(0)) &&
		    (category == BUILDING_HUT || category == BUILDING_TOWER ||
		     category == BUILDING_FORTRESS)) {
			continue;
		}

		ai_scan_points_of_interest(player);
		rating = W(ai_rate_site_dispatch(player));
		if (rating < W(ai_game.some_location->value)) {
			/* Never taken: the value was just cleared. */
			int better = 0;
			for (int i = 0; i < AI_LOCATIONS_PER_CATEGORY; i++) {
				if (rating < W(ai->locations[category][i].value)) {
					better = 1;
					break;
				}
			}
			if (better) {
				ai_game.some_location->value = rating;
				continue;
			}
		}
		break;
	}

	if (rating == 0) return;

	if (mode < 0) {
		/* Flag only */
		game_build_flag(AI_CURSOR_POS(player), player);
		if (rating < 0x9470) {
			ai_pull_roads_through_flag(player);
			player->build |= BIT(4);
			road_params(player, 0, 0, 0, 0, -1);
		} else {
			player->build &= ~BIT(4);
			road_params(player, 0, 0, 0, 6, -1);
		}
		ai_build_road(player);
		return;
	}

	road_params(player, 0, 0, 12, ai->u_1a4, -1);
	if (ai->map_cursor_type != AI_CURSOR_CLEAR) {
		int flag_with_paths = 1;
		if (ai->map_cursor_type == AI_CURSOR_CLEAR_BY_FLAG) {
			map_pos_t fp = MAP_POS((ai->cursor_col + 1) & game.map.col_mask,
					       (ai->cursor_row + 1) & game.map.row_mask);
			flag_with_paths = PATHBITS(fp) != 0;
		}
		if (flag_with_paths) {
			ai->u_19e = 0;
			ai->u_1ba = 30;
			ai->u_19c = 12;
		}
	}

	if (mode == 0) {
		/* Geologist: flag at the building flag position. */
		move_cursor(player, 1, 1);
		if (ai->map_cursor_type >= AI_CURSOR_CLEAR_BY_PATH) {
			if (ai->map_cursor_type == AI_CURSOR_CLEAR_BY_PATH) {
				/* game_build_flag splits the path. */
				ai->map_cursor_type = AI_CURSOR_PATH;
			}
			game_build_flag(AI_CURSOR_POS(player), player);
			if (ai_pull_roads_through_flag(player) >= 0) {
				ai->u_1ba = 50;
				ai->u_19c = 8;
				ai->u_19e = 0;
			}
			ai->u_1a4 = 0;
			if (ai->u_19c != 0) {
				ai->u_19c = 8;
			} else {
				ai->u_1a8 = 8;
			}
			player->build &= ~BIT(4);
			if (ai_build_road(player) < 0) {
				/* legacy game_build_flag may have refused. */
				map_pos_t fpos = AI_CURSOR_POS(player);
				if (MAP_OBJ(fpos) == MAP_OBJ_FLAG &&
				    !FLAG_HAS_BUILDING(game_get_flag(MAP_OBJ_INDEX(fpos)))) {
					demolish_flag(fpos);
				}
				return;
			}
		}
		map_pos_t pos = AI_CURSOR_POS(player);
		if (MAP_HAS_FLAG(pos)) {
			game_send_geologist(game_get_flag(MAP_OBJ_INDEX(pos)));
		}
		return;
	}

	/* Building: raise the threshold for the next one. */
	unsigned land = player->total_land_area >= 0x1000 ? 0xfff :
		W(player->total_land_area);
	ai->build_threshold = sadd(ai->build_threshold, W(0x3000 - 2 * land));

	game_build_building(AI_CURSOR_POS(player),
			    (building_type_t)ai_game.build_building_type, player);
	move_cursor(player, 1, 1);
	if (ai_pull_roads_through_flag(player) >= 0) {
		ai->u_1ba = 70;
		ai->u_19c = 12;
		ai->u_19e = 0;
	}
	ai->u_1a4 = 0;
	player->build &= ~BIT(4);
	if (ai_build_road(player) < 0) {
		map_pos_t pos = MAP_POS((ai->cursor_col - 1) & game.map.col_mask,
					(ai->cursor_row - 1) & game.map.row_mask);
		/* legacy game_build_building may have refused. */
		if (MAP_OBJ(pos) >= MAP_OBJ_SMALL_BUILDING &&
		    MAP_OBJ(pos) <= MAP_OBJ_CASTLE) {
			demolish_building(pos);
		}
	}
}

/* Slots 2, 6, 10, 11, 14 of ai_update (0x28f66): compute the build
   wants and build the most wanted type. */
static void
ai_want_and_build(player_t *player)
{
	player_ai_t *ai = &player->ai;

	if (ai_check_construction_limit(player) < 0) {
		/* Original bug: the stale want of type 25 is tested here. */
		ai->build_want[24] = 0;
		ai_want_type25(player);
		if (ai->build_want[24] != 0) {
			ai_game.build_building_type = 25;
			ai_build_building(player);
			return;
		}
		ai_want_castle(player);
		if (W(ai->build_want[23]) < 10000) {
			ai_scan_sites(player);
			return;
		}
		ai_game.build_building_type = 24;
		ai_build_building(player);
		return;
	}

	/* Original bug: only the first 24 words are cleared (0x28f9e), so
	   the want of type 25 stays set and wins every later choice. */
	for (int i = 0; i < 25; i++) ai->build_want[i] = 0;

	if (BIT_TEST(player->emergency_flags, 6)) {
		if (!BIT_TEST(player->emergency_flags, 3) &&
		    W(player->lumberjack_index) == 0) {
			ai_want_lumberjack(player);
		}
		if (!BIT_TEST(player->emergency_flags, 4) &&
		    W(player->sawmill_index) == 0) {
			ai_want_sawmill(player);
		}
		if (!BIT_TEST(player->emergency_flags, 5) &&
		    W(player->stonecutter_index) == 0) {
			ai_want_stonecutter(player);
		}
	} else {
		ai_want_type25(player);
		ai_want_castle(player);
		ai_want_fisher(player);
		ai_want_lumberjack(player);
		ai_want_boatbuilder(player);
		ai_want_stonecutter(player);
		ai_want_stonemine(player);
		ai_want_goldmine(player);
		ai_want_coalmine(player);
		ai_want_ironmine(player);
		ai_want_forester(player);
		ai_want_hut(player);
		ai_want_farm(player);
		ai_want_pigfarm(player);
		ai_want_mill(player);
		if (W(IDLE(SERF_DIGGER)) == 0 &&
		    W(STOCK(RESOURCE_SHOVEL)) == 0) {
			/* No leveling possible. */
			ai->build_want[BUILDING_FARM - 1] = 0;
			ai->build_want[BUILDING_PIGFARM - 1] = 0;
		} else {
			ai_want_stock(player);
			ai_want_butcher(player);
			ai_want_baker(player);
			ai_want_sawmill(player);
			ai_want_steelsmelter(player);
			ai_want_toolmaker(player);
			ai_want_weaponsmith(player);
			ai_want_tower(player);
			ai_want_fortress(player);
			ai_want_goldsmelter(player);
		}
	}

	unsigned best = 0;
	int type = -1;
	for (int t = 1; t <= 25; t++) {
		if (W(ai->build_want[t - 1]) > best) {
			best = W(ai->build_want[t - 1]);
			type = t;
		}
	}
	if (type < 0) return;
	if (best < W(ai->build_threshold)) return;

	ai_game.build_building_type = type;
	ai->build_damp[type - 1] = W(ai->build_damp[type - 1]) >> 1;
	ai_build_building(player);
}

/* ai_find_flag_connection on the flag at pos (the original passes it
   in A0 and D0/D1). budget is the caller's D7: 0 when a road was
   built, -600 on failure, unchanged if the flag has all paths. */
static void
find_flag_connection_at(player_t *player, map_pos_t pos, int *budget)
{
	AI_SET_CURSOR(player, pos);
	int r = ai_find_flag_connection(player);
	if (r == 0) {
		*budget = 0;
	} else if (r < 0) {
		*budget = (int16_t)W(*budget - 600);
	}
}

static int
asr1(int v)
{
	return v < 0 ? -((1 - v) / 2) : v / 2;
}

/* Slots 7 and 15 of ai_update (0x290b0): connect the pending
   positions (conquered buildings) to the own road network, then try
   to connect own flags found by a linear map scan. */
static void
ai_connect_roads(player_t *player)
{
	player_ai_t *ai = &player->ai;
	int budget = 1000;	/* D7 */

	for (int e = 0; e < 8; e++) {
		int *pcol = ai_pending(player, e, 0);
		int *prow = ai_pending(player, e, 1);
		if (*pcol & 0x8000) break;

		int ecol = *pcol & game.map.col_mask;
		int erow = *prow & game.map.row_mask;
		map_pos_t epos = MAP_POS(ecol, erow);

		/* Nearest own flag on the rings 1..16. */
		map_pos_t pos = epos;
		int ring = -1;
		for (int r = 0; r < 16 && ring < 0; r++) {
			pos = MAP_MOVE_DOWN_RIGHT(pos);
			for (int d = DIR_UP; d >= DIR_RIGHT && ring < 0; d--) {
				for (int s = 0; s <= r; s++) {
					pos = MAP_MOVE(pos, d);
					if (MAP_HAS_FLAG(pos) &&
					    OWNERBITS(pos) == own_bits(player)) {
						ring = r;
						break;
					}
				}
			}
		}

		if (ring < 0) {
			budget = (int16_t)W(budget - 50);
		} else if (ring < 8) {
			ai->cursor_col = ecol;
			ai->cursor_row = erow;
			road_params(player, 0, 0, 12, 0, -1);
			player->build &= ~BIT(4);
			ai_build_road(player);
			budget = -1;
		} else {
			/* Flag halfway to the found flag. */
			int dc = (MAP_POS_COL(pos) - ecol) & game.map.col_mask;
			if (dc >= (int)(game.map.cols / 2)) dc -= game.map.cols;
			int dr = (MAP_POS_ROW(pos) - erow) & game.map.row_mask;
			if (dr >= (int)(game.map.rows / 2)) dr -= game.map.rows;
			map_pos_t mid = MAP_POS((ecol + asr1(dc)) & game.map.col_mask,
						(erow + asr1(dr)) & game.map.row_mask);

			int built = 0;
			for (int i = 0; i < 37; i++) {
				map_pos_t p = spiral(mid, i);
				AI_SET_CURSOR(player, p);
				ai_determine_map_cursor_type(player);
				if (ai->map_cursor_type < AI_CURSOR_PATH ||
				    ai->panel_btn_type < AI_CAN_BUILD_FLAG ||
				    (player->build & BIT(1))) {
					continue;
				}

				game_build_flag(p, player);
				ai->cursor_col = ecol;
				ai->cursor_row = erow;
				road_params(player, 0, 0, 12, 0, -1);
				player->build &= ~BIT(4);
				ai_build_road(player);
				if (MAP_HAS_FLAG(p)) {
					int dummy = budget;
					find_flag_connection_at(player, p, &dummy);
				}
				budget = -1;
				built = 1;
				break;
			}
			if (!built) budget = (int16_t)W(budget - 50);
		}

		*pcol = -1;
		*prow = -1;
	}

	if (budget < 0) return;

	if (W(IDLE(SERF_TRANSPORTER) + IDLE(SERF_GENERIC)) < 2) {
		ai_scan_sites(player);
		return;
	}

	/* Linear scan of budget + 1 positions from ptr+0x1a0. */
	map_pos_t pos = (map_pos_t)ai->u_1a0 & (game.map.tile_count - 1);
	do {
		if (MAP_HAS_FLAG(pos) &&
		    OWNERBITS(pos) == own_bits(player)) {
			find_flag_connection_at(player, pos, &budget);
		}
		pos = (pos + 1) & (game.map.tile_count - 1);
		budget = (int16_t)W(budget - 1);
	} while (budget >= 0);
	ai->u_1a0 = pos;
}


/* ---- Phase machine ---- */

/* ai_update @0x28ee2: one step of the computer player. */
void
ai_update(player_t *player)
{
	player_ai_t *ai = &player->ai;

	if (game.game_speed == 0) return;

	switch (ai->phase) {
	case 0: ai_place_castle(player); return;
	case 1: ai_wait(player); return;
	case 2: break;
	default: return;
	}

	ai->counter = W(ai->counter + 1);
	if (ai->counter & 7) {
		ai_scan_sites(player);
		return;
	}

	switch ((ai->counter & 0x78) >> 3) {
	case 0: ai_update_settings(player); break;
	case 1: case 5: case 9: case 13:
		ai_update_building_stats(player);
		break;
	case 2: case 6: case 10: case 11: case 14:
		ai_want_and_build(player);
		break;
	case 3: ai_manage_buildings(player); break;
	case 4: case 12: ai_attack(player); break;
	case 7: case 15: ai_connect_roads(player); break;
	case 8: ai_validate_locations(player); break;
	}
}
