/*
 * fsserver.cc - Main program source.
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

#include <string>

#include "src/freeserf.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#include "src/log.h"
#include "src/version.h"
#include "src/game.h"

#define USAGE                                               \
  "Usage: %s [options]\n"
#define HELP                                                \
  USAGE                                                     \
      " -d NUM\t\tSet debug output level\n"                 \
      " -h\t\tShow this help text\n"                        \
      " -l FILE\tLoad saved game\n"                         \
      " -t GEN\t\tMap generator (0 or 1)\n"                 \
      "\n"                                                  \
      "Please report bugs to <" PACKAGE_BUGREPORT ">\n"

int
main(int argc, char *argv[]) {
  std::string save_file;

#ifdef HAVE_GETOPT_H
  while (true) {
    char opt = getopt(argc, argv, "d:fg:hl:r:");
    if (opt < 0) break;

    switch (opt) {
      case 'd': {
          int d = atoi(optarg);
          if (d >= 0 && d < Log::LevelMax) {
            Log::set_level(static_cast<Log::Level>(d));
          }
        }
        break;
      case 'h':
        fprintf(stdout, HELP, argv[0]);
        exit(EXIT_SUCCESS);
        break;
      case 'l':
        if (strlen(optarg) > 0) {
          save_file = optarg;
        }
        break;
      default:
        fprintf(stderr, USAGE, argv[0]);
        exit(EXIT_FAILURE);
        break;
    }
  }
#endif

  /* Set up logging */
  Log::Info["server"] << "freeserf " << FREESERF_VERSION;

  Game *game = new Game();
  game->init();

  /* Either load a save game if specified or
     start a new game. */
  if (!save_file.empty()) {
    if (!game->load_save_game(save_file)) exit(EXIT_FAILURE);
  } else {
    PGameInfo game_info(new GameInfo(Random()));
    if (!game->load_mission_map(game_info)) exit(EXIT_FAILURE);
  }

  /* Init game loop */
  EventLoop *event_loop = EventLoop::get_instance();
  event_loop->add_handler(game);

  /* Start game loop */
  event_loop->run();

  event_loop->del_handler(game);

  Log::Info["server"] << "Cleaning up...";

  /* Clean up */
  if (game != NULL) {
    EventLoop::get_instance()->del_handler(game);
    delete game;
    game = NULL;
  }
  delete event_loop;

  return EXIT_SUCCESS;
}
