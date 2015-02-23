/*
 * random.h - Random number generator
 *
 * Copyright (C) 2012  Jon Lund Steffensen <jonlst@gmail.com>
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

#ifndef _RANDOM_H
#define _RANDOM_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

typedef struct {
	uint16_t state[3];
} random_state_t;

uint16_t random_int(random_state_t *random);
random_state_t random_generate_random_state();
char *random_to_string(const random_state_t *random);
random_state_t string_to_random(const char *str);

#endif /* ! _RANDOM_H */
