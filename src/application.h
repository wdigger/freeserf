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

#ifndef SRC_APPLICATION_H_
#define SRC_APPLICATION_H_

class Video;
class Audio;
class EventLoop;

class Application {
 public:
  Application();
  virtual ~Application();

  static Application &get_instance();
  Video &get_video();
  Audio &get_audio();
  EventLoop &get_event_loop();

  void quit();
};

#endif  // SRC_APPLICATION_H_
