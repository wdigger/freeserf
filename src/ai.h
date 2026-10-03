/*
 * ai.h - Computer player, ported from the Amiga original
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

/* AI state of a player. Field comments give the offset relative to the
   Amiga player pointer (which points 0x80 bytes into the player record),
   the same convention as the hex comments in player.h. All values are
   16-bit words in the original unless noted otherwise. */

#ifndef _AI_H
#define _AI_H

#include <stdint.h>

/* Area statistics of one distance ring set (Amiga point_of_interest_t,
   88 bytes). */
typedef struct {
	int field_0;		/* +0x00 */
	int field_2;		/* +0x02 */
	int field_4;		/* +0x04 */
	int field_6;		/* +0x06 */
	int field_8;		/* +0x08 */
	int bld_count[25];	/* +0x0a buildings by type */
	int trees;		/* +0x3c */
	int field_3e;		/* +0x3e */
	int stones;		/* +0x40 */
	int deposit[4];		/* +0x42 gold, iron, coal, stone */
	int field_4a;		/* +0x4a */
	int field_4c;		/* +0x4c */
	int field_4e;		/* +0x4e */
	int field_50;		/* +0x50 */
	int field_52;		/* +0x52 */
	int field_54;		/* +0x54 */
	int field_56;		/* +0x56 */
} ai_poi_t;

/* Stored candidate site (Amiga ai_location_t). */
typedef struct {
	int value;
	int col;
	int row;
} ai_location_t;

#define AI_LOCATION_CATEGORIES  35
#define AI_LOCATIONS_PER_CATEGORY  8

typedef struct {
	/* Cursor of the computer player (Amiga ptr+0xfc..0x101, the same
	   fields the human player's panel uses). */
	int cursor_col;		/* 0xfc */
	int cursor_row;		/* 0xfe */
	int map_cursor_type;	/* 0x100 (byte), map_cursor_type_t */
	int panel_btn_type;	/* 0x101 (byte), build_possibility_t */

	/* Unknown words 0x19a..0x1ac (road builder parameters etc.). */
	int u_19a, u_19c, u_19e, u_1a0, u_1a2, u_1a4, u_1a6, u_1a8,
	    u_1aa, u_1ac;

	int military_ratio;	/* 0x186: own vs. others' military strength */

	int u_1b0;		/* 0x1b0 */
	int u_1b2;		/* 0x1b2 */
	int phase;		/* 0x1b4: 0 place castle, 1 wait, 2 running, 3 off */
	int counter;		/* 0x1b6 */
	int build_threshold;	/* 0x1b8 */

	/* Unknown words 0x1ba..0x1da. */
	int u_1ba, u_1bc, u_1be, u_1c0, u_1c2, u_1c4, u_1c6, u_1c8,
	    u_1ca, u_1cc, u_1ce, u_1d0, u_1d2, u_1d4, u_1d6, u_1d8,
	    u_1da;

	ai_poi_t poi[5];	/* 0x1dc, 0x234, 0x28c, 0x2e4, 0x33c */

	/* Unknown words 0x394..0x3ce. */
	int u_394, u_396, u_398, u_39a, u_39c, u_39e, u_3a0, u_3a2,
	    u_3a4, u_3a6, u_3a8, u_3aa, u_3ac, u_3ae, u_3b0, u_3b2,
	    u_3b4, u_3b6, u_3b8, u_3ba, u_3bc, u_3be, u_3c0, u_3c2,
	    u_3c4, u_3c6, u_3c8, u_3ca, u_3cc, u_3ce;

	int build_want[25];	/* 0x3d0: index = building type - 1 */
	int build_damp[25];	/* 0x402: index = building type - 1 */
	ai_location_t locations[AI_LOCATION_CATEGORIES][AI_LOCATIONS_PER_CATEGORY]; /* 0x434 */
} player_ai_t;

#endif /* !_AI_H */
