/*
 * event_loop-net.h - User and system events handling
 *
 * Copyright (C) 2016  Wicked_Digger <wicked_digger@mail.ru>
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

#ifndef SRC_EVENT_LOOP_NET_H_
#define SRC_EVENT_LOOP_NET_H_

#include <list>
#include <SDL_net.h>

#include "src/event_loop.h"

class NetConnection {
 protected:
  TCPsocket socket;

 public:
  explicit NetConnection(const TCPsocket &socket);
  virtual ~NetConnection();

  const TCPsocket &get_socket() const;
  bool process();
  void close();
};

typedef std::list<NetConnection*> NetConnections;

class EventLoopNet : public EventLoop {
 protected:
  NetConnections connections;

 public:
  EventLoopNet();

  virtual void quit();
  virtual void run();
  virtual void deferred_call(DeferredCallee *deferred_callee, void *data);
};

#endif  // SRC_EVENT_LOOP_NET_H_
