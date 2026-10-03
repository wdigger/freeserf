/*
 * ai_want.c - Computer player: how much each building type is wanted
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

/* Port of the Amiga build-choice functions 0x2b8c4..0x2ccc6. Each
   ai_want_<type>() writes player->ai.build_want[type - 1] (and some
   write a preliminary value for a related type that a later call
   finishes). All arithmetic is unsigned 16-bit as in the original:
   W() wraps, sadd() saturates (the original's add + bcs -> 0xffff),
   mulhi() is mulu.w + swap. The inputs come from
   ai_update_building_stats (0x2ccc8):
     ptr+0x33c..0x364  (poi[4] words 0..20): stock fill ratios
                       (amount << 16) / capacity, 0xffff when full.
     ptr+0x366..0x39a  (poi[4] words 21..43, u_394..u_39a): number of
                       serfs per serf type idle in inventories.
     ptr+0x39c..0x3ce  (u_39c..u_3ce): resources per resource type in
                       all inventories of the player (saturated). */

#include <stddef.h>

#include "ai_internal.h"
#include "resource.h"

#define W(x)  ((unsigned)(x) & 0xffff)

/* Stock fill ratio slots (ptr+0x33c + 2 * slot). */
#define FILL_BOATBUILDER_PLANK  0
#define FILL_STONEMINE_FOOD     1
#define FILL_COALMINE_FOOD      2
#define FILL_IRONMINE_FOOD      3
#define FILL_GOLDMINE_FOOD      4
#define FILL_TOOLMAKER_PLANK   11

#define DONE(type)        W(player->completed_building_count[type])
#define INCOMPLETE(type)  W(player->incomplete_building_count[type])
#define BOTH(type)        W(DONE(type) + INCOMPLETE(type))
#define FILL(slot)        W(*ai_stat(player, 0x33c + 2*(slot)))
#define IDLE(serf)        W(*ai_stat(player, 0x366 + 2*(serf)))
#define STOCK(res)        W(*ai_stat(player, 0x39c + 2*(res)))
#define WANT(type)        (player->ai.build_want[(type)-1])
#define DAMP(type)        W(player->ai.build_damp[(type)-1])
#define LOC(cat)          (player->ai.locations[cat])

/* Idle generic serfs (ptr+0x390). */
#define IDLE_GENERIC  IDLE(SERF_GENERIC)

/* Word at Amiga pointer-relative offset off (0x33c..0x3ce) of the
   statistics area: poi[4] and u_394..u_3ce. */
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

static unsigned
sadd(unsigned a, unsigned b)
{
	a = W(a) + W(b);
	return a > 0xffff ? 0xffff : a;
}

static unsigned
mulhi(unsigned a, unsigned b)
{
	return (W(a) * W(b)) >> 16;
}

static unsigned
min16(unsigned a, unsigned b)
{
	return W(a) < W(b) ? W(a) : W(b);
}

/* Recurring curve of the original: have (D1) against need (D0).
   have < need: ~((have << 16 >> shift) / need), else falling from
   base with the surplus. */
static unsigned
ai_want_curve(unsigned have, unsigned need, int shift, int step,
	      unsigned base)
{
	if (have < need) return W(~(((have << 16) >> shift) / need));
	have -= need;
	if (have >= 0x10) return 0;
	return W(~(have << step) + base);
}

/* Shared tail of ai_want_fisher and ai_want_pigfarm: split want between
   the type itself (return value, D1) and the related type (*second,
   D2) by the site scales s1, s2. */
static unsigned
ai_want_share(unsigned want, unsigned s1, unsigned s2, int *second)
{
	if (s1 == 0) {
		*second = (s2 != 0) ? want : 0;
		return 0;
	}
	if (s2 == 0) {
		/* The original leaves the site scale in D1 here, so the
		   type gets the scale as want instead of want. */
		*second = 0;
		return s1;
	}
	if (s1 == s2) {
		*second = want;
		return want;
	}
	if (s2 > s1) {
		*second = want;
		return mulhi((s1 << 16) / s2, want);
	}
	*second = mulhi((s2 << 16) / s1, want);
	return want;
}

/* Stone needed by the incomplete buildings (0x2bc8c/0x2bdae). */
static unsigned
ai_stone_needed(player_t *player)
{
	unsigned d1 = sadd(INCOMPLETE(BUILDING_FORTRESS),
			   INCOMPLETE(BUILDING_FORTRESS));
	d1 = sadd(d1, INCOMPLETE(BUILDING_STOCK));
	d1 = sadd(d1, INCOMPLETE(BUILDING_FARM));
	d1 = sadd(d1, INCOMPLETE(BUILDING_BUTCHER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_PIGFARM));
	d1 = sadd(d1, INCOMPLETE(BUILDING_MILL));
	d1 = sadd(d1, INCOMPLETE(BUILDING_BAKER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_SAWMILL));
	d1 = sadd(d1, INCOMPLETE(BUILDING_STEELSMELTER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_TOOLMAKER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_WEAPONSMITH));
	d1 = sadd(d1, INCOMPLETE(BUILDING_TOWER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_GOLDSMELTER));
	d1 = sadd(d1, d1);
	d1 = sadd(d1, INCOMPLETE(BUILDING_STONEMINE));
	d1 = sadd(d1, INCOMPLETE(BUILDING_HUT));
	d1 = sadd(d1, 8);
	return d1;
}

