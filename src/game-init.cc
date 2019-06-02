/*
 * game-init.cc - Game initialization GUI component
 *
 * Copyright (C) 2013-2019  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "src/game-init.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <memory>

#include "src/game.h"
#include "src/interface.h"
#include "src/text-input.h"
#include "src/minimap.h"
#include "src/map-generator.h"
#include "src/list.h"
#include "src/game-manager.h"
#include "src/popup.h"
#include "src/version.h"

class RandomInput : public TextInput {
 protected:
  std::string saved_text;

 public:
  RandomInput() : TextInput(34, 34) {
    set_filter(text_input_filter);
    set_max_length(16);
  }

  void set_random(const Random &rnd) {
    std::string str = rnd;
    set_text(str);
  }
  Random get_random() {
    return Random(text);
  }

 protected:
  static bool
  text_input_filter(const char key, TextInput *text_input) {
    if (key < '1' || key > '8') {
      return false;
    }

    if (text_input->get_text().length() > 16) {
      return false;
    }

    return true;
  }

  virtual bool handle_click_left(int cx, int cy) {
    TextInput::handle_click_left(cx, cy);
    saved_text = text;
    text.clear();
    return true;
  }

  virtual void handle_focus_loose() {
    TextInput::handle_focus_loose();
    if ((text.length() < 16) && (saved_text.length() == 16)) {
      text = saved_text;
      saved_text.clear();
    }
  }
};

class PlayerView : public Control {
 protected:
  unsigned int index;
  std::function<PPlayerInfo(unsigned int)> delegte;

 public:
  PlayerView(unsigned int _index,
             std::function<PPlayerInfo(unsigned int)> _delegte)
    : Control(80, 80) {
    index = _index;
    delegte = _delegte;
  }

 public:
  virtual void draw(Frame *frame, unsigned int x, unsigned int y) {
    frame->draw_sprite(x, y, Data::AssetIcon, 251);
    frame->draw_sprite(x, y + 72, Data::AssetIcon, 252);
    frame->draw_sprite(x, y + 8, Data::AssetIcon, 253);
    frame->draw_sprite(x + 40, y + 8, Data::AssetIcon, 254);
    frame->draw_sprite(x + 72, y + 8, Data::AssetIcon, 255);

    frame->draw_sprite(x + 48, y + 8, Data::AssetIcon, 282);

    unsigned int sprite = 281;
    if (delegte != nullptr) {
      PPlayerInfo player_info = delegte(index);
      if (player_info) {
        sprite = player_info->get_face() + 267;

        unsigned int supplies = player_info->get_supplies();
        frame->fill_rect(x + 52, y + 68 - supplies, 4, supplies,
                         Color(0x00, 0x93, 0x87));

        unsigned int intelligence = player_info->get_intelligence();
        frame->fill_rect(x + 58, y + 68 - intelligence, 4, intelligence,
                         Color(0x6b, 0xab, 0x3b));

        unsigned int reproduction = player_info->get_reproduction();
        frame->fill_rect(x + 64, y + 68 - reproduction, 4, reproduction,
                         Color(0xa7, 0x27, 0x27));
      }
    }
    frame->draw_sprite(x + 8, y + 8, Data::AssetIcon, sprite);
  }

  virtual bool handle_click_left(int x, int y) {
    if (x < 8 || x > 8 + 64 || y < 8 || y > 76) {
      return true;
    }

    PPlayerInfo player_info = nullptr;
    if (delegte != nullptr) {
      player_info = delegte(index);
      if (!player_info) {
        return true;
      }
    }

    if (x >= 8 && x < 8+32 && y >= 8 && y < 72) {
      // Face
      bool in_use = false;
      do {
        unsigned int next = (player_info->get_face() + 1) % 14;
        next = std::max(1u, next);
        player_info->set_character(next);
/*
        // Check that face is not already in use by another player
        in_use = 0;
        for (size_t i = 0; i < mission->get_player_count(); i++) {
          if (index != i &&
              mission->get_player(i)->get_face() == next) {
            in_use = true;
            break;
          }
        }
 */
      } while (in_use);
    } else {
      x -= 8 + 32 + 8 + 3;
      if (x < 0) {
        return false;
      }
      if (y >= 27 && y < 69) {
        unsigned int value = clamp(0, 68 - y, 40);
        if (x > 0 && x < 6) {
          // Supplies
          player_info->set_supplies(value);
        } else if (x > 6 && x < 12) {
          // Intelligence
          player_info->set_intelligence(value);
        } else if (x > 12 && x < 18) {
          // Reproduction
          player_info->set_reproduction(value);
        }
      }
    }

    return true;
  }
};

