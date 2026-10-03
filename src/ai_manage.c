/*
 * ai_manage.c - Computer player: building management, location
 *               validation and attacks (ported from the Amiga original)
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

#include "ai_internal.h"
#include "resource.h"

/* Amiga map_space_from_obj table @0x1f42 (game+0x150). Its encoding
   differs from legacy map_space_from_obj (findings MAP-12): 0 open,
   1 filled, 2 semipassable/impassable, 3 flag, 4/5/6 small/large
   building/castle, 0xff for object 127. */
static const uint8_t ai_space_from_obj[128] = {
	0, 3, 4, 5, 6, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 1, 0, 0, 0, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1,
	1, 0, 1, 1, 1, 1, 0, 1, 1, 2, 2, 2, 2, 2, 2, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 255
};

#define AI_SPACE(pos)  (ai_space_from_obj[MAP_OBJ(pos)])

/* Height byte of the original: bit 7 has owner, bits 5-6 owner.
   Compared with (player + 4) << 5 by the original. */
#define AI_OWNED_BY(pos, num)  (MAP_HAS_OWNER(pos) && MAP_OWNER(pos) == (uint)(num))

/* 16-bit unsigned add / double with saturation at 0xffff (the
   original's add.w; bcs -> moveq -1). */
static unsigned int
sat_add16(unsigned int a, unsigned int b)
{
	unsigned int r = (a & 0xffff) + (b & 0xffff);
	return r > 0xffff ? 0xffff : r;
}

static int
is_military_type(building_type_t type)
{
	return type == BUILDING_HUT || type == BUILDING_TOWER ||
	       type == BUILDING_FORTRESS || type == BUILDING_CASTLE;
}


/* 0x29a4c: demolish the building at pos (D3 in the original). */
static void
ai_demolish_building(map_pos_t pos)
{
	demolish_building(pos);
}

/* The handlers below get the remaining work budget of
   ai_manage_buildings (D7w) and return the new budget. -1 then
   subtracting marks "stop" after a demolition. */

/* 0x29590 */
static int
ai_manage_fisher(player_t *player, building_t *building, int budget)
{
	for (int i = 1; i <= 64; i++) {
		map_pos_t pos = MAP_POS_ADD(building->pos,
					    game.spiral_pos_pattern[i]);
		/* The original tests the water marker (obj bit 7), the
		   water flag (paths bit 6) and fish != 0; legacy keeps
		   neither marker, MAP_IN_WATER is its counterpart (as in
		   map_update_hidden). */
		if (MAP_IN_WATER(pos) && MAP_RES_FISH(pos) != 0) {
			return budget - 10;
		}
	}

	ai_demolish_building(building->pos);
	return -1 - 10;
}

/* 0x295ca */
static int
ai_manage_lumberjack(player_t *player, building_t *building, int budget)
{
	for (int i = 1; i <= 128; i++) {
		map_pos_t pos = MAP_POS_ADD(building->pos,
					    game.spiral_pos_pattern[i]);
		int obj = MAP_OBJ(pos);
		if (obj >= MAP_OBJ_TREE_0 && obj < MAP_OBJ_TREE_0 + 16) {
			return budget - 20;
		}
	}

	ai_demolish_building(building->pos);
	return -1 - 20;
}

/* 0x29606 */
static int
ai_manage_stonecutter(player_t *player, building_t *building, int budget)
{
	for (int i = 1; i <= 128; i++) {
		map_pos_t pos = MAP_POS_ADD(building->pos,
					    game.spiral_pos_pattern[i]);
		int obj = MAP_OBJ(pos);
		if (obj >= MAP_OBJ_STONE_0 && obj < MAP_OBJ_STONE_0 + 8) {
			return budget - 20;
		}
	}

	ai_demolish_building(building->pos);
	return -1 - 20;
}

/* Shared body of the mine handlers 0x29640 (stone), 0x29672 (coal),
   0x296a4 (iron), 0x296d6 (gold): keep the mine while one of the 32
   nearest positions still has its deposit type. */
static int
ai_manage_mine(player_t *player, building_t *building, int budget,
	       ground_deposit_t deposit)
{
	for (int i = 1; i <= 32; i++) {
		map_pos_t pos = MAP_POS_ADD(building->pos,
					    game.spiral_pos_pattern[i]);
		if (MAP_RES_TYPE(pos) == deposit) return budget - 5;
	}

	ai_demolish_building(building->pos);
	return -1 - 5;
}

