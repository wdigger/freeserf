/*
 * dialog-demolish-confirm.cc - Demolish confirmation dialog
 *
 * Copyright (C) 2018  Wicked_Digger <wicked_digger@mail.ru>
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

#include "src/dialog-demolish-confirm.h"

#include <sstream>
#include <memory>

#include "src/interface.h"
#include "src/audio.h"

DialogDemolishConfirm::DialogDemolishConfirm(Interface *_interface) {
  interface = _interface;
  set_size(144, 160);
  set_background(314);
}

DialogDemolishConfirm::~DialogDemolishConfirm() {
}

void
DialogDemolishConfirm::init() {
  add_layout(create_layout_first());
}

PLayout
DialogDemolishConfirm::create_layout_first() {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(24, 8);

  layout->add_item(16, 10, std::make_shared<Label>(72, 10, "Demolish:"));
  layout->add_item(8, 30, std::make_shared<Label>(80, 10, "Click here"));
  layout->add_item(8, 68, std::make_shared<Label>(80, 10, "if you are"));
  layout->add_item(32, 86, std::make_shared<Label>(32, 10, "sure"));

  // Confirm checkbox
  layout->make_item<Button>(40, 45, 16, 16, 288,
                            [this](unsigned int x, unsigned int y) {
    interface->demolish_object();
    close();
  });
  // Exit button
  layout->make_item<Button>(96, 128, 16, 16, 0x3c,
                            [this](unsigned int x, unsigned int y) {
    close();
  });

  return layout;
}
