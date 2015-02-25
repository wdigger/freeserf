/*
 * application.cc - Platform dependent functionality factory.
 *
 * Copyright (C) 2015  Wicked_Digger <wicked_digger@mail.ru>
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

#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "video.h"
#include "audio.h"
#include "event_loop.h"

class application_t
{
protected:
  static application_t *instance;

  video_t *video;
  audio_t *audio;
  event_loop_t *event_loop;

public:
  application_t();
  virtual ~application_t();

  static application_t *get_application();
  video_t *get_video();
  audio_t *get_audio();
  event_loop_t *get_event_loop();

  void quit();
};

#endif /* !_PLATFORM_H */