GameInitBox::GameInitBox(Interface *_interface) {
  interface = _interface;
  game_type = GameNone;

  game_mission = 0;

  set_size(360, 254);

  custom_mission = std::make_shared<GameInfo>(Random());
  custom_mission->remove_all_players();
  custom_mission->add_player(12, {0x00, 0xe3, 0xe3}, 40, 40, 40);
  custom_mission->add_player(1, {0xcf, 0x63, 0x63}, 20, 30, 40);
  mission = custom_mission;
}

GameInitBox::~GameInitBox() {
}

void
GameInitBox::init() {
  minimap = std::make_shared<Minimap>(150, 160, nullptr);
  minimap->set_draw_grid(true);
  generate_map_preview();

  add_layout(create_layout_custom());
  add_layout(create_layout_mission());
  add_layout(create_layout_load());
}

PLayout
GameInitBox::create_layout_custom() {
  layout_custom = std::make_shared<Layout>(320, 222);
  layout_custom->set_indents(20, 16);
  // Start button
  layout_custom->make_item<Button>(0, 0, 32, 32, 266, [this](int x, int y) {
    Game *game = new Game();
    if (!game->load_mission_map(this->mission)) {
      return;
    }

    Game *old_game = this->interface->get_game();
    if (old_game != nullptr) {
      EventLoop::get_instance()->del_handler(old_game);
    }

    EventLoop::get_instance()->add_handler(game);
    this->interface->set_game(game);
    if (old_game != nullptr) {
      delete old_game;
    }
    this->interface->set_player(0);
    this->interface->close_game_init();
  });

  // Game type button
  layout_custom->make_item<Button>(40, 0, 32, 32, 263, [this](int x, int y) {
    this->mission = GameInfo::get_mission(this->game_mission);
    this->generate_map_preview();
    this->next_layout();
  });

  layout_custom->make_item<Label>(80, 4, 112, 9, "New game");

  layout_custom->make_item<Label>(80, 18, 112, 9, [this]()->std::string {
    std::stringstream str_map_size;
    str_map_size << this->mission->get_map_size();
    return "Mapsize: " + str_map_size.str();
  });

  std::shared_ptr<RandomInput> rnd_field = std::make_shared<RandomInput>();

  // Map size button
  layout_custom->make_item<Button>(200, 0, 40, 32, 265,
                                   [this, rnd_field](int x, int y) {
    if (y < 9) {
      if (x < 9) {
        this->custom_mission->set_map_size(std::max(3u,
                                     this->custom_mission->get_map_size() - 1));
        this->generate_map_preview();
      } else if (x > 24) {
        rnd_field->set_random(Random());
      }
    } else {
      if (x > 24) {
        std::string str = rnd_field->get_text();
        if (str.length() == 16) {
          this->custom_mission->set_random_base(rnd_field->get_random());
          this->generate_map_preview();
        }
      } else {
        this->custom_mission->set_map_size(std::max(3u,
                                     this->custom_mission->get_map_size() + 1));
        this->generate_map_preview();
      }
    }
  });

  rnd_field->set_random(custom_mission->get_random_base());
  layout_custom->add_item(248, 0, rnd_field);

  // Options button
  layout_custom->make_item<Button>(288, 0, 32, 32, 267, [this](int x, int y) {
  });

  unsigned int x = 0;
  unsigned int y = 40;
  for (int i = 0; i < 4; i++) {
    if (i == 2) {
      x = 0;
      y += 80;
    }
    // Player #i
    layout_custom->make_item<PlayerView>(x, y, i,
                                       [this](unsigned int index)->PPlayerInfo {
      if (index >= this->mission->get_player_count()) {
        return nullptr;
      }
      return this->mission->get_player(index);
    });
    x += 80;
  }

  layout_custom->add_item(170, 40, minimap);

  layout_custom->make_item<Label>(0, 212, 296, 9, FREESERF_VERSION);

  // Exit button
  layout_custom->make_item<Button>(304, 208, 16, 16, 60, [this](int x, int y) {
    this->interface->close_game_init();
  });

  return layout_custom;
}