/* 0x29708 */
static int
ai_manage_forester(player_t *player, building_t *building, int budget)
{
	/* Original bug: the loop reuses D3 (the building position) as a
	   scratch byte, so after the first position it scans around a
	   corrupted base and finally demolishes at a corrupted position.
	   The port scans around the building and demolishes it. */
	for (int i = 1; i <= 128; i++) {
		map_pos_t pos = MAP_POS_ADD(building->pos,
					    game.spiral_pos_pattern[i]);
		if ((game.map.tiles[pos].paths & 0x7f) == 0 &&
		    MAP_OBJ(pos) == MAP_OBJ_NONE &&
		    game.map.tiles[pos].type == 0x55) {
			return budget - 20;
		}
	}

	ai_demolish_building(building->pos);
	return -1 - 20;
}

/* Thresholds of ai_manage_stock @0x298f0, indexed by the military
   state of the nearby military building: resource sum out / stop,
   generic serfs out / stop. */
static const unsigned int stock_thresholds[4][4] = {
	{ 15000, 10000, 200, 100 },
	{  4000,  3000, 200, 100 },
	{  1000,   600, 200, 100 },
	{   500,     0,   0,   0 }
};

static int
find_res_dest_cb(flag_t *flag, void *data)
{
	return FLAG_ACCEPTS_RESOURCES(flag);
}

static int
find_serf_dest_cb(flag_t *flag, void *data)
{
	return FLAG_ACCEPTS_SERFS(flag);
}

/* 0x239fa as used by the AI: < 0 if the flag itself accepts resources
   or no other flag accepting resources is reachable over roads with
   transporters. (Legacy schedule_slot_to_unknown_dest is a different
   function.) */
static int
ai_find_other_resource_inventory(flag_t *flag)
{
	if (FLAG_ACCEPTS_RESOURCES(flag)) return -1;
	return flag_search_single(flag, find_res_dest_cb, 0, 1, NULL) == 0 ?
		0 : -1;
}

/* 0x238fa as used by the AI: < 0 if the flag itself accepts serfs or
   no other flag accepting serfs is reachable over land paths. */
static int
ai_find_other_serf_inventory(flag_t *flag)
{
	if (FLAG_ACCEPTS_SERFS(flag)) return -1;
	return flag_search_single(flag, find_serf_dest_cb, 1, 0, NULL) == 0 ?
		0 : -1;
}