/* Miners available for mine type with the stonecutter correction
   (common head of the four mine functions, 0x2bd4e). */
static unsigned
ai_miners_available(player_t *player)
{
	unsigned d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_PICK));
	unsigned d1 = INCOMPLETE(BUILDING_STONECUTTER);
	if (d1 >= IDLE(SERF_STONECUTTER)) {
		d1 -= IDLE(SERF_STONECUTTER);
		d0 = (d0 >= d1) ? d0 - d1 : 0;
	}
	return W(d0 + IDLE(SERF_MINER));
}

/* Incomplete mines of all four kinds. */
static unsigned
ai_incomplete_mines(player_t *player)
{
	return W(INCOMPLETE(BUILDING_STONEMINE) +
		 INCOMPLETE(BUILDING_COALMINE) +
		 INCOMPLETE(BUILDING_IRONMINE) +
		 INCOMPLETE(BUILDING_GOLDMINE));
}

/* Knights available for military buildings (0x2c154). */
static unsigned
ai_knights_available(player_t *player)
{
	unsigned d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_SWORD));
	d0 = min16(d0, STOCK(RESOURCE_SHIELD));
	d0 = W(d0 + IDLE(SERF_KNIGHT_0) + IDLE(SERF_KNIGHT_1) +
	       IDLE(SERF_KNIGHT_2) + IDLE(SERF_KNIGHT_3) +
	       IDLE(SERF_KNIGHT_4));
	return d0;
}

static unsigned
ai_incomplete_military(player_t *player)
{
	return W(INCOMPLETE(BUILDING_HUT) + INCOMPLETE(BUILDING_TOWER) +
		 INCOMPLETE(BUILDING_FORTRESS));
}

/* ai_want_scale @0x2cc16: mean value of the 8 stored sites of a
   category. */
static unsigned
ai_want_scale(ai_location_t *loc)
{
	uint32_t sum = 0;
	int i;
	for (i = 0; i < 8; i++) sum += W(loc[i].value);
	return W(sum >> 3);
}

/* ai_want_scale_mine @0x2cc5c: mean of the two best stored sites of a
   category. */
static unsigned
ai_want_scale_mine(ai_location_t *loc)
{
	unsigned d6 = 0, d7 = 0;
	int i;
	/* Original bug: the unrolled loop reads (A3) eight times without
	   advancing, so the result is just the value of the first site.
	   Each of the 8 sites is used here. */
	for (i = 0; i < 8; i++) {
		unsigned v = W(loc[i].value);
		if (v > d6) {
			d6 = v;
			if (d6 > d7) {
				unsigned t = d6;
				d6 = d7;
				d7 = t;
			}
		}
	}
	return W((d6 >> 1) + (d7 >> 1));
}

/* 0x2b8c4: type 25 (pseudo building type). */
void
ai_want_type25(player_t *player)
{
	if (W(IDLE(SERF_TRANSPORTER) + IDLE_GENERIC) < 3) return;
	if (ai_want_scale(LOC(0)) == 0) return;
	WANT(25) = mulhi(0xdac0, DAMP(25));
}

/* 0x2b8f0: fisher; also the preliminary want of farm (finished by
   ai_want_farm). */
