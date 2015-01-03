/*
 * random.c - Random number generator
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

#include "random.h"
#include "freeserf_endian.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <string.h>

uint16_t
random_int(random_state_t *random)
{
	uint16_t *rnd = random->state;
	uint16_t r = (rnd[0] + rnd[1]) ^ rnd[2];
	rnd[2] += rnd[1];
	rnd[1] ^= rnd[2];
	rnd[1] = (rnd[1] >> 1) | (rnd[1] << 15);
	rnd[2] = (rnd[2] >> 1) | (rnd[2] << 15);
	rnd[0] = r;

	return r;
}

random_state_t
random_generate_random_state()
{
  random_state_t random;

  srand((unsigned int)time(NULL));
  random.state[0] = rand();
  random.state[1] = rand();
  random.state[2] = rand();
  random_int(&random);

  return random;
}

char *
random_to_string(random_state_t *random)
{
  uint64_t tmp0 = random->state[0];
  uint64_t tmp1 = random->state[1];
  uint64_t tmp2 = random->state[2];

  uint64_t tmp = tmp0;
  tmp |= tmp1 << 16;
  tmp |= tmp2 << 32;

  char str[17] = {0};

  for (int i = 0; i < 16; i++) {
    uint8_t c = tmp & 0x07;
    str[i] = '0' + c + 1;
    tmp >>= 3;
  }

  return strdup(str);
}

random_state_t
string_to_random(const char *str)
{
  random_state_t random;
  uint64_t tmp = 0;

  for (int i = 15; i >= 0; i--) {
    tmp <<= 3;
    uint8_t c = str[i] - '0' - 1;
    tmp |= c;
  }

  random.state[0] = tmp & 0xFFFF;
  tmp >>= 16;
  random.state[1] = tmp & 0xFFFF;
  tmp >>= 16;
  random.state[2] = tmp & 0xFFFF;

  return random;
}