/* 0x2974c: move resources and serfs out of a stock near the front. */
static int
ai_manage_stock(player_t *player, building_t *building, int budget)
{
	if (!BUILDING_HAS_SERF(building)) return budget;

	for (int i = 7; i < 7 + 0x102; i++) {
		map_pos_t pos = MAP_POS_ADD(building->pos,
					    game.spiral_pos_pattern[i]);
		int obj = MAP_OBJ(pos);
		if (obj < MAP_OBJ_SMALL_BUILDING || obj > MAP_OBJ_CASTLE) continue;

		building_t *mil = game_get_building(MAP_OBJ_INDEX(pos));
		if (BUILDING_PLAYER(mil) != (int)player->player_num ||
		    !BUILDING_IS_ACTIVE(mil) ||
		    !is_military_type(BUILDING_TYPE(mil))) {
			continue;
		}

		const unsigned int *thr = stock_thresholds[BUILDING_STATE(mil)];
		flag_t *flag = game_get_flag(building->flag);
		inventory_t *inv = building->u.inventory;
		const int *r = inv->resources;

		/* Weighted resource sum, saturating at 0xffff. */
		unsigned int sum = r[RESOURCE_GOLDBAR];
		sum = sat_add16(sum, sum);
		sum = sat_add16(sum, r[RESOURCE_SWORD]);
		sum = sat_add16(sum, r[RESOURCE_SHIELD]);
		sum = sat_add16(sum, r[RESOURCE_GOLDORE]);
		sum = sat_add16(sum, sum);
		for (int res = RESOURCE_SHOVEL; res <= RESOURCE_PINCER; res++) {
			sum = sat_add16(sum, r[res]);
		}
		sum = sat_add16(sum, sum);
		sum = sat_add16(sum, r[RESOURCE_IRONORE]);
		sum = sat_add16(sum, r[RESOURCE_STEEL]);
		sum = sat_add16(sum, r[RESOURCE_COAL]);
		sum = sat_add16(sum, sum);
		sum = sat_add16(sum, r[RESOURCE_BOAT]);
		sum = sat_add16(sum, r[RESOURCE_STONE]);
		sum = sat_add16(sum, sum);
		sum = sat_add16(sum, sum);

		/* Resource mode (res_dir bits 0-1; the original sets the
		   bits directly, without the panel's delivery cancelling). */
		int mode; /* 0 in, 1 stop, 3 out */
		if (sum >= thr[0]) {
			budget -= 100;
			if (ai_find_other_resource_inventory(flag) < 0) {
				mode = 0;
			} else if (inv->res_dir & BIT(1)) {
				mode = 3;
			} else if (player->ai.u_1b2 >= 10000) {
				/* Original bug: the original reads and writes
				   (0x1b2,A4) with A4 = inventory pointer here.
				   The player's counter ai.u_1b2 (decremented
				   each tick at 0xa554) is meant. */
				mode = 1;
			} else {
				player->ai.u_1b2 += 9000;
				mode = 3;
			}
		} else if (sum >= thr[1]) {
			mode = 1;
		} else {
			mode = 0;
		}

		inv->res_dir = (inv->res_dir & ~3) | mode;
		if (mode == 0) flag->bld2_flags |= BIT(7);
		else flag->bld2_flags &= ~BIT(7);

		/* Serf mode (res_dir bits 2-3) from the generic serfs. */
		unsigned int serfs = inv->generic_count & 0xffff;
		if (serfs >= thr[2]) {
			budget -= 100;
			/* Original bug: find_nearest_inventory takes a flag
			   index in D0, which here still holds the building
			   index of ai_manage_buildings. The stock's flag is
			   meant (A1 is set up for it). */
			if (ai_find_other_serf_inventory(flag) < 0) {
				mode = 0;
			} else if (inv->res_dir & BIT(3)) {
				mode = 3;
			} else if (player->ai.u_1b2 >= 10000) {
				/* Original bug: A4 = inventory, as above. */
				mode = 1;
			} else {
				player->ai.u_1b2 += 3000;
				mode = 3;
			}
		} else if (serfs >= thr[3]) {
			mode = 1;
		} else {
			mode = 0;
		}

		inv->res_dir = (inv->res_dir & ~0xc) | (mode << 2);
		if (mode == 0) flag->bld_flags |= BIT(7);
		else flag->bld_flags &= ~BIT(7);

		return budget - 10;
	}

	return budget - 10;
}

/* 0x299b0: demolish farms when there is enough food. */
static int
ai_manage_farm(player_t *player, building_t *building, int budget)
{
	/* ai.u_39c.. = resource totals by type (food: 0x39c..0x3a6). */
	unsigned int food = player->ai.u_39c & 0xffff;
	food = sat_add16(food, player->ai.u_39e);
	food = sat_add16(food, player->ai.u_3a0);
	food = sat_add16(food, player->ai.u_3a2);
	food = sat_add16(food, player->ai.u_3a4);
	food = sat_add16(food, player->ai.u_3a6);

	if (food < 500) return budget;

	unsigned int max_farms;
	if (food < 600) max_farms = 8;
	else if (food < 700) max_farms = 7;
	else if (food < 800) max_farms = 6;
	else if (food < 900) max_farms = 5;
	else if (food < 1000) max_farms = 4;
	else if (food < 1500) max_farms = 3;
	else if (food < 2000) max_farms = 2;
	else max_farms = 1;

	unsigned int farms = player->completed_building_count[BUILDING_FARM] & 0xffff;
	if (max_farms < farms) ai_demolish_building(building->pos);

	return budget;
}

/* 0x294ae: walk up to 500 finished buildings of the player starting at
   the cursor ai.u_19a and call the handler of their type (jump table
   0x2952a: types without a handler point to an rts). */