void
ai_want_fisher(player_t *player)
{
	unsigned d0, d1, d2, d3, s1, s2;
	int farm;

	/* Food demand of the mines weighted by their food stock fill. */
	d1 = BOTH(BUILDING_STONEMINE);
	d3 = (d1 < 0x2000 ? d1 : 0x1fff) << 3;
	d0 = mulhi(d3, FILL(FILL_STONEMINE_FOOD));

	d2 = BOTH(BUILDING_COALMINE);
	d1 = W(d1 + d2);
	d3 = (d2 < 0x2000 ? d2 : 0x1fff) << 3;
	d0 = W(d0 + mulhi(d3, FILL(FILL_COALMINE_FOOD)));

	d2 = BOTH(BUILDING_IRONMINE);
	d1 = W(d1 + d2);
	d3 = (d2 < 0x2000 ? d2 : 0x1fff) << 3;
	d0 = W(d0 + mulhi(d3, FILL(FILL_IRONMINE_FOOD)));

	d2 = BOTH(BUILDING_GOLDMINE);
	d1 = W(d1 + d2);
	d3 = (d2 < 0x2000 ? d2 : 0x1fff) << 3;
	d0 = sadd(d0, mulhi(d3, FILL(FILL_GOLDMINE_FOOD)));

	/* Food in stock. */
	d0 = sadd(d0, STOCK(RESOURCE_FISH));
	d0 = sadd(d0, STOCK(RESOURCE_MEAT));
	d0 = sadd(d0, STOCK(RESOURCE_BREAD));
	if (DONE(BUILDING_BUTCHER) != 0) d0 = sadd(d0, STOCK(RESOURCE_PIG));
	if (DONE(BUILDING_BAKER) != 0) {
		d0 = sadd(d0, STOCK(RESOURCE_FLOUR));
		if (DONE(BUILDING_MILL) != 0) {
			d0 = sadd(d0, STOCK(RESOURCE_WHEAT));
		}
	}

	if (d1 < 8) {
		d0 = W(d0 + d0);
		if (d1 < 4) {
			d0 = W(d0 + d0);
			if (d1 < 2) d0 = W(d0 + d0);
		}
	}

	if (d1 >= 0x2000) d1 = 0x1fff;
	d1 <<= 3;
	d0 = (d0 >= d1) ? d0 - d1 : 0;

	if (d0 < 0x20) {
		d0 = W(~(d0 << 10));
	} else if (d0 < 0x50) {
		d0 = W(~(d0 << 9) - 0x4000);
	} else if (d0 < 0x70) {
		d0 = W(~(d0 << 8) + 0x7001);
	} else {
		d0 = 0;
	}

	s1 = ai_want_scale(LOC(1));
	s2 = ai_want_scale(LOC(12));
	d1 = ai_want_share(d0, s1, s2, &farm);
	WANT(BUILDING_FARM) = farm;

	d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_ROD));
	d0 = W(d0 + IDLE(SERF_FISHER));
	if (INCOMPLETE(BUILDING_FISHER) >= d0) return;

	WANT(BUILDING_FISHER) = mulhi(d1, DAMP(BUILDING_FISHER));
}

/* 0x2ba88: lumberjack. */
void
ai_want_lumberjack(player_t *player)
{
	unsigned d0, d1, d2;

	d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_AXE));
	d0 = W(d0 + IDLE(SERF_LUMBERJACK));
	if (INCOMPLETE(BUILDING_LUMBERJACK) >= d0) return;

	if (BOTH(BUILDING_LUMBERJACK) == 0) {
		WANT(BUILDING_LUMBERJACK) = 0xffff;
		return;
	}

	/* Planks and lumber available. */
	d0 = sadd(STOCK(RESOURCE_LUMBER), STOCK(RESOURCE_PLANK));
	d2 = DONE(BUILDING_BOATBUILDER);
	d2 = (d2 < 0x2000 ? d2 : 0x1fff) << 3;
	d0 = sadd(d0, mulhi(d2, FILL(FILL_BOATBUILDER_PLANK)));
	d2 = DONE(BUILDING_TOOLMAKER);
	d2 = (d2 < 0x2000 ? d2 : 0x1fff) << 3;
	d0 = sadd(d0, mulhi(d2, FILL(FILL_TOOLMAKER_PLANK)));

	/* Planks needed. */
	d1 = sadd(DONE(BUILDING_TOOLMAKER), INCOMPLETE(BUILDING_FORTRESS));
	d1 = sadd(d1, d1);
	d1 = sadd(d1, DONE(BUILDING_BOATBUILDER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_STONEMINE));
	d1 = sadd(d1, INCOMPLETE(BUILDING_COALMINE));
	d1 = sadd(d1, INCOMPLETE(BUILDING_IRONMINE));
	d1 = sadd(d1, INCOMPLETE(BUILDING_GOLDMINE));
	d1 = sadd(d1, INCOMPLETE(BUILDING_STOCK));
	d1 = sadd(d1, INCOMPLETE(BUILDING_FARM));
	d1 = sadd(d1, INCOMPLETE(BUILDING_BUTCHER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_PIGFARM));
	d1 = sadd(d1, INCOMPLETE(BUILDING_MILL));
	d1 = sadd(d1, INCOMPLETE(BUILDING_BAKER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_SAWMILL));
	d1 = sadd(d1, INCOMPLETE(BUILDING_STEELSMELTER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_TOOLMAKER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_WEAPONSMITH));
	d1 = sadd(d1, INCOMPLETE(BUILDING_TOWER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_GOLDSMELTER));
	d1 = sadd(d1, d1);
	d1 = sadd(d1, INCOMPLETE(BUILDING_FISHER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_LUMBERJACK));
	d1 = sadd(d1, INCOMPLETE(BUILDING_BOATBUILDER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_STONECUTTER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_FORESTER));
	d1 = sadd(d1, INCOMPLETE(BUILDING_HUT));

	d0 = (d0 >= d1) ? d0 - d1 : 0;

	if (d0 < 0x40) {
		d0 = W(~(d0 << 9));
	} else if (d0 < 0x70) {
		/* Original bug: subtracts 0x4000 here as well (copied from
		   ai_want_fisher), which wraps for 0x60..0x6f to nearly
		   0xffff. Without it the curve joins both neighbours. */
		d0 = W(~(d0 << 9));
	} else if (d0 < 0x90) {
		d0 = W(~(d0 << 8) - 0x6fff);
	} else {
		d0 = 0;
	}

	WANT(BUILDING_LUMBERJACK) = mulhi(d0, DAMP(BUILDING_LUMBERJACK));
}

