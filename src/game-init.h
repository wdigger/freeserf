/*
 * game-init.h - Game initialization GUI component header
 *
 * Copyright (C) 2013-2017  Jon Lund Steffensen <jonlst@gmail.com>
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

#ifndef SRC_GAME_INIT_H_
#define SRC_GAME_INIT_H_

#include <memory>

#include "src/dialog.h"
#include "src/mission.h"

class Interface;
class Minimap;

class GameInitBox : public Dialog {
 protected:
  typedef enum GameType {
    GameNone = -1,
    GameCustom = 0,
    GameMission = 1,
    GameLoad = 2
  } GameType;

 protected:
  Interface *interface;

  int game_type;
  int game_mission;

  PGameInfo custom_mission;
  PGameInfo mission;

  std::shared_ptr<Minimap> minimap;
  std::shared_ptr<ListSavedFiles> file_list;
  PLayout layout_custom;
  PLayout layout_mission;
  PLayout layout_load;

 public:
  explicit GameInitBox(Interface *interface);
  virtual ~GameInitBox();

 protected:
  PLayout create_layout_custom();
  PLayout create_layout_mission();
  PLayout create_layout_load();

  void apply_random(Random rnd);
  void generate_map_preview();

  virtual void init();
  virtual void draw_background();
};

#endif  // SRC_GAME_INIT_H_