PLayout
GameInitBox::create_layout_mission() {
  layout_mission = std::make_shared<Layout>(320, 222);
  layout_mission->set_indents(20, 16);
  // Start button
  layout_mission->make_item<Button>(0, 0, 32, 32, 266, [this](int x, int y) {
    Game *game = new Game();
    if (!game->load_mission_map(this->mission)) {
      return;
    }

    Game *old_game = this->interface->get_game();
    if (old_game != nullptr) {
      EventLoop::get_instance()->del_handler(old_game);
    }

    EventLoop::get_instance()->add_handler(game);
    this->interface->set_game(game);
    if (old_game != nullptr) {
      delete old_game;
    }
    this->interface->set_player(0);
    this->interface->close_game_init();
  });

  // Game type button
  layout_mission->make_item<Button>(40, 0, 32, 32, 260, [this](int x, int y) {
    this->next_layout();
  });

  layout_mission->make_item<Label>(80, 4, 112, 9, "Start mission");

  layout_mission->make_item<Label>(80, 18, 112, 9, [this]()->std::string {
    std::stringstream str_map_size;
    str_map_size << (this->game_mission + 1);
    return "Mission: " + str_map_size.str();
  });

  // Up button
  layout_mission->make_item<Button>(264, 0, 16, 16, 237, [this](int x, int y) {
    this->game_mission = std::min(this->game_mission+1,
                             static_cast<int>(GameInfo::get_mission_count())-1);
    this->mission = GameInfo::get_mission(this->game_mission);
    this->generate_map_preview();
  });

  // Down button
  layout_mission->make_item<Button>(264, 16, 16, 16, 240, [this](int x, int y) {
    this->game_mission = std::max(0, this->game_mission-1);
    this->mission = GameInfo::get_mission(this->game_mission);
    this->generate_map_preview();
  });

  // Options button
  layout_mission->make_item<Button>(288, 0, 32, 32, 267, [this](int x, int y) {
  });

  unsigned int x = 0;
  unsigned int y = 40;
  for (int i = 0; i < 4; i++) {
    if (i == 2) {
      x = 0;
      y += 80;
    }
    // Player #i
    layout_mission->make_item<PlayerView>(x, y, i,
                                       [this](unsigned int index)->PPlayerInfo {
      if (index >= this->mission->get_player_count()) {
        return nullptr;
      }
      return this->mission->get_player(index);
    });
    x += 80;
  }

  layout_mission->add_item(170, 40, minimap);

  layout_mission->make_item<Label>(0, 212, 296, 9, FREESERF_VERSION);

  // Exit button
  layout_mission->make_item<Button>(304, 208, 16, 16, 60, [this](int x, int y) {
    this->interface->close_game_init();
  });

  return layout_mission;
}

PLayout
GameInitBox::create_layout_load() {
  layout_load = std::make_shared<Layout>(320, 222);
  layout_load->set_indents(20, 16);

  // Start button
  layout_load->make_item<Button>(0, 0, 32, 32, 266, [this](int x, int y) {
    Game *game = new Game();
    if (!game->load_mission_map(this->mission)) {
      return;
    }

    Game *old_game = this->interface->get_game();
    if (old_game != nullptr) {
      EventLoop::get_instance()->del_handler(old_game);
    }

    EventLoop::get_instance()->add_handler(game);
    this->interface->set_game(game);
    if (old_game != nullptr) {
      delete old_game;
    }
    this->interface->set_player(0);
    this->interface->close_game_init();
  });

  // Game type button
  layout_load->make_item<Button>(40, 0, 32, 32, 316, [this](int x, int y) {
    this->mission = this->custom_mission;
    this->generate_map_preview();
    this->next_layout();
  });

  layout_load->make_item<Label>(80, 4, 112, 9, "Load game");

  layout_load->make_item<Label>(80, 18, 112, 9, "");

  std::shared_ptr<ListSavedFiles> list =
                                     std::make_shared<ListSavedFiles>(160, 160);
  layout_load->add_item(0, 40, list);
  list->set_selection_handler([this](const std::string &item) {
    Game game;
    if (GameStore::get_instance()->load(item, &game)) {
      player_index = mission->get_player_count() - 1;
    }
  });

  // Options button
  layout_load->make_item<Button>(288, 0, 32, 32, 267, [this](int x, int y) {
  });

  layout_load->make_item<Label>(0, 212, 296, 9, FREESERF_VERSION);

  // Exit button
  layout_load->make_item<Button>(304, 208, 16, 16, 60, [this](int x, int y) {
    this->interface->close_game_init();
  });

  return layout_load;
}

void
GameInitBox::draw_background() {
  unsigned int icon = 290;
  for (int y = 9; y < (height - 7); y += 8) {
    for (int x = 8; x < (width - 8); x += 40) {
      frame->draw_sprite(x, y, Data::AssetIcon, icon);
    }
    icon--;
    if (icon < 290) {
      icon = 294;
    }
  }
}

void
GameInitBox::generate_map_preview() {
  PMap map = std::make_shared<Map>(MapGeometry(mission->get_map_size()));

  if (game_type == GameMission) {
    ClassicMissionMapGenerator generator(*map, mission->get_random_base());
    generator.init();
    generator.generate();
    map->init_tiles(generator);
  } else {
    ClassicMapGenerator generator(*map, mission->get_random_base());
    generator.init(MapGenerator::HeightGeneratorMidpoints, true);
    generator.generate();
    map->init_tiles(generator);
  }

  minimap->set_map(map);
}
