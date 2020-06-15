/*
 * dialog-ground-analysis.cc - Building dialog
 *
 * Copyright (C) 2017  Wicked_Digger <wicked_digger@mail.ru>
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

#include "src/dialog-ground-analysis.h"

#include <memory>
#include <string>

#include "src/interface.h"
#include "src/game.h"

DialogGroundAnalysis::DialogGroundAnalysis(Interface *_interface) {
  interface = _interface;
  set_size(144, 160);
  set_background(129);
}

DialogGroundAnalysis::~DialogGroundAnalysis() {
}

// Translate resource amount to text.
const std::string
DialogGroundAnalysis::prepare_res_amount_text(unsigned int amount) const {
  if (amount == 0) return "Not Present";
  else if (amount < 100) return "Minimum";
  else if (amount < 180) return "Very Few";
  else if (amount < 240) return "Few";
  else if (amount < 300) return "Below Average";
  else if (amount < 400) return "Average";
  else if (amount < 500) return "Above Average";
  else if (amount < 600) return "Much";
  else if (amount < 800) return "Very Much";
  return "Perfect";
}

void
DialogGroundAnalysis::init() {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(8, 9);

  layout->make_item<Label>(56, 10, 16, 16, 28);
  layout->make_item<Label>(0, 30, 128, 9, "GROUND-ANALYSIS:");

  MapPos pos = interface->get_map_cursor_pos();
  int estimates[5];
  interface->get_game()->prepare_ground_analysis(pos, estimates);

  // Gold
  PLabel label = layout->make_item<Label>(4, 50, 120, 16, 0x2f);
  label->set_text(prepare_res_amount_text(2*estimates[Map::MineralsGold]));

  // Iron
  label = layout->make_item<Label>(4, 70, 120, 16, 0x2c);
  label->set_text(prepare_res_amount_text(2*estimates[Map::MineralsIron]));

  // Coal
  label = layout->make_item<Label>(4, 90, 120, 16, 0x2e);
  label->set_text(prepare_res_amount_text(2*estimates[Map::MineralsCoal]));

  // Stone
  label = layout->make_item<Label>(4, 110, 120, 16, 0x2b);
  label->set_text(prepare_res_amount_text(2*estimates[Map::MineralsStone]));

  // Exit button
  layout->make_item<Button>(112, 128, 16, 16, 0x3c,
                            [this](unsigned int x, unsigned int y) {
    close();
  });

  add_layout(layout);
}