/* 0x2bbe0: boatbuilder; also sets the planks priority of
   boatbuilders. */
void
ai_want_boatbuilder(player_t *player)
{
	unsigned d0;

	d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_HAMMER));
	d0 = W(d0 + IDLE(SERF_BOATBUILDER));
	if (INCOMPLETE(BUILDING_BOATBUILDER) >= d0) return;

	d0 = STOCK(RESOURCE_BOAT);
	if (d0 == 0) {
		d0 = 0x88b8;
	} else if (d0 < 2) {
		d0 = 0x4e20;
	} else if (d0 < 8) {
		d0 = W(~(d0 << 10) + 0x2800);
	} else {
		d0 = 0;
	}

	player->planks_boatbuilder = d0;
	WANT(BUILDING_BOATBUILDER) = mulhi(d0, DAMP(BUILDING_BOATBUILDER));
}

/* 0x2bc38: stonecutter. */
void
ai_want_stonecutter(player_t *player)
{
	unsigned d0, d1;

	/* Picks are kept for missing coal and iron mines and for the
	   miners needed by incomplete mines. */
	d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_PICK));
	if (DONE(BUILDING_COALMINE) == 0 && d0 > 0) d0 -= 1;
	if (DONE(BUILDING_IRONMINE) == 0 && d0 > 0) d0 -= 1;
	d1 = ai_incomplete_mines(player);
	if (d1 >= IDLE(SERF_MINER)) {
		d1 -= IDLE(SERF_MINER);
		d0 = (d0 >= d1) ? d0 - d1 : 0;
	}
	d0 = W(d0 + IDLE(SERF_STONECUTTER));
	if (INCOMPLETE(BUILDING_STONECUTTER) >= d0) return;

	d0 = STOCK(RESOURCE_STONE);
	d1 = ai_stone_needed(player);
	d0 = (d0 >= d1) ? d0 - d1 : 0;

	if (d0 < 0x10) {
		d0 = W(~(d0 << 11));
	} else if (d0 < 0x40) {
		d0 = W(~(d0 << 9) - 0x6000);
	} else if (d0 < 0x140) {
		d0 = W(~(d0 << 5) + 0x2801);
	} else {
		d0 = 0;
	}

	d1 = ai_want_scale(LOC(4));
	if (d1 >= 0x400) d1 = 0x3ff;
	d0 = mulhi(d0, d1 << 6);
	WANT(BUILDING_STONECUTTER) = mulhi(d0, DAMP(BUILDING_STONECUTTER));
}

/* 0x2bd4e: stonemine. */
void
ai_want_stonemine(player_t *player)
{
	unsigned d0, d1, d2;

	d0 = ai_miners_available(player);
	if (DONE(BUILDING_COALMINE) == 0) {
		if (d0 == 0) return;
		d0 -= 1;
	}
	if (DONE(BUILDING_IRONMINE) == 0) {
		if (d0 == 0) return;
		d0 -= 1;
	}
	if (d0 <= ai_incomplete_mines(player)) return;

	d0 = ai_want_scale_mine(LOC(5));
	d2 = STOCK(RESOURCE_STONE);
	d1 = ai_stone_needed(player);
	if (d2 < d1) {
		if (d0 >= 0xff0) d0 = 0xfef;
		d0 <<= 4;
	} else {
		if (d0 >= 0x1f40) d0 = 0x1f3f;
		d1 = W(STOCK(RESOURCE_STONE) << 4);
		d0 = (d0 >= d1) ? d0 - d1 : 0;
		d0 = W(d0 << 3);
	}

	WANT(BUILDING_STONEMINE) = mulhi(d0, DAMP(BUILDING_STONEMINE));
}

/* 0x2be4a: coalmine. */
void
ai_want_coalmine(player_t *player)
{
	unsigned d0, d1;

	d0 = ai_miners_available(player);
	if (DONE(BUILDING_IRONMINE) == 0) {
		if (d0 == 0) return;
		d0 -= 1;
	}
	if (ai_incomplete_mines(player) >= d0) return;

	d0 = ai_want_scale_mine(LOC(6));
	if (d0 >= 0x1f40) d0 = 0x1f3f;
	d1 = W(STOCK(RESOURCE_COAL) << 2);
	d0 = (d0 >= d1) ? d0 - d1 : 0;
	d0 <<= 3;
	if (d0 > 0xffff) d0 = 0xffff;

	WANT(BUILDING_COALMINE) = mulhi(d0, DAMP(BUILDING_COALMINE));
}

/* 0x2bec0: ironmine. */
void
ai_want_ironmine(player_t *player)
{
	unsigned d0, d1;

	d0 = ai_miners_available(player);
	if (DONE(BUILDING_COALMINE) == 0) {
		if (d0 == 0) return;
		d0 -= 1;
	}
	if (ai_incomplete_mines(player) >= d0) return;

	d0 = ai_want_scale_mine(LOC(7));
	if (d0 >= 0x1f40) d0 = 0x1f3f;
	d1 = W(STOCK(RESOURCE_IRONORE) << 3);
	d0 = (d0 >= d1) ? d0 - d1 : 0;
	d0 <<= 3;
	if (d0 > 0xffff) d0 = 0xffff;

	WANT(BUILDING_IRONMINE) = mulhi(d0, DAMP(BUILDING_IRONMINE));
}

