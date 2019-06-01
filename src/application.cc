/*
 * application.cc - Platform dependent functionality factory.
 *
 * Copyright (C) 2019  Wicked_Digger <wicked_digger@mail.ru>
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

#include "src/application.h"

#include "src/video.h"
#include "src/audio.h"
#include "src/event_loop.h"

Application &
Application::get_instance() {
  static Application instance;
  return instance;
}

Application::Application() {
}

Application::~Application() {
}

Video &
Application::get_video() {
  return Video::get_instance();
}

Audio &
Application::get_audio() {
  return Audio::get_instance();
}

EventLoop &
Application::get_event_loop() {
  return EventLoop::get_instance();
}

void
Application::quit() {
  EventLoop &el = get_event_loop();
  el.quit();
}
