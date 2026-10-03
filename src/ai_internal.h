/*
 * ai_internal.h - Shared declarations of the computer player modules
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

/* The original AI passes most values through fields of the player
   record and a few global game fields instead of arguments. The port
   keeps that structure: player fields live in player->ai (ai.h), the
   global ones in ai_game below. Comments give the Amiga addresses. */

#ifndef _AI_INTERNAL_H
#define _AI_INTERNAL_H

#include "game.h"
#include "player.h"
#include "flag.h"
#include "building.h"
#include "serf.h"
#include "map.h"

/* Values of player->ai.map_cursor_type (= map_cursor_type_t). */
#define AI_CURSOR_NONE            0
#define AI_CURSOR_FLAG            1
#define AI_CURSOR_REMOVABLE_FLAG  2
#define AI_CURSOR_BUILDING        3
#define AI_CURSOR_PATH            4
#define AI_CURSOR_CLEAR_BY_FLAG   5
#define AI_CURSOR_CLEAR_BY_PATH   6
#define AI_CURSOR_CLEAR           7

/* Values of player->ai.panel_btn_type (= build_possibility_t). */
#define AI_CAN_BUILD_NONE    0
#define AI_CAN_BUILD_FLAG    1
#define AI_CAN_BUILD_MINE    2
#define AI_CAN_BUILD_SMALL   3
#define AI_CAN_BUILD_LARGE   4
#define AI_CAN_BUILD_CASTLE  5

/* Global AI work fields of the Amiga game record. */
typedef struct {
	int g_246;		/* game+0x246 */
	int g_248;		/* game+0x248 */
	int g_24a;		/* game+0x24a (col of the road end / flag built) */
	int g_24c;		/* game+0x24c (row) */
	int g_24e;		/* game+0x24e */
	ai_location_t *some_location;	/* game+0x254 */
	int build_building_type;	/* game+0x27a */
	int ticks_288;		/* game+0x288, ticks for ai_update_build_damping */
	/* game+0x12c land_influence, used by the AI as a scratch byte
	   array (indexed like the Amiga byte offsets). */
	uint8_t land_influence[8192];
} ai_game_t;

extern ai_game_t ai_game;

#define AI_CURSOR_POS(player)  MAP_POS((player)->ai.cursor_col, (player)->ai.cursor_row)
#define AI_SET_CURSOR(player, pos)  do { \
		(player)->ai.cursor_col = MAP_POS_COL(pos); \
		(player)->ai.cursor_row = MAP_POS_ROW(pos); \
	} while (0)

/* ---- ai.c (core: 0x28ee2 ai_update and friends) ---- */
/* ai_update @0x28ee2: one AI step of player (called by the scheduler). */
void ai_update(player_t *player);
/* ai_update_build_damping_all @0xb094 (scheduler slot 32). */
void ai_update_build_damping_all(void);
/* determine_map_cursor_type @0x19368 for player->ai.cursor: sets
   ai.map_cursor_type, ai.panel_btn_type and the PLAYER_ALLOW_* bits
   of player->build. */
void ai_determine_map_cursor_type(player_t *player);
/* player_ai @0x2d162: random site scan. */
void ai_scan_sites(player_t *player);
/* ai_build_building @0x2a5c4: build ai_game.build_building_type. */
void ai_build_building(player_t *player);
int ai_check_construction_limit(player_t *player);

/* ---- ai_road.c ---- */
/* ai_build_road @0x2a8f2: road from the cursor; parameters in
   ai.u_19c, u_19e, u_1a4, u_1a8, u_1ba and player->build bit 4.
   Returns < 0 on failure (the original's N flag). */
int ai_build_road(player_t *player);
/* ai_find_flag_connection @0x29316. */
int ai_find_flag_connection(player_t *player);
/* ai_pull_roads_through_flag @0x271d4 at the cursor; < 0 on failure. */
int ai_pull_roads_through_flag(player_t *player);

/* ---- ai_manage.c ---- */
void ai_manage_buildings(player_t *player);	/* 0x294ae */
void ai_validate_locations(player_t *player);	/* 0x29a70 */
void ai_attack(player_t *player);		/* 0x29de2 */
/* Military ratio of player_update_knight_morale @0xb4bc. */
int ai_calc_military_ratio(player_t *player);

/* ---- ai_want.c (0x2b8c4..0x2cc5c) ---- */
/* Each writes player->ai.build_want[type - 1]. */
void ai_want_type25(player_t *player);
void ai_want_fisher(player_t *player);
void ai_want_lumberjack(player_t *player);
void ai_want_boatbuilder(player_t *player);
void ai_want_stonecutter(player_t *player);
void ai_want_stonemine(player_t *player);
void ai_want_coalmine(player_t *player);
void ai_want_ironmine(player_t *player);
void ai_want_goldmine(player_t *player);
void ai_want_forester(player_t *player);
void ai_want_stock(player_t *player);
void ai_want_hut(player_t *player);
void ai_want_farm(player_t *player);
void ai_want_butcher(player_t *player);
void ai_want_pigfarm(player_t *player);
void ai_want_mill(player_t *player);
void ai_want_baker(player_t *player);
void ai_want_sawmill(player_t *player);
void ai_want_steelsmelter(player_t *player);
void ai_want_toolmaker(player_t *player);
void ai_want_weaponsmith(player_t *player);
void ai_want_tower(player_t *player);
void ai_want_fortress(player_t *player);
void ai_want_goldsmelter(player_t *player);
void ai_want_castle(player_t *player);

/* ---- ai_rate.c (0x2d316..0x2eda6) ---- */
/* ai_scan_points_of_interest @0x2e88e: fills player->ai.poi[] around
   the cursor, depending on ai.panel_btn_type. */
void ai_scan_points_of_interest(player_t *player);
/* ai_rate_building_site @0x2d858: categories = bit mask of allowed
   site categories (D2 in the original). */
void ai_rate_building_site(player_t *player, uint32_t categories);
/* ai_rate_attack_target @0x2d3de. */
void ai_rate_attack_target(player_t *player);
/* ai_rate_site_dispatch @0x2b830: rating of the cursor site for
   category ai_game.build_building_type (returned D7). */
int ai_rate_site_dispatch(player_t *player);
/* ai_rate_site_castle @0x2e68a, called directly by ai_place_castle. */
int ai_rate_site_castle(player_t *player);

/* ---- legacy game.c functions used by the AI ---- */
int demolish_building(map_pos_t pos);
int demolish_flag(map_pos_t pos);
void schedule_slot_to_unknown_dest(flag_t *flag, int slot);
int find_nearest_inventory(flag_t *flag);
int get_road_length_value(int length);
uint16_t game_random_int(void);

#endif /* !_AI_INTERNAL_H */
