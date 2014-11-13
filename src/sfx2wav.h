/*
 * sfx2wav.h - SFX to WAV converter.
 *
 * Copyright (C) 2012-2014  Wicked_Digger <wicked_digger@mail.ru>
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

#ifndef _SFX2WAV_H
#define _SFX2WAV_H

#include <stdlib.h>

void *sfx2wav(void* sfx, size_t sfx_size, size_t *wav_size);

#endif /* ! _SFX2WAV_H */
