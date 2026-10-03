/*
 * mission.c - Predefined game mission maps
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

#include "mission.h"

mission_t mission[30] = {0};

void
init_missions()
{
	/* Mission 1: START */
	mission[0].rnd.state[0] = 0x6d6f;
	mission[0].rnd.state[1] = 0xf7f0;
	mission[0].rnd.state[2] = 0xc8d4;

	mission[0].player[0].supplies = 35,
	mission[0].player[0].reproduction = 30,
	mission[0].player[0].castle.col = -1;
	mission[0].player[0].castle.row = -1;

	mission[0].player[1].face = 1,
	mission[0].player[1].intelligence = 10,
	mission[0].player[1].supplies = 5,
	mission[0].player[1].reproduction = 30,
	mission[0].player[1].castle.col = -1;
	mission[0].player[1].castle.row = -1;

	/* Mission 2: STATION */
	mission[1].rnd.state[0] = 0x60b9;
	mission[1].rnd.state[1] = 0xe728;
	mission[1].rnd.state[2] = 0xc484;

	mission[1].player[0].supplies = 30,
	mission[1].player[0].reproduction = 40,
	mission[1].player[0].castle.col = -1;
	mission[1].player[0].castle.row = -1;

	mission[1].player[1].face = 2,
	mission[1].player[1].intelligence = 12,
	mission[1].player[1].supplies = 15,
	mission[1].player[1].reproduction = 30,
	mission[1].player[1].castle.col = -1;
	mission[1].player[1].castle.row = -1;

	mission[1].player[2].face = 3,
	mission[1].player[2].intelligence = 14,
	mission[1].player[2].supplies = 15,
	mission[1].player[2].reproduction = 30,
	mission[1].player[2].castle.col = -1;
	mission[1].player[2].castle.row = -1;

		/* Mission 3: UNITY */
	mission[2].rnd.state[0] = 0x12ab;
	mission[2].rnd.state[1] = 0x7a4a;
	mission[2].rnd.state[2] = 0xe483;

	mission[2].player[0].supplies = 30,
	mission[2].player[0].reproduction = 30,
	mission[2].player[0].castle.col = -1;
	mission[2].player[0].castle.row = -1;

	mission[2].player[1].face = 2,
	mission[2].player[1].intelligence = 18,
	mission[2].player[1].supplies = 10,
	mission[2].player[1].reproduction = 25,
	mission[2].player[1].castle.col = -1;
	mission[2].player[1].castle.row = -1;

	mission[2].player[2].face = 4,
	mission[2].player[2].intelligence = 18,
	mission[2].player[2].supplies = 10,
	mission[2].player[2].reproduction = 25,
	mission[2].player[2].castle.col = -1;
	mission[2].player[2].castle.row = -1;

	/* Mission 4 */
	mission[3].rnd.state[0] = 0xacdf;
	mission[3].rnd.state[1] = 0xee65;
	mission[3].rnd.state[2] = 0x3701;

	mission[3].player[0].supplies = 25,
	mission[3].player[0].reproduction = 40,
	mission[3].player[0].castle.col = -1;
	mission[3].player[0].castle.row = -1;

	mission[3].player[1].face = 2,
	mission[3].player[1].intelligence = 15,
	mission[3].player[1].supplies = 20,
	mission[3].player[1].reproduction = 30,
	mission[3].player[1].castle.col = -1;
	mission[3].player[1].castle.row = -1;

	/* Mission 5 */
	mission[4].rnd.state[0] = 0x3b8b;
	mission[4].rnd.state[1] = 0xd867;
	mission[4].rnd.state[2] = 0xd847;

	mission[4].player[0].supplies = 30,
	mission[4].player[0].reproduction = 30,
	mission[4].player[0].castle.col = -1;
	mission[4].player[0].castle.row = -1;

	mission[4].player[1].face = 3,
	mission[4].player[1].intelligence = 16,
	mission[4].player[1].supplies = 25,
	mission[4].player[1].reproduction = 20,
	mission[4].player[1].castle.col = -1;
	mission[4].player[1].castle.row = -1;

	mission[4].player[2].face = 4,
	mission[4].player[2].intelligence = 16,
	mission[4].player[2].supplies = 25,
	mission[4].player[2].reproduction = 20,
	mission[4].player[2].castle.col = -1;
	mission[4].player[2].castle.row = -1;

	/* Mission 6 */
	mission[5].rnd.state[0] = 0x4491;
	mission[5].rnd.state[1] = 0x36fb;
	mission[5].rnd.state[2] = 0xf9e1;

	mission[5].player[0].supplies = 30,
	mission[5].player[0].reproduction = 30,
	mission[5].player[0].castle.col = -1;
	mission[5].player[0].castle.row = -1;

	mission[5].player[1].face = 3,
	mission[5].player[1].intelligence = 20,
	mission[5].player[1].supplies = 12,
	mission[5].player[1].reproduction = 14,
	mission[5].player[1].castle.col = -1;
	mission[5].player[1].castle.row = -1;

	mission[5].player[2].face = 5,
	mission[5].player[2].intelligence = 20,
	mission[5].player[2].supplies = 12,
	mission[5].player[2].reproduction = 14,
	mission[5].player[2].castle.col = -1;
	mission[5].player[2].castle.row = -1;

	/* Mission 7 */
	mission[6].rnd.state[0] = 0xca18;
	mission[6].rnd.state[1] = 0x4221;
	mission[6].rnd.state[2] = 0x7f96;

	mission[6].player[0].supplies = 30,
	mission[6].player[0].reproduction = 40,
	mission[6].player[0].castle.col = -1;
	mission[6].player[0].castle.row = -1;

	mission[6].player[1].face = 3,
	mission[6].player[1].intelligence = 22,
	mission[6].player[1].supplies = 30,
	mission[6].player[1].reproduction = 30,
	mission[6].player[1].castle.col = -1;
	mission[6].player[1].castle.row = -1;

	/* Mission 8 */
	mission[7].rnd.state[0] = 0x88fe;
	mission[7].rnd.state[1] = 0xe0db;
	mission[7].rnd.state[2] = 0xed5c;

	mission[7].player[0].supplies = 25,
	mission[7].player[0].reproduction = 30,
	mission[7].player[0].castle.col = -1;
	mission[7].player[0].castle.row = -1;

	mission[7].player[1].face = 4,
	mission[7].player[1].intelligence = 23,
	mission[7].player[1].supplies = 25,
	mission[7].player[1].reproduction = 30,
	mission[7].player[1].castle.col = -1;
	mission[7].player[1].castle.row = -1;

	mission[7].player[2].face = 6,
	mission[7].player[2].intelligence = 24,
	mission[7].player[2].supplies = 25,
	mission[7].player[2].reproduction = 30,
	mission[7].player[2].castle.col = -1;
	mission[7].player[2].castle.row = -1;

	/* Mission 9 */
	mission[8].rnd.state[0] = 0xe9c4;
	mission[8].rnd.state[1] = 0x16fe;
	mission[8].rnd.state[2] = 0x2ef0;

	mission[8].player[0].supplies = 25,
	mission[8].player[0].reproduction = 40,
	mission[8].player[0].castle.col = -1;
	mission[8].player[0].castle.row = -1;

	mission[8].player[1].face = 4,
	mission[8].player[1].intelligence = 26,
	mission[8].player[1].supplies = 13,
	mission[8].player[1].reproduction = 30,
	mission[8].player[1].castle.col = -1;
	mission[8].player[1].castle.row = -1;

	mission[8].player[2].face = 5,
	mission[8].player[2].intelligence = 28,
	mission[8].player[2].supplies = 13,
	mission[8].player[2].reproduction = 30,
	mission[8].player[2].castle.col = -1;
	mission[8].player[2].castle.row = -1;

	mission[8].player[3].face = 6,
	mission[8].player[3].intelligence = 30,
	mission[8].player[3].supplies = 13,
	mission[8].player[3].reproduction = 30,
	mission[8].player[3].castle.col = -1;
	mission[8].player[3].castle.row = -1;

	/* Mission 10 */
	mission[9].rnd.state[0] = 0x15c2;
	mission[9].rnd.state[1] = 0xf9d0;
	mission[9].rnd.state[2] = 0x5fb1;

	mission[9].player[0].supplies = 20,
	mission[9].player[0].reproduction = 16,
	mission[9].player[0].castle.col = 28;
	mission[9].player[0].castle.row = 14;

	mission[9].player[1].face = 4,
	mission[9].player[1].intelligence = 30,
	mission[9].player[1].supplies = 19,
	mission[9].player[1].reproduction = 20,
	mission[9].player[1].castle.col = 5;
	mission[9].player[1].castle.row = 47;

	/* Mission 11 */
	mission[10].rnd.state[0] = 0x9b93;
	mission[10].rnd.state[1] = 0x6be1;
	mission[10].rnd.state[2] = 0x79c0;

	mission[10].player[0].supplies = 16,
	mission[10].player[0].reproduction = 20,
	mission[10].player[0].castle.col = 16;
	mission[10].player[0].castle.row = 42;

	mission[10].player[1].face = 5,
	mission[10].player[1].intelligence = 33,
	mission[10].player[1].supplies = 10,
	mission[10].player[1].reproduction = 20,
	mission[10].player[1].castle.col = 0x34;
	mission[10].player[1].castle.row = 0x19;

	mission[10].player[2].face = 7,
	mission[10].player[2].intelligence = 34,
	mission[10].player[2].supplies = 13,
	mission[10].player[2].reproduction = 20,
	mission[10].player[2].castle.col = 0x17;
	mission[10].player[2].castle.row = 12;

	/* Mission 12 */
	mission[11].rnd.state[0] = 0x4195;
	mission[11].rnd.state[1] = 0x7dba;
	mission[11].rnd.state[2] = 0xd884;

	mission[11].player[0].supplies = 23,
	mission[11].player[0].reproduction = 27,
	mission[11].player[0].castle.col = 0x35;
	mission[11].player[0].castle.row = 13;

	mission[11].player[1].face = 5,
	mission[11].player[1].intelligence = 27,
	mission[11].player[1].supplies = 17,
	mission[11].player[1].reproduction = 24,
	mission[11].player[1].castle.col = 0x1b;
	mission[11].player[1].castle.row = 10;

	mission[11].player[2].face = 6,
	mission[11].player[2].intelligence = 27,
	mission[11].player[2].supplies = 13,
	mission[11].player[2].reproduction = 24,
	mission[11].player[2].castle.col = 0x1d;
	mission[11].player[2].castle.row = 0x26;

	mission[11].player[3].face = 7,
	mission[11].player[3].intelligence = 27,
	mission[11].player[3].supplies = 13,
	mission[11].player[3].reproduction = 24,
	mission[11].player[3].castle.col = 15;
	mission[11].player[3].castle.row = 32;

	/* Mission 13: ISLAND */
	mission[12].rnd.state[0] = 0x259f;
	mission[12].rnd.state[1] = 0xcea6;
	mission[12].rnd.state[2] = 0xc000;

	mission[12].player[0].supplies = 24;
	mission[12].player[0].reproduction = 20;
	mission[12].player[0].castle.col = 7;
	mission[12].player[0].castle.row = 26;

	mission[12].player[1].face = 5;
	mission[12].player[1].intelligence = 20;
	mission[12].player[1].supplies = 30;
	mission[12].player[1].reproduction = 20;
	mission[12].player[1].castle.col = 2;
	mission[12].player[1].castle.row = 10;

	/* Mission 14: LEGION */
	mission[13].rnd.state[0] = 0x7d40;
	mission[13].rnd.state[1] = 0xc22e;
	mission[13].rnd.state[2] = 0x75bf;

	mission[13].player[0].supplies = 20;
	mission[13].player[0].reproduction = 23;
	mission[13].player[0].castle.col = 19;
	mission[13].player[0].castle.row = 3;

	mission[13].player[1].face = 6;
	mission[13].player[1].intelligence = 28;
	mission[13].player[1].supplies = 16;
	mission[13].player[1].reproduction = 20;
	mission[13].player[1].castle.col = 55;
	mission[13].player[1].castle.row = 7;

	mission[13].player[2].face = 8;
	mission[13].player[2].intelligence = 28;
	mission[13].player[2].supplies = 16;
	mission[13].player[2].reproduction = 20;
	mission[13].player[2].castle.col = 55;
	mission[13].player[2].castle.row = 46;

	/* Mission 15: PIECE */
	mission[14].rnd.state[0] = 0xb1a1;
	mission[14].rnd.state[1] = 0x86a6;
	mission[14].rnd.state[2] = 0x61c3;

	mission[14].player[0].supplies = 20;
	mission[14].player[0].reproduction = 17;
	mission[14].player[0].castle.col = 41;
	mission[14].player[0].castle.row = 5;

	mission[14].player[1].face = 6;
	mission[14].player[1].intelligence = 40;
	mission[14].player[1].supplies = 23;
	mission[14].player[1].reproduction = 20;
	mission[14].player[1].castle.col = 19;
	mission[14].player[1].castle.row = 49;

	mission[14].player[2].face = 7;
	mission[14].player[2].intelligence = 37;
	mission[14].player[2].supplies = 20;
	mission[14].player[2].reproduction = 20;
	mission[14].player[2].castle.col = 58;
	mission[14].player[2].castle.row = 52;

	mission[14].player[3].face = 8;
	mission[14].player[3].intelligence = 40;
	mission[14].player[3].supplies = 15;
	mission[14].player[3].reproduction = 15;
	mission[14].player[3].castle.col = 43;
	mission[14].player[3].castle.row = 31;

	/* Mission 16: RIVAL */
	mission[15].rnd.state[0] = 0x5563;
	mission[15].rnd.state[1] = 0x46ea;
	mission[15].rnd.state[2] = 0xde0c;

	mission[15].player[0].supplies = 26;
	mission[15].player[0].reproduction = 23;
	mission[15].player[0].castle.col = 36;
	mission[15].player[0].castle.row = 63;

	mission[15].player[1].face = 6;
	mission[15].player[1].intelligence = 28;
	mission[15].player[1].supplies = 29;
	mission[15].player[1].reproduction = 40;
	mission[15].player[1].castle.col = 14;
	mission[15].player[1].castle.row = 15;

	/* Mission 17: SAVAGE */
	mission[16].rnd.state[0] = 0x820e;
	mission[16].rnd.state[1] = 0x3971;
	mission[16].rnd.state[2] = 0x6058;

	mission[16].player[0].supplies = 25;
	mission[16].player[0].reproduction = 12;
	mission[16].player[0].castle.col = 63;
	mission[16].player[0].castle.row = 59;

	mission[16].player[1].face = 7;
	mission[16].player[1].intelligence = 29;
	mission[16].player[1].supplies = 17;
	mission[16].player[1].reproduction = 10;
	mission[16].player[1].castle.col = 29;
	mission[16].player[1].castle.row = 24;

	mission[16].player[2].face = 8;
	mission[16].player[2].intelligence = 29;
	mission[16].player[2].supplies = 17;
	mission[16].player[2].reproduction = 10;
	mission[16].player[2].castle.col = 39;
	mission[16].player[2].castle.row = 26;

	mission[16].player[3].face = 9;
	mission[16].player[3].intelligence = 32;
	mission[16].player[3].supplies = 17;
	mission[16].player[3].reproduction = 10;
	mission[16].player[3].castle.col = 42;
	mission[16].player[3].castle.row = 49;

	/* Mission 18: XAVER */
	mission[17].rnd.state[0] = 0x3b8b;
	mission[17].rnd.state[1] = 0xd867;
	mission[17].rnd.state[2] = 0xd844;

	mission[17].player[0].supplies = 25;
	mission[17].player[0].reproduction = 40;
	mission[17].player[0].castle.col = 15;
	mission[17].player[0].castle.row = 0;

	mission[17].player[1].face = 7;
	mission[17].player[1].intelligence = 40;
	mission[17].player[1].supplies = 30;
	mission[17].player[1].reproduction = 35;
	mission[17].player[1].castle.col = 34;
	mission[17].player[1].castle.row = 48;

	mission[17].player[2].face = 9;
	mission[17].player[2].intelligence = 30;
	mission[17].player[2].supplies = 30;
	mission[17].player[2].reproduction = 35;
	mission[17].player[2].castle.col = 58;
	mission[17].player[2].castle.row = 5;

	/* Mission 19: BLADE */
	mission[18].rnd.state[0] = 0xe2c6;
	mission[18].rnd.state[1] = 0xc37d;
	mission[18].rnd.state[2] = 0xbf32;

	mission[18].player[0].supplies = 30;
	mission[18].player[0].reproduction = 20;
	mission[18].player[0].castle.col = 13;
	mission[18].player[0].castle.row = 37;

	mission[18].player[1].face = 7;
	mission[18].player[1].intelligence = 40;
	mission[18].player[1].supplies = 20;
	mission[18].player[1].reproduction = 20;
	mission[18].player[1].castle.col = 32;
	mission[18].player[1].castle.row = 34;

	/* Mission 20: BEACON */
	mission[19].rnd.state[0] = 0x83fd;
	mission[19].rnd.state[1] = 0x045f;
	mission[19].rnd.state[2] = 0xbfa4;

	mission[19].player[0].supplies = 9;
	mission[19].player[0].reproduction = 10;
	mission[19].player[0].castle.col = 14;
	mission[19].player[0].castle.row = 42;

	mission[19].player[1].face = 8;
	mission[19].player[1].intelligence = 40;
	mission[19].player[1].supplies = 16;
	mission[19].player[1].reproduction = 22;
	mission[19].player[1].castle.col = 62;
	mission[19].player[1].castle.row = 1;

	mission[19].player[2].face = 9;
	mission[19].player[2].intelligence = 40;
	mission[19].player[2].supplies = 16;
	mission[19].player[2].reproduction = 23;
	mission[19].player[2].castle.col = 32;
	mission[19].player[2].castle.row = 14;

	/* Mission 21: PASTURE */
	mission[20].rnd.state[0] = 0x02f6;
	mission[20].rnd.state[1] = 0x2275;
	mission[20].rnd.state[2] = 0xa9aa;

	mission[20].player[0].supplies = 20;
	mission[20].player[0].reproduction = 11;
	mission[20].player[0].castle.col = 38;
	mission[20].player[0].castle.row = 17;

	mission[20].player[1].face = 8;
	mission[20].player[1].intelligence = 30;
	mission[20].player[1].supplies = 22;
	mission[20].player[1].reproduction = 13;
	mission[20].player[1].castle.col = 32;
	mission[20].player[1].castle.row = 51;

	mission[20].player[2].face = 9;
	mission[20].player[2].intelligence = 30;
	mission[20].player[2].supplies = 23;
	mission[20].player[2].reproduction = 13;
	mission[20].player[2].castle.col = 1;
	mission[20].player[2].castle.row = 50;

	mission[20].player[3].face = 10;
	mission[20].player[3].intelligence = 30;
	mission[20].player[3].supplies = 21;
	mission[20].player[3].reproduction = 13;
	mission[20].player[3].castle.col = 4;
	mission[20].player[3].castle.row = 9;

	/* Mission 22: OMNUS */
	mission[21].rnd.state[0] = 0xa775;
	mission[21].rnd.state[1] = 0x79db;
	mission[21].rnd.state[2] = 0x8732;

	mission[21].player[0].supplies = 20;
	mission[21].player[0].reproduction = 40;
	mission[21].player[0].castle.col = 42;
	mission[21].player[0].castle.row = 20;

	mission[21].player[1].face = 8;
	mission[21].player[1].intelligence = 36;
	mission[21].player[1].supplies = 25;
	mission[21].player[1].reproduction = 40;
	mission[21].player[1].castle.col = 48;
	mission[21].player[1].castle.row = 47;

	/* Mission 23: TRIBUTE */
	mission[22].rnd.state[0] = 0xeefc;
	mission[22].rnd.state[1] = 0x9acd;
	mission[22].rnd.state[2] = 0x5085;

	mission[22].player[0].supplies = 5;
	mission[22].player[0].reproduction = 11;
	mission[22].player[0].castle.col = 53;
	mission[22].player[0].castle.row = 1;

	mission[22].player[1].face = 9;
	mission[22].player[1].intelligence = 35;
	mission[22].player[1].supplies = 30;
	mission[22].player[1].reproduction = 10;
	mission[22].player[1].castle.col = 20;
	mission[22].player[1].castle.row = 2;

	mission[22].player[2].face = 10;
	mission[22].player[2].intelligence = 37;
	mission[22].player[2].supplies = 30;
	mission[22].player[2].reproduction = 10;
	mission[22].player[2].castle.col = 16;
	mission[22].player[2].castle.row = 55;

	/* Mission 24: FOUNTAIN */
	mission[23].rnd.state[0] = 0xc5c5;
	mission[23].rnd.state[1] = 0xfae1;
	mission[23].rnd.state[2] = 0x69bc;

	mission[23].player[0].supplies = 20;
	mission[23].player[0].reproduction = 12;
	mission[23].player[0].castle.col = 3;
	mission[23].player[0].castle.row = 34;

	mission[23].player[1].face = 9;
	mission[23].player[1].intelligence = 30;
	mission[23].player[1].supplies = 25;
	mission[23].player[1].reproduction = 10;
	mission[23].player[1].castle.col = 47;
	mission[23].player[1].castle.row = 41;

	mission[23].player[2].face = 10;
	mission[23].player[2].intelligence = 30;
	mission[23].player[2].supplies = 26;
	mission[23].player[2].reproduction = 10;
	mission[23].player[2].castle.col = 42;
	mission[23].player[2].castle.row = 52;

	/* Mission 25: CHUDE */
	mission[24].rnd.state[0] = 0x84ae;
	mission[24].rnd.state[1] = 0x70f4;
	mission[24].rnd.state[2] = 0x4bc6;

	mission[24].player[0].supplies = 20;
	mission[24].player[0].reproduction = 40;
	mission[24].player[0].castle.col = 23;
	mission[24].player[0].castle.row = 38;

	mission[24].player[1].face = 9;
	mission[24].player[1].intelligence = 40;
	mission[24].player[1].supplies = 25;
	mission[24].player[1].reproduction = 40;
	mission[24].player[1].castle.col = 57;
	mission[24].player[1].castle.row = 52;

	/* Mission 26: TRAILER */
	mission[25].rnd.state[0] = 0x8724;
	mission[25].rnd.state[1] = 0x56cd;
	mission[25].rnd.state[2] = 0x2157;

	mission[25].player[0].supplies = 20;
	mission[25].player[0].reproduction = 30;
	mission[25].player[0].castle.col = 29;
	mission[25].player[0].castle.row = 11;

	mission[25].player[1].face = 10;
	mission[25].player[1].intelligence = 38;
	mission[25].player[1].supplies = 30;
	mission[25].player[1].reproduction = 35;
	mission[25].player[1].castle.col = 15;
	mission[25].player[1].castle.row = 40;

	/* Mission 27: CANYON */
	mission[26].rnd.state[0] = 0x3242;
	mission[26].rnd.state[1] = 0x4801;
	mission[26].rnd.state[2] = 0xd21f;

	mission[26].player[0].supplies = 18;
	mission[26].player[0].reproduction = 28;
	mission[26].player[0].castle.col = 49;
	mission[26].player[0].castle.row = 53;

	mission[26].player[1].face = 10;
	mission[26].player[1].intelligence = 39;
	mission[26].player[1].supplies = 25;
	mission[26].player[1].reproduction = 40;
	mission[26].player[1].castle.col = 14;
	mission[26].player[1].castle.row = 53;

	/* Mission 28: REPRESS */
	mission[27].rnd.state[0] = 0x3f61;
	mission[27].rnd.state[1] = 0xd9a4;
	mission[27].rnd.state[2] = 0xb4f7;

	mission[27].player[0].supplies = 20;
	mission[27].player[0].reproduction = 40;
	mission[27].player[0].castle.col = 44;
	mission[27].player[0].castle.row = 39;

	mission[27].player[1].face = 10;
	mission[27].player[1].intelligence = 39;
	mission[27].player[1].supplies = 25;
	mission[27].player[1].reproduction = 40;
	mission[27].player[1].castle.col = 44;
	mission[27].player[1].castle.row = 63;

	/* Mission 29: YOKI */
	mission[28].rnd.state[0] = 0xdab2;
	mission[28].rnd.state[1] = 0x5453;
	mission[28].rnd.state[2] = 0xea3e;

	mission[28].player[0].supplies = 5;
	mission[28].player[0].reproduction = 22;
	mission[28].player[0].castle.col = 53;
	mission[28].player[0].castle.row = 8;

	mission[28].player[1].face = 11;
	mission[28].player[1].intelligence = 40;
	mission[28].player[1].supplies = 15;
	mission[28].player[1].reproduction = 20;
	mission[28].player[1].castle.col = 30;
	mission[28].player[1].castle.row = 22;

	/* Mission 30: PASSIVE */
	mission[29].rnd.state[0] = 0x319c;
	mission[29].rnd.state[1] = 0x22be;
	mission[29].rnd.state[2] = 0xc149;

	mission[29].player[0].supplies = 5;
	mission[29].player[0].reproduction = 20;
	mission[29].player[0].castle.col = 25;
	mission[29].player[0].castle.row = 46;

	mission[29].player[1].face = 11;
	mission[29].player[1].intelligence = 40;
	mission[29].player[1].supplies = 20;
	mission[29].player[1].reproduction = 20;
	mission[29].player[1].castle.col = 51;
	mission[29].player[1].castle.row = 42;
}

const int mission_count = sizeof(mission) / sizeof(mission[0]);