void
ai_manage_buildings(player_t *player)
{
	int budget = 500;
	uint index = player->ai.u_19a & 0xffff;

	if (index > game.max_building_index) {
		/* The original checks the (unallocated) entry, then wraps. */
		player->ai.u_19a = 0;
		return;
	}

	while (1) {
		if (index != 0 && BUILDING_ALLOCATED(index)) {
			building_t *building = game_get_building(index);
			if (!BUILDING_IS_BURNING(building) &&
			    BUILDING_IS_DONE(building) &&
			    BUILDING_PLAYER(building) == (int)player->player_num) {
				switch (BUILDING_TYPE(building)) {
				case BUILDING_FISHER:
					budget = ai_manage_fisher(player, building, budget);
					break;
				case BUILDING_LUMBERJACK:
					budget = ai_manage_lumberjack(player, building, budget);
					break;
				case BUILDING_STONECUTTER:
					budget = ai_manage_stonecutter(player, building, budget);
					break;
				case BUILDING_STONEMINE:
					budget = ai_manage_mine(player, building, budget,
								GROUND_DEPOSIT_STONE);
					break;
				case BUILDING_COALMINE:
					budget = ai_manage_mine(player, building, budget,
								GROUND_DEPOSIT_COAL);
					break;
				case BUILDING_IRONMINE:
					budget = ai_manage_mine(player, building, budget,
								GROUND_DEPOSIT_IRON);
					break;
				case BUILDING_GOLDMINE:
					budget = ai_manage_mine(player, building, budget,
								GROUND_DEPOSIT_GOLD);
					break;
				case BUILDING_FORESTER:
					budget = ai_manage_forester(player, building, budget);
					break;
				case BUILDING_STOCK:
					budget = ai_manage_stock(player, building, budget);
					break;
				case BUILDING_FARM:
					budget = ai_manage_farm(player, building, budget);
					break;
				default:
					break;
				}
			}
		}

		index += 1;
		budget -= 1;
		if (budget < 0) break;
		if (index > game.max_building_index) {
			index = 0;
			break;
		}
	}

	player->ai.u_19a = index;
}


/* Location lists of ai.locations, addressed as the original does: n
   consecutive entries starting at category first. */
static ai_location_t *
location_entry(player_t *player, int first, int k)
{
	int n = first * AI_LOCATIONS_PER_CATEGORY + k;
	return &player->ai.locations[n / AI_LOCATIONS_PER_CATEGORY]
		[n % AI_LOCATIONS_PER_CATEGORY];
}

/* 0x29ac8: small building sites must still be owned and free. */
static void
ai_validate_locations_small(player_t *player, int first, int count)
{
	for (int k = 0; k < count; k++) {
		ai_location_t *loc = location_entry(player, first, k);
		if (loc->value == 0) continue;

		map_pos_t pos = MAP_POS(loc->col, loc->row);
		int ok = 0;
		if (AI_OWNED_BY(pos, player->player_num) && AI_SPACE(pos) < 2) {
			pos = MAP_MOVE_RIGHT(pos);
			if (AI_SPACE(pos) < 3) {
				pos = MAP_MOVE_DOWN(pos);
				if (AI_SPACE(pos) < 3) {
					pos = MAP_MOVE_LEFT(pos);
					if (AI_SPACE(pos) < 3) {
						pos = MAP_MOVE_UP_LEFT(pos);
						if (AI_SPACE(pos) < 4) {
							pos = MAP_MOVE_UP(pos);
							if (AI_SPACE(pos) < 4) {
								pos = MAP_MOVE_RIGHT(pos);
								if (AI_SPACE(pos) < 4) ok = 1;
							}
						}
					}
				}
			}
		}

		if (!ok) loc->value = 0;
	}
}

/* 0x29b92: large building sites. */
static void
ai_validate_locations_large(player_t *player, int first, int count)
{
	/* Steps from the site and the space limit after each step. */
	static const struct { int dir; unsigned int limit; } steps[] = {
		{ DIR_RIGHT, 2 }, { DIR_DOWN, 2 }, { DIR_LEFT, 2 },
		{ DIR_UP_LEFT, 2 }, { DIR_UP, 2 }, { DIR_RIGHT, 2 },
		{ DIR_RIGHT, 4 }, { DIR_DOWN_RIGHT, 4 }, { DIR_DOWN, 4 },
		{ DIR_DOWN, 4 }, { DIR_LEFT, 4 }, { DIR_LEFT, 4 },
		{ DIR_UP_LEFT, 4 }, { DIR_UP_LEFT, 4 }, { DIR_UP, 4 },
		{ DIR_UP, 4 }, { DIR_RIGHT, 4 }, { DIR_RIGHT, 4 }
	};

	for (int k = 0; k < count; k++) {
		ai_location_t *loc = location_entry(player, first, k);
		if (loc->value == 0) continue;

		map_pos_t pos = MAP_POS(loc->col, loc->row);
		int ok = AI_OWNED_BY(pos, player->player_num) && AI_SPACE(pos) < 2;
		for (uint s = 0; ok && s < sizeof(steps)/sizeof(steps[0]); s++) {
			pos = MAP_MOVE(pos, steps[s].dir);
			if (AI_SPACE(pos) >= steps[s].limit) ok = 0;
		}

		if (!ok) loc->value = 0;
	}
}