/* 0x2bf36: goldmine. */
void
ai_want_goldmine(player_t *player)
{
	unsigned d0, d1;

	d0 = ai_miners_available(player);
	if (DONE(BUILDING_COALMINE) == 0) {
		if (d0 == 0) return;
		d0 -= 1;
	}
	if (DONE(BUILDING_IRONMINE) == 0) {
		if (d0 == 0) return;
		d0 -= 1;
	}
	if (ai_incomplete_mines(player) >= d0) return;

	d0 = ai_want_scale_mine(LOC(8));
	if (d0 >= 0x7f8) d0 = 0x7f7;
	d1 = STOCK(RESOURCE_GOLDORE);
	d0 = (d0 >= d1) ? d0 - d1 : 0;
	d0 <<= 5;
	if (d0 > 0xffff) d0 = 0xffff;

	WANT(BUILDING_GOLDMINE) = mulhi(d0, DAMP(BUILDING_GOLDMINE));
}

/* 0x2bfb6: forester. */
void
ai_want_forester(player_t *player)
{
	unsigned d0, d1;

	d0 = W(IDLE_GENERIC + IDLE(SERF_FORESTER));
	if (INCOMPLETE(BUILDING_FORESTER) >= d0) return;

	d0 = BOTH(BUILDING_LUMBERJACK);
	d1 = BOTH(BUILDING_FORESTER);
	if (d1 < d0) {
		d0 = ai_want_scale(LOC(9));
		if (d0 >= 0x7d0) d0 = 0x7cf;
		d0 <<= 5;
	} else if (d1 == 0) {
		WANT(BUILDING_FORESTER) = 0;
		return;
	} else if (d0 == 0) {
		/* Original bug: ((0 << 16) - 1) / d1 overflows divu, which
		   leaves 0xffffffff and gives a want of 0x7ff * damp. No
		   lumberjacks means no foresters are wanted. */
		d0 = 0;
	} else {
		d0 = ((((uint32_t)d0 << 16) - 1) / d1) >> 5;
	}

	WANT(BUILDING_FORESTER) = mulhi(d0, DAMP(BUILDING_FORESTER));
}

/* 0x2c012: stock. */
void
ai_want_stock(player_t *player)
{
	unsigned d0, d1;

	if (INCOMPLETE(BUILDING_STOCK) >= IDLE_GENERIC) return;

	/* Large buildings count double. */
	d0 = W(BOTH(BUILDING_STONEMINE) + BOTH(BUILDING_COALMINE) +
	       BOTH(BUILDING_IRONMINE) + BOTH(BUILDING_GOLDMINE) +
	       BOTH(BUILDING_STEELSMELTER) + BOTH(BUILDING_TOOLMAKER) +
	       BOTH(BUILDING_WEAPONSMITH) + BOTH(BUILDING_GOLDSMELTER));
	d0 = sadd(d0, d0);
	d0 = sadd(d0, INCOMPLETE(BUILDING_FISHER));
	d0 = sadd(d0, DONE(BUILDING_FISHER));
	d0 = sadd(d0, INCOMPLETE(BUILDING_LUMBERJACK));
	d0 = sadd(d0, DONE(BUILDING_LUMBERJACK));
	d0 = sadd(d0, INCOMPLETE(BUILDING_BOATBUILDER));
	d0 = sadd(d0, DONE(BUILDING_BOATBUILDER));
	d0 = sadd(d0, INCOMPLETE(BUILDING_STONECUTTER));
	d0 = sadd(d0, DONE(BUILDING_STONECUTTER));
	d0 = sadd(d0, INCOMPLETE(BUILDING_FARM));
	d0 = sadd(d0, DONE(BUILDING_FARM));
	d0 = sadd(d0, INCOMPLETE(BUILDING_BUTCHER));
	d0 = sadd(d0, DONE(BUILDING_BUTCHER));
	d0 = sadd(d0, INCOMPLETE(BUILDING_PIGFARM));
	d0 = sadd(d0, DONE(BUILDING_PIGFARM));
	d0 = sadd(d0, INCOMPLETE(BUILDING_MILL));
	d0 = sadd(d0, DONE(BUILDING_MILL));
	d0 = sadd(d0, INCOMPLETE(BUILDING_BAKER));
	d0 = sadd(d0, DONE(BUILDING_BAKER));
	d0 = sadd(d0, INCOMPLETE(BUILDING_SAWMILL));
	d0 = sadd(d0, DONE(BUILDING_SAWMILL));

	if (STOCK(RESOURCE_PLANK) >= 0x50) {
		d0 = (d0 >> 4) + 4;
	} else if (STOCK(RESOURCE_PLANK) >= 0x28) {
		d0 = (d0 >> 4) + 2;
	} else if (d0 < 0x20) {
		d0 = 0;
	} else if (d0 < 0x40) {
		d0 = ((d0 - 0x20) >> 3) + 2;
	} else {
		d0 = (d0 >> 4) + 2;
	}

	d1 = BOTH(BUILDING_STOCK);
	d1 = ai_want_curve(d1, d0, 2, 8, 0x1001);
	if (d1 >= 0xfeb0) d1 = 0xfeaf;

	WANT(BUILDING_STOCK) = mulhi(d1, DAMP(BUILDING_STOCK));
}

