/*
 * event_loop-net.cc - User and system events handling
 *
 * Copyright (C) 2012-2014  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "src/event_loop-net.h"

#include <SDL_net.h>

#include "src/log.h"
#include "src/freeserf.h"

EventLoop *
EventLoop::get_instance() {
  if (instance == NULL) {
    instance = new EventLoopNet();
  }

  return instance;
}

EventLoop *
create_event_loop() {
  return new EventLoopNet();
}

EventLoopNet::EventLoopNet() {
}

void
EventLoopNet::quit() {
}

void
EventLoopNet::deferred_call(DeferredCallee *deferred_callee, void *data) {
}

#define MAX_CLIENTS (100)
#define SERVER_PORT (1994)

/* event_loop() has been turned into a SDL based loop.
 The code for one iteration of the original game_loop is
 in game_loop_iter. */
void
EventLoopNet::run() {
  if (SDL_Init(0) == -1) {
    printf("SDL_Init: %s\n", SDL_GetError());
    return;
  }

  if (SDLNet_Init() == -1) {
    printf("SDLNet_Init: %s\n", SDLNet_GetError());
    return;
  }

  IPaddress ip;
  TCPsocket serverSocket;

  if (SDLNet_ResolveHost(&ip, NULL, SERVER_PORT) == -1) {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return;
  }

  serverSocket = SDLNet_TCP_Open(&ip);
  if (!serverSocket) {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return;
  }

  SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(MAX_CLIENTS);
  if (socketSet == NULL) {
    printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    return;
  }

  SDLNet_TCP_AddSocket(socketSet, serverSocket);

  do {
    int sockets = SDLNet_CheckSockets(socketSet, TICK_LENGTH);
    if (sockets == -1) {
      printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
      break;
    } else if (sockets == 0) {
      // Do game loop
      notify_update();
    } else {
      if (SDLNet_SocketReady(serverSocket) != 0) {
        // Connect new client
        TCPsocket clientSocket = SDLNet_TCP_Accept(serverSocket);
        NetConnection *connection = new NetConnection(clientSocket);
        connections.push_back(connection);
        SDLNet_TCP_AddSocket(socketSet, clientSocket);
      } else {
        NetConnections cons = connections;
        NetConnections::iterator it = cons.begin();
        for (; it != cons.end(); ++it) {
          if (SDLNet_SocketReady((*it)->get_socket()) != 0) {
            if (!(*it)->process()) {
              SDLNet_TCP_DelSocket(socketSet, (*it)->get_socket());
              SDLNet_TCP_Close((*it)->get_socket());
              connections.remove(*it);
              delete *it;
            }
          }
        }
      }
    }
  } while (true);

  SDLNet_FreeSocketSet(socketSet);
  SDLNet_TCP_Close(serverSocket);
  SDLNet_Quit();
  SDL_Quit();
}

NetConnection::NetConnection(const TCPsocket &_socket) {
  socket = _socket;
}

NetConnection::~NetConnection() {
  close();
}

const TCPsocket &
NetConnection::get_socket() const {
  return socket;
}

bool
NetConnection::process() {
  return false;
}

void
NetConnection::close() {
  if (socket != NULL) {
    SDLNet_TCP_Close(socket);
    socket = NULL;
  }
}