/* 0x29d80: attack targets must still be enemy buildings. */
static void
ai_validate_locations_end(player_t *player, int first, int count)
{
	for (int k = 0; k < count; k++) {
		ai_location_t *loc = location_entry(player, first, k);
		if (loc->value == 0) continue;

		map_pos_t pos = MAP_POS(loc->col, loc->row);
		if (!MAP_HAS_OWNER(pos) ||
		    MAP_OWNER(pos) == player->player_num ||
		    AI_SPACE(pos) < 4) {
			loc->value = 0;
		}
	}
}

/* 0x29a70: drop stored locations that are no longer usable. */
void
ai_validate_locations(player_t *player)
{
	ai_validate_locations_small(player, 1, 72);	/* 0x464: 1..9 */
	ai_validate_locations_small(player, 11, 8);	/* 0x644 */
	ai_validate_locations_small(player, 15, 8);	/* 0x704 */
	ai_validate_locations_large(player, 10, 8);	/* 0x614 */
	ai_validate_locations_large(player, 12, 24);	/* 0x674: 12..14 */
	ai_validate_locations_large(player, 16, 64);	/* 0x734: 16..23 */
	ai_validate_locations_end(player, 26, 72);	/* 0x914: 26..34 */
}


/* Military strength ratio of the player against all others, the word
   the original keeps at player ptr+0x186 (computed by
   player_update_knight_morale @0xb4bc..0xb542). update_knight_morale
   stores the result in player->ai.military_ratio. */
int
ai_calc_military_ratio(player_t *player)
{
	uint32_t own = player->total_military_score;
	unsigned int morale = (player->knight_morale & 0xffff) >> 5;
	while (own >= 0x10000) {
		own >>= 1;
		morale = (morale << 1) & 0xffff;
	}
	own = (own * morale) >> 7;

	uint32_t other = 0;
	for (int i = 0; i < GAME_MAX_PLAYER_COUNT; i++) {
		player_t *p = game.player[i];
		if (p == NULL || p == player) continue;
		other += p->total_military_score;
	}

	while (own >= 0x10000) {
		own >>= 1;
		other >>= 1;
	}
	while (other >= 0x10000) {
		own >>= 1;
		other >>= 1;
	}

	own = (own & 0xffff) >> 1;
	if (own == 0 || other == 0) return 0;
	if (((other - own) & 0x8000) != 0) return 0xffff;

	/* Original bug: divu overflows when other == own and stores 0. */
	uint32_t q = (own << 16) / other;
	return q > 0xffff ? 0xffff : q;
}

/* Attack chance factor by ai.u_1aa, table @0x29dc0. */
static const unsigned int attack_factor[17] = {
	500, 700, 1000, 1400, 1900, 2500, 3000, 3500, 4096,
	5000, 7000, 10000, 15000, 21000, 28000, 36000, 45000
};