/* 0x2c154: hut. */
void
ai_want_hut(player_t *player)
{
	unsigned d0;

	if (ai_incomplete_military(player) >= ai_knights_available(player)) {
		return;
	}

	d0 = ai_want_scale(LOC(11));
	if (d0 >= 0x4000) d0 = 0x3fff;
	d0 <<= 2;
	if (d0 >= W(player->ai_value_5)) d0 = W(player->ai_value_5);

	WANT(BUILDING_HUT) = mulhi(d0, DAMP(BUILDING_HUT));
}

/* 0x2c1c0: farm (want prepared by ai_want_fisher). */
void
ai_want_farm(player_t *player)
{
	unsigned d0;

	d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_SCYTHE));
	d0 = W(d0 + IDLE(SERF_FARMER));
	if (INCOMPLETE(BUILDING_FARM) >= d0) {
		WANT(BUILDING_FARM) = 0;
		return;
	}

	WANT(BUILDING_FARM) = mulhi(WANT(BUILDING_FARM),
				    DAMP(BUILDING_FARM));
}

/* 0x2c1f0: butcher. */
void
ai_want_butcher(player_t *player)
{
	unsigned d0, d1;

	d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_CLEAVER));
	d0 = W(d0 + IDLE(SERF_BUTCHER));
	if (INCOMPLETE(BUILDING_BUTCHER) >= d0) return;

	d0 = BOTH(BUILDING_PIGFARM);
	d1 = BOTH(BUILDING_BUTCHER);
	if (d1 >= 0x4000) d1 = 0x3fff;
	d1 <<= 2;
	d1 = ai_want_curve(d1, d0, 2, 7, 0x801);

	d0 = STOCK(RESOURCE_PIG);
	if (d0 >= 0x200) d0 = 0x1ff;
	d0 = sadd(d0 << 7, d1);

	WANT(BUILDING_BUTCHER) = mulhi(d0, DAMP(BUILDING_BUTCHER));
}

/* 0x2c26e: pigfarm; also the preliminary want of mill (finished by
   ai_want_mill). */
void
ai_want_pigfarm(player_t *player)
{
	unsigned d0, d1, d2, s1, s2;
	int mill;

	d0 = W(IDLE_GENERIC + IDLE(SERF_PIGFARMER));
	if (INCOMPLETE(BUILDING_PIGFARM) >= d0) return;

	/* Wheat production. */
	d0 = BOTH(BUILDING_FARM);
	if (d0 >= 0x4000) d0 = 0x3fff;
	if (d0 == 0) return;
	d0 <<= 2;
	d1 = sadd(STOCK(RESOURCE_WHEAT), 0x20) >> 6;
	d0 = sadd(d0, d1);

	/* Wheat consumption: 12 per mill, 3 per pigfarm. */
	d1 = BOTH(BUILDING_MILL);
	d1 = sadd(d1, d1);
	d1 = sadd(d1, d1);
	d2 = d1;
	d1 = sadd(d1, d1);
	d1 = sadd(d1, d2);
	d2 = BOTH(BUILDING_PIGFARM) * 3;
	d1 = (d2 > 0xffff) ? 0xffff : sadd(d1, d2);

	d1 = ai_want_curve(d1, d0, 2, 7, 0x801);

	d0 = STOCK(RESOURCE_WHEAT);
	if (d0 >= 0x200) d0 = 0x1ff;
	d0 = sadd(d0 << 7, d1);

	s1 = ai_want_scale(LOC(14));
	s2 = ai_want_scale(LOC(15));
	d1 = ai_want_share(d0, s1, s2, &mill);
	WANT(BUILDING_MILL) = mill;

	WANT(BUILDING_PIGFARM) = mulhi(d1, DAMP(BUILDING_PIGFARM));
}

/* 0x2c382: mill (want prepared by ai_want_pigfarm). */
void
ai_want_mill(player_t *player)
{
	unsigned d0;

	d0 = W(IDLE_GENERIC + IDLE(SERF_MILLER));
	if (INCOMPLETE(BUILDING_MILL) >= d0) {
		WANT(BUILDING_MILL) = 0;
		return;
	}

	WANT(BUILDING_MILL) = mulhi(WANT(BUILDING_MILL),
				    DAMP(BUILDING_MILL));
}

