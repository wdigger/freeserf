/*
 * dialog-quit-confirm.cc - Quit confirmation dialog
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

#include "src/dialog-quit-confirm.h"

#include <sstream>
#include <memory>

#include "src/interface.h"
#include "src/audio.h"

DialogQuitConfirm::DialogQuitConfirm(Interface *_interface) {
  interface = _interface;
  set_size(144, 160);
  set_background(310);
}

DialogQuitConfirm::~DialogQuitConfirm() {
}

void
DialogQuitConfirm::init() {
  add_layout(create_layout_first());
  add_layout(create_layout_second());
}

PLayout
DialogQuitConfirm::create_layout_first() {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(24, 8);

  layout->add_item(4, 10, std::make_shared<Label>(88, 10, "Do you want"));
  layout->add_item(20, 20, std::make_shared<Label>(56, 10, "to quit"));
  layout->add_item(8, 30, std::make_shared<Label>(80, 10, "this game?"));

  layout->add_item(0, 50, std::make_shared<ButtonText>(24, 10, "Yes",
                                                       [this](int x, int y){
    play_sound(Audio::TypeSfxAhhh);
    EventLoop::get_instance().quit();
  }));
  layout->add_item(80, 50, std::make_shared<ButtonText>(16, 10, "No",
                                                        [this](int x, int y){
    this->close();
  }));

  return layout;
}

PLayout
DialogQuitConfirm::create_layout_second() {
  PLayout layout = std::make_shared<Layout>(144, 144);
  layout->set_indents(8, 8);

  layout->add_item(20, 10, std::make_shared<Label>(88, 10, "Do you want"));
  layout->add_item(36, 20, std::make_shared<Label>(56, 10, "to quit"));
  layout->add_item(24, 30, std::make_shared<Label>(80, 10, "this game?"));

  layout->add_item(0, 50, std::make_shared<Label>(128, 10, "The game has not"));
  layout->add_item(24, 60, std::make_shared<Label>(80, 10, "been saved"));
  layout->add_item(28, 70, std::make_shared<Label>(72, 10, "recently."));
  layout->add_item(36, 80, std::make_shared<Label>(56, 10, "Are you"));
  layout->add_item(44, 90, std::make_shared<Label>(40, 10, "sure?"));

  layout->add_item(16, 110, std::make_shared<ButtonText>(24, 10, "Yes",
                                                         [this](int x, int y){
    play_sound(Audio::TypeSfxAhhh);
    EventLoop::get_instance().quit();
  }));
  layout->add_item(96, 110, std::make_shared<ButtonText>(16, 10, "No",
                                                         [this](int x, int y){
    this->close();
  }));

  return layout;
}