/* 0x29de2: decide whether to attack; otherwise scan for sites. */
void
ai_attack(player_t *player)
{
	unsigned int ratio = player->ai.military_ratio & 0xffff;

	if (ratio < 0x8000) {
		unsigned int d0;
		if (ratio >= 0x2000) d0 = (ratio - 0x2000)*2 + 0x1000;
		else d0 = ratio >> 1;

		/* Original reads past the table for u_1aa > 16. */
		int fi = player->ai.u_1aa & 0xffff;
		if (fi > 16) fi = 16;
		uint32_t v = (d0 * attack_factor[fi]) >> 12;
		d0 = v >= 0x10000 ? 0xffff : v;

		/* Knights weighted by level (the first add wraps). */
		unsigned int k = (player->serf_count[SERF_KNIGHT_4] +
				  player->serf_count[SERF_KNIGHT_3]) & 0xffff;
		k = sat_add16(k, k);
		if (k != 0xffff) k = sat_add16(k, player->serf_count[SERF_KNIGHT_2]);
		if (k != 0xffff) k = sat_add16(k, k);
		if (k != 0xffff) k = sat_add16(k, k);
		if (k != 0xffff) k = sat_add16(k, player->serf_count[SERF_KNIGHT_1]);
		if (k < 0x100) d0 = ((d0 * k) >> 8) & 0xffff;

		unsigned int intel = ((0xffff - (player->ai_intelligence & 0xffff)) >> 1) + 0x8000;
		uint64_t p = (uint64_t)d0 * intel * 2;
		uint32_t hi = p > 0xffffffffu ? 0xffff : (uint32_t)(p >> 16);
		uint32_t chance = ((hi * (player->ai_value_2 & 0xffff)) >> 16) * 2;
		if (chance > 0xffff) chance = 0xffff;

		if (game_random_int() >= chance) {
			ai_scan_sites(player);
			return;
		}
	}

	/* Best value of each attack category 26..34. */
	unsigned int w[9];
	for (int c = 0; c < 9; c++) {
		unsigned int best = 0;
		for (int i = 0; i < AI_LOCATIONS_PER_CATEGORY; i++) {
			unsigned int val = player->ai.locations[26+c][i].value & 0xffff;
			if (val > best) best = val;
		}
		w[c] = best;
	}

	/* Weight categories 31..34 by the resource totals
	   (ai.u_3ae stone, u_3b0 iron ore, u_3b4 coal, u_3b6 gold ore). */
	unsigned int stone = player->ai.u_3ae & 0xffff;
	unsigned int ironore = player->ai.u_3b0 & 0xffff;
	unsigned int coal = player->ai.u_3b4 & 0xffff;
	unsigned int goldore = player->ai.u_3b6 & 0xffff;
	unsigned int d;
	uint32_t v;

	d = sat_add16(coal >> 1, 100);
	d = d >= goldore ? d - goldore : 0;
	if (d >= 400) d = 400;
	d = ((d + 50) << 6) & 0xffff;
	v = (d * w[5]) >> 12;
	w[5] = v >= 0x10000 ? 0xffff : v;

	d = sat_add16(coal, 100);
	d = d >= ironore ? d - ironore : 0;
	if (d >= 400) d = 400;
	d = (d << 6) & 0xffff;
	v = (d * w[6]) >> 12;
	w[6] = v >= 0x10000 ? 0xffff : v;

	d = sat_add16(ironore, goldore);
	if (d != 0xffff) d = sat_add16(d, 50);
	d = d >= coal ? d - coal : 0;
	if (d >= 400) d = 400;
	d = (d << 5) & 0xffff;
	v = (d * w[7]) >> 12;
	w[7] = v >= 0x10000 ? 0xffff : v;

	d = stone;
	if (d >= 300) d = 300;
	d = (d << 5) & 0xffff;
	v = (d * w[8]) >> 12;
	w[8] = v >= 0x10000 ? 0xffff : v;

	/* Preferred categories (bits of ai_value_3) count four times. */
	for (int c = 0; c < 9; c++) {
		if (player->ai_value_3 & BIT(c)) {
			unsigned int x = sat_add16(w[c], w[c]);
			if (x != 0xffff) x = sat_add16(x, x);
			w[c] = x;
		}
	}

	unsigned int r = game_random_int();
	int cat;
	if (r & 1) {
		/* Best category. */
		unsigned int best = 0;
		cat = 0;
		for (int c = 0; c < 9; c++) {
			if (w[c] > best) {
				best = w[c];
				cat = c;
			}
		}
		if (best == 0) {
			ai_scan_sites(player);
			return;
		}
	} else {
		/* Random category weighted by w; halve all while the sum
		   overflows 16 bits. */
		unsigned int sum;
		while (1) {
			sum = 0;
			int overflow = 0;
			for (int c = 0; c < 9; c++) {
				sum += w[c];
				if (sum > 0xffff) {
					overflow = 1;
					break;
				}
			}
			if (!overflow) break;
			for (int c = 0; c < 9; c++) w[c] >>= 1;
		}
		if (sum == 0) {
			ai_scan_sites(player);
			return;
		}

		unsigned int pick = (sum * (r & 0xffff)) >> 16;
		unsigned int acc = 0;
		cat = 0;
		for (int c = 0; c < 9; c++) {
			acc += w[c];
			if (pick < acc) break;
			cat += 1;
		}
	}
	ai_game.build_building_type = cat + 26;

	/* Original: endless when the rating keeps refreshing a slot; the
	   port bounds the number of retries. */
	for (int tries = 0; tries < 1000; tries++) {
		ai_location_t *list = player->ai.locations[ai_game.build_building_type];
		ai_location_t *best_loc = NULL;
		unsigned int best = 0;
		for (int i = 0; i < AI_LOCATIONS_PER_CATEGORY; i++) {
			if ((unsigned int)(list[i].value & 0xffff) > best) {
				best = list[i].value & 0xffff;
				best_loc = &list[i];
			}
		}
		if (best == 0) return;

		ai_game.some_location = best_loc;
		best_loc->value = 0;
		player->ai.cursor_col = best_loc->col;
		player->ai.cursor_row = best_loc->row;

		map_pos_t pos = AI_CURSOR_POS(player);
		if (!MAP_HAS_OWNER(pos) ||
		    MAP_OWNER(pos) == player->player_num) continue;
		int obj = MAP_OBJ(pos);
		if (obj < MAP_OBJ_SMALL_BUILDING || obj > MAP_OBJ_CASTLE) continue;

		player->building_attacked = MAP_OBJ_INDEX(pos);
		building_t *target = game_get_building(player->building_attacked);
		/* Original bug: a non-military building branches into the
		   middle of the maximum search (0x2a032) with stale
		   registers; the port rejects it like the other checks. */
		if (!is_military_type(BUILDING_TYPE(target))) continue;
		if (!BUILDING_IS_ACTIVE(target) || BUILDING_STATE(target) != 3) {
			continue;
		}

		/* Own land must be near the target. */
		int near = 0;
		for (int i = 7; i < 7 + 0x102; i++) {
			map_pos_t p = MAP_POS_ADD(target->pos,
						  game.spiral_pos_pattern[i]);
			if (AI_OWNED_BY(p, player->player_num)) {
				near = 1;
				break;
			}
		}
		if (!near) continue;

		player->ai.panel_btn_type = AI_CAN_BUILD_NONE;
		ai_scan_points_of_interest(player);
		unsigned int rating = ai_rate_site_dispatch(player) & 0xffff;

		if (rating < (unsigned int)(ai_game.some_location->value & 0xffff)) {
			int lower = 0;
			for (int i = 0; i < AI_LOCATIONS_PER_CATEGORY; i++) {
				if (rating < (unsigned int)(list[i].value & 0xffff)) {
					lower = 1;
					break;
				}
			}
			if (lower) {
				ai_game.some_location->value = rating;
				continue;
			}
		}

		/* Attack (0x2a15e). */
		player_knights_available_for_attack(player, AI_CURSOR_POS(player));

		/* Defenders of the target, weighted 1 << knight level. */
		target = game_get_building(player->building_attacked);
		unsigned int def = 0;
		int si = target->serf_index & 0xffff;
		while (si != 0) {
			serf_t *serf = game_get_serf(si);
			def += 1 << ((SERF_TYPE(serf) - SERF_KNIGHT_0) & 0x1f);
			si = serf->s.defending.next_knight & 0xffff;
		}
		def &= 0xffff;
		if (BUILDING_TYPE(target) == BUILDING_CASTLE) def = (def * 2) & 0xffff;

		/* Original bug: divu by knight_morale overflows (or traps
		   on 0); the port saturates. */
		unsigned int morale = player->knight_morale & 0xffff;
		uint32_t q = morale ? ((uint32_t)def << 14) / morale : 0xffff;
		if (q > 0xffff) q = 0xffff;
		unsigned int need = (q * (player->ai_value_1 & 0xffff)) >> 16;
		if (need == 0) need = 1;

		uint32_t score = (uint32_t)player->total_military_score << 4;
		unsigned int knights = 0;
		for (int i = 0; i < 5; i++) {
			knights += player->serf_count[SERF_KNIGHT_0 + i];
		}
		knights &= 0xffff;
		if (knights == 0) return;

		/* Original bug: these divu can overflow (and trap when the
		   strength per knight is 0); the need is sign extended. The
		   port uses unsigned values, saturates and does not attack
		   on a zero divisor. */
		uint32_t per_knight = score / knights;
		if (per_knight > 0xffff) per_knight = 0xffff;
		if (per_knight == 0) return;
		uint32_t n = ((uint32_t)need << 4) / per_knight;
		if (n > 0xffff) n = 0xffff;
		n += 1;

		if ((unsigned int)(player->total_attacking_knights & 0xffff) < n) return;
		player->knights_attacking = n;
		if (player->attacking_building_count == 0) return;
		player_start_attack(player);
		return;
	}
}