/* 0x2c3a8: baker. */
void
ai_want_baker(player_t *player)
{
	unsigned d0, d1;

	d0 = W(IDLE_GENERIC + IDLE(SERF_BAKER));
	if (INCOMPLETE(BUILDING_BAKER) >= d0) return;

	d0 = BOTH(BUILDING_MILL);
	d1 = BOTH(BUILDING_BAKER);
	d1 = ai_want_curve(d1, d0, 2, 7, 0x801);

	d0 = STOCK(RESOURCE_FLOUR);
	if (d0 >= 0x200) d0 = 0x1ff;
	d0 = sadd(d0 << 7, d1);

	WANT(BUILDING_BAKER) = mulhi(d0, DAMP(BUILDING_BAKER));
}

/* 0x2c410: sawmill. */
void
ai_want_sawmill(player_t *player)
{
	unsigned d0, d1;

	d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_SAW));
	d0 = W(d0 + IDLE(SERF_SAWMILLER));
	if (INCOMPLETE(BUILDING_SAWMILL) >= d0) return;

	d0 = BOTH(BUILDING_LUMBERJACK);
	d1 = sadd(STOCK(RESOURCE_LUMBER), 0x20) >> 5;
	d0 = sadd(d0, d1);

	d1 = BOTH(BUILDING_SAWMILL) * 3;
	if (d1 > 0xffff) d1 = 0xffff;
	d1 = ai_want_curve(d1, d0, 2, 7, 0x801);

	d0 = STOCK(RESOURCE_LUMBER);
	if (d0 >= 0x200) d0 = 0x1ff;
	d0 = sadd(d0 << 7, d1);

	WANT(BUILDING_SAWMILL) = mulhi(d0, DAMP(BUILDING_SAWMILL));
}

/* 0x2c4a2: steelsmelter. */
void
ai_want_steelsmelter(player_t *player)
{
	unsigned d0, d1;

	d0 = W(IDLE_GENERIC + IDLE(SERF_SMELTER));
	d1 = W(INCOMPLETE(BUILDING_STEELSMELTER) +
	       INCOMPLETE(BUILDING_GOLDSMELTER));
	if (d1 >= d0) return;

	d0 = min16(BOTH(BUILDING_COALMINE), BOTH(BUILDING_IRONMINE));
	d1 = min16(STOCK(RESOURCE_IRONORE), STOCK(RESOURCE_COAL)) >> 4;
	d0 = W(d0 + d1 + 1) >> 1;

	d1 = BOTH(BUILDING_STEELSMELTER);
	d1 = ai_want_curve(d1, d0, 2, 7, 0x801);

	d0 = STOCK(RESOURCE_IRONORE);
	if (d0 >= 0x400) d0 = 0x3ff;
	d0 = sadd(d0 << 6, d1);

	WANT(BUILDING_STEELSMELTER) = mulhi(d0, DAMP(BUILDING_STEELSMELTER));
}

/* 0x2c532: toolmaker. */
void
ai_want_toolmaker(player_t *player)
{
	unsigned d0, d1, d2;
	int i;

	d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_SAW));
	d0 = min16(d0, STOCK(RESOURCE_HAMMER));
	d0 = W(d0 + IDLE(SERF_TOOLMAKER));
	if (INCOMPLETE(BUILDING_TOOLMAKER) >= d0) return;

	/* Highest tool priority. */
	d2 = W(player->tool_prio[0]);
	for (i = 1; i < 9; i++) {
		if (W(player->tool_prio[i]) > d2) d2 = W(player->tool_prio[i]);
	}

	d0 = d2;
	if (BOTH(BUILDING_TOOLMAKER) != 0) {
		d1 = min16(STOCK(RESOURCE_PLANK), STOCK(RESOURCE_STEEL));
		d2 = min16(BOTH(BUILDING_LUMBERJACK), BOTH(BUILDING_IRONMINE));
		d1 = W(d1 + W(d2 << 4));
		d2 = W(BOTH(BUILDING_TOOLMAKER) << 6);
		d1 = (d1 >= d2) ? d1 - d2 : 0;
		if (d1 >= 0x10) d1 = 0xf;
		d0 = mulhi(d0, d1 << 12);
	}

	WANT(BUILDING_TOOLMAKER) = mulhi(d0, DAMP(BUILDING_TOOLMAKER));
}

/* 0x2c610: weaponsmith. */
void
ai_want_weaponsmith(player_t *player)
{
	unsigned d0, d1, d2;

	d0 = min16(IDLE_GENERIC, STOCK(RESOURCE_PINCER));
	d0 = min16(d0, STOCK(RESOURCE_HAMMER));
	d0 = W(d0 + IDLE(SERF_WEAPONSMITH));
	if (INCOMPLETE(BUILDING_WEAPONSMITH) >= d0) return;

	d0 = min16(STOCK(RESOURCE_SWORD), STOCK(RESOURCE_SHIELD));
	if (d0 < 0x40) {
		d0 = W(~(d0 << 9));
	} else if (d0 < 0x100) {
		d0 = W(~(d0 << 7) - 0x6000);
	} else if (d0 < 0x500) {
		d0 = W(~(d0 << 3) + 0x2801);
	} else {
		d0 = 0;
	}

	d1 = W(min16(STOCK(RESOURCE_STEEL), STOCK(RESOURCE_COAL)) + 7);
	d2 = min16(BOTH(BUILDING_COALMINE), BOTH(BUILDING_IRONMINE));
	d1 = W(d1 + W(d2 << 3));
	d2 = W(BOTH(BUILDING_WEAPONSMITH) << 4);
	d1 = (d1 >= d2) ? d1 - d2 : 0;
	if (d1 >= 0x10) d1 = 0xf;
	d1 = (d1 << 12) + 0xfa0;
	d0 = mulhi(d0, d1);

	WANT(BUILDING_WEAPONSMITH) = mulhi(d0, DAMP(BUILDING_WEAPONSMITH));
}

