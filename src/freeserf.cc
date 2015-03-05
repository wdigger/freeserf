/*
 * freeserf.cc - Main program source.
 *
 * Copyright (C) 2013-2014  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "interface.h"
#include "viewport.h"
#include "data.h"
#include "application.h"

#ifndef _MSC_VER
extern "C" {
#endif
  #include "log.h"
  #include "mission.h"
  #include "version.h"
  #include "game.h"

  #ifdef HAVE_CONFIG_H
  # include <config.h>
  #endif
#ifndef _MSC_VER
}
#else
#include "getopt-win.h"
#define PACKAGE_BUGREPORT "https://github.com/freeserf/freeserf/issues"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <SDL.h>

#define DEFAULT_SCREEN_WIDTH  800
#define DEFAULT_SCREEN_HEIGHT 600

#ifndef DEFAULT_LOG_LEVEL
# ifndef NDEBUG
#  define DEFAULT_LOG_LEVEL  LOG_LEVEL_DEBUG
# else
#  define DEFAULT_LOG_LEVEL  LOG_LEVEL_INFO
# endif
#endif

class game_wrapper_t
  : public event_handler_t
{
public:
  virtual bool handle_event(const event_t *event) {
    switch (event->type) {
      case EVENT_UPDATE:
        game_update();
		return true;
        break;
      default:
        break;
    }
	return false;
  }
};

#define USAGE					\
  "Usage: %s [-g DATA-FILE]\n"
#define HELP                                                            \
  USAGE                                                                 \
      " -d NUM\t\tSet debug output level\n"				\
      " -f\t\tFullscreen mode (CTRL-q to exit)\n"			\
      " -g DATA-FILE\tUse specified data file\n"			\
      " -h\t\tShow this help text\n"					\
      " -l FILE\tLoad saved game\n"					\
      " -r RES\t\tSet display resolution (e.g. 800x600)\n"		\
      " -t GEN\t\tMap generator (0 or 1)\n"				\
      "\n"								\
      "Please report bugs to <" PACKAGE_BUGREPORT ">\n"

int
freeserf_main(int argc, char *argv[])
{
  int r;

  char *data_file = NULL;
  char *save_file = NULL;

  unsigned int screen_width = DEFAULT_SCREEN_WIDTH;
  unsigned int screen_height = DEFAULT_SCREEN_HEIGHT;
  bool fullscreen = false;
  int map_generator = 0;

  init_missions();

  log_level_t log_level = DEFAULT_LOG_LEVEL;

#ifdef HAVE_GETOPT
  char opt;
  while (1) {
    opt = getopt(argc, argv, "d:fg:hl:r:t:");
    if (opt < 0) break;

    switch (opt) {
      case 'd':
        {
          int d = atoi(optarg);
          if (d >= 0 && d < LOG_LEVEL_MAX) {
            log_level = static_cast<log_level_t>(d);
          }
        }
        break;
      case 'f':
        fullscreen = 1;
        break;
      case 'g':
        if (data_file != NULL) {
          free(data_file);
          data_file = NULL;
        }
        if (strlen(optarg) > 0) {
          data_file = static_cast<char*>(malloc(strlen(optarg)+1));
          if (data_file == NULL) exit(EXIT_FAILURE);
          strcpy(data_file, optarg);
        }
        break;
      case 'h':
        fprintf(stdout, HELP, argv[0]);
        exit(EXIT_SUCCESS);
        break;
      case 'l':
        if (save_file != NULL) {
          free(save_file);
          save_file = NULL;
        }
        if (strlen(optarg) > 0) {
          save_file = static_cast<char*>(malloc(strlen(optarg)+1));
          if (save_file == NULL) exit(EXIT_FAILURE);
          strcpy(save_file, optarg);
        }
        break;
      case 'r':
        {
          char *hstr = strchr(optarg, 'x');
          if (hstr == NULL) {
            fprintf(stderr, USAGE, argv[0]);
            exit(EXIT_FAILURE);
          }
          screen_width = atoi(optarg);
          screen_height = atoi(hstr+1);
        }
        break;
      case 't':
        map_generator = atoi(optarg);
        break;
      default:
        fprintf(stderr, USAGE, argv[0]);
        exit(EXIT_FAILURE);
        break;
    }
  }
#endif

  /* Set up logging */
  log_set_file(stdout);
  log_set_level(log_level);

  LOGI("main", "freeserf %s", FREESERF_VERSION);

  r = data_init(data_file);
  if (data_file != NULL) {
    free(data_file);
  }
  if (r < 0) {
    LOGE("main", "Could not load game data.");
    exit(EXIT_FAILURE);
  }

  LOGI("main", "Initialize graphics...");
  gfx_t *gfx = new gfx_t(screen_width, screen_height, fullscreen);

  application_t *application = application_t::get_application();

  /* TODO move to right place */
  audio_t *audio = application->get_audio();
  if (audio != NULL) {
    audio->set_volume(75);
    audio->midi_play_track(MIDI_TRACK_0);
  }

  game_init(map_generator);
  game_wrapper_t *game_wrapper = new game_wrapper_t();

  /* Initialize interface */
  gfx->get_resolution(screen_width, screen_height);

  interface_t *interface = new interface_t();
  interface->set_size(screen_width, screen_height);
  interface->set_displayed(1);

  /* Either load a save game if specified or
     start a new game. */
  if (save_file != NULL) {
    int r = game_load_save_game(save_file);
    if (r < 0) exit(EXIT_FAILURE);
    free(save_file);

    interface->set_player(0);
  } else {
    int r = game_load_random_map(3, interface->get_random());
    if (r < 0) exit(EXIT_FAILURE);

    /* Add default player */
    r = game_add_player(12, 64, 40, 40, 40);
    if (r < 0) exit(EXIT_FAILURE);

    interface->set_player(r);
  }

  viewport_t *viewport = interface->get_viewport();
  viewport->map_reinit();

  if (save_file != NULL) {
    interface->close_game_init();
  }

  /* Start game loop */
  event_handler_t *handlers[] = {
    interface, game_wrapper, NULL
  };

  application->get_event_loop()->run(handlers);

  LOGI("main", "Cleaning up...");

  /* Clean up */
  delete interface;
  map_deinit(&game.map);
  delete gfx;
  data_deinit();
  game_deinit();
  delete game_wrapper;
  delete application;

  return EXIT_SUCCESS;
}
