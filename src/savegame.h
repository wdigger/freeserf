/*
 * savegame.h - Loading and saving of save games
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

#ifndef _SAVEGAME_H
#define _SAVEGAME_H

#include <stdio.h>

/* Original game format */
int load_v0_state(FILE *f);

/* Text format */
int save_text_state(FILE *f);
int load_text_state(FILE *f);

/* Generic save/load function that will try to detect the right
   format on load and save to the best format on write. */
int save_state(const char *path);
int load_state(const char *path);

int save_game(int autosave);

#endif /* !_SAVEGAME_H */