/* 0x2c6ce: tower; also the preliminary want of fortress (finished by
   ai_want_fortress). */
void
ai_want_tower(player_t *player)
{
	unsigned d0, d1;

	if (W(player->ai.u_1aa) < 8) return;
	if (ai_incomplete_military(player) >= ai_knights_available(player)) {
		return;
	}

	/* Buildings to protect. */
	d0 = W(DONE(BUILDING_STONEMINE) + DONE(BUILDING_COALMINE) +
	       DONE(BUILDING_IRONMINE) + DONE(BUILDING_GOLDMINE) +
	       DONE(BUILDING_FARM) + DONE(BUILDING_BUTCHER) +
	       DONE(BUILDING_PIGFARM) + DONE(BUILDING_BAKER) +
	       DONE(BUILDING_SAWMILL) + DONE(BUILDING_STEELSMELTER) +
	       DONE(BUILDING_TOOLMAKER) + DONE(BUILDING_WEAPONSMITH) +
	       DONE(BUILDING_GOLDSMELTER));
	d1 = DONE(BUILDING_STOCK);
	d1 = (d1 < 0x2000 ? d1 : 0x1fff) << 3;
	d0 = sadd(d0, d1);

	/* Large military buildings. */
	d1 = W(DONE(BUILDING_FORTRESS) + 1);
	d1 = sadd(d1, d1);
	d1 = sadd(d1, DONE(BUILDING_TOWER));
	if (d1 >= 0x800) d1 = 0x7ff;
	d1 <<= 3;
	d1 = ai_want_curve(d1, d0, 1, 7, 0x801);

	WANT(BUILDING_FORTRESS) = d1;
	WANT(BUILDING_TOWER) = mulhi(d1, DAMP(BUILDING_TOWER));
}

/* 0x2c7b2: fortress (want prepared by ai_want_tower). */
void
ai_want_fortress(player_t *player)
{
	if (W(player->ai.u_1aa) < 10) return;
	if (ai_incomplete_military(player) >= ai_knights_available(player)) {
		WANT(BUILDING_FORTRESS) = 0;
		return;
	}

	WANT(BUILDING_FORTRESS) = mulhi(WANT(BUILDING_FORTRESS),
					DAMP(BUILDING_FORTRESS));
}

/* 0x2c80e: goldsmelter. */
void
ai_want_goldsmelter(player_t *player)
{
	unsigned d0, d1;

	d0 = W(IDLE_GENERIC + IDLE(SERF_SMELTER));
	d1 = W(INCOMPLETE(BUILDING_STEELSMELTER) +
	       INCOMPLETE(BUILDING_GOLDSMELTER));
	if (d1 >= d0) return;

	d0 = min16(BOTH(BUILDING_COALMINE), BOTH(BUILDING_GOLDMINE));
	d1 = min16(STOCK(RESOURCE_GOLDORE), STOCK(RESOURCE_COAL)) >> 4;
	d0 = W(d0 + d1 + 1) >> 1;

	d1 = BOTH(BUILDING_GOLDSMELTER);
	d1 = ai_want_curve(d1, d0, 2, 7, 0x801);

	d0 = STOCK(RESOURCE_GOLDORE);
	if (d0 >= 0x400) d0 = 0x3ff;
	d0 = sadd(d0 << 6, d1);

	WANT(BUILDING_GOLDSMELTER) = mulhi(d0, DAMP(BUILDING_GOLDSMELTER));
}

/* 0x2c89e: castle. */
void
ai_want_castle(player_t *player)
{
	unsigned d0, d1;

	if (IDLE(SERF_GEOLOGIST) != 0) {
		if (W(IDLE(SERF_TRANSPORTER) + IDLE_GENERIC) < 2) return;
	} else {
		d0 = W((player->total_land_area >> 7) + 3);
		if (d0 < W(player->serf_count[SERF_GEOLOGIST])) return;
		if (STOCK(RESOURCE_HAMMER) == 0) return;
		if (IDLE_GENERIC == 0) return;
		if (W(IDLE_GENERIC + IDLE(SERF_TRANSPORTER)) < 3) return;
	}

	d1 = ai_want_scale(LOC(25));
	if (d1 >= 0x3a98) d1 = 0x3a97;
	d1 <<= 2;

	WANT(BUILDING_CASTLE) = mulhi(d1, DAMP(BUILDING_CASTLE));
}
