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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

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
	random_state_t random = {{ 0x5a5a, (uint16_t)(time(NULL) >> 16), (uint16_t)time(NULL) }};
	return random;
}

random_state_t
random_generate_random_state2()
{
  random_state_t random;

  srand((unsigned int)time(NULL));
  random.state[0] = rand();
  random.state[1] = rand();
  random.state[2] = rand();
  random_int(&random);

  return random;
}
