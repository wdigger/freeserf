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

#include "application.h"

application_t *application_t::instance = NULL;

extern video_t *create_video();
extern audio_t *create_audio();
extern event_loop_t *create_event_loop();

application_t *
application_t::get_application()
{
  if (instance == NULL) {
    instance = new application_t();
  }

  return instance;
}

application_t::application_t()
{
  video = NULL;
  audio = NULL;
  event_loop = NULL;
}

application_t::~application_t()
{
  if (video != NULL) {
    delete video;
    video = NULL;
  }

  if (audio != NULL) {
    delete audio;
    audio = NULL;
  }

  if (event_loop != NULL) {
    delete event_loop;
    event_loop = NULL;
  }
}

video_t *
application_t::get_video()
{
  if (video == NULL) {
    video = create_video();
  }
  return video;
}

audio_t *
application_t::get_audio()
{
  if (audio == NULL) {
    audio = create_audio();
  }
  return audio;
}

event_loop_t *
application_t::get_event_loop()
{
  if (event_loop == NULL) {
    event_loop = create_event_loop();
  }
  return event_loop;
}

void
application_t::quit()
{
  event_loop_t *event_loop = get_event_loop();
  if (event_loop == NULL) {
    return;
  }

  event_loop->quit();
}
