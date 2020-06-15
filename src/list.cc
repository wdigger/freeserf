/*
 * list.cc - String list GUI component
 *
 * Copyright (C) 2017-2018  Wicked_Digger <wicked_digger@mail.ru>
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

#include "src/list.h"

ListSavedFiles::ListSavedFiles(unsigned int _width, unsigned int _height)
  : Control(_width, _height)
  , save_game(GameStore::get_instance()) {
  color_focus = Color(0x00, 0x8b, 0x47);
  color_text = Color::green;
  color_background = Color::black;

  items = save_game.get_saved_games();
  first_visible_item = 0;
  selected_item = -1;
}

std::string
ListSavedFiles::get_selected() const {
  if (items.size() == 0) {
    return std::string();
  }
  std::string file_path = items[selected_item].path;
  return file_path;
}

void
ListSavedFiles::draw(Frame *frame, unsigned int _x, unsigned int _y) {
  frame->fill_rect(_x, _y, width, height, color_background);

  if (is_focused()) {
    frame->draw_rect(_x, _y, width, height, color_focus);
  }

  unsigned int item = first_visible_item;
  for (unsigned int y = 0;
       (y < (height - 6)) && (item < items.size()); y += 9) {
    Color tc = color_text;
    if (static_cast<int>(item) == selected_item) {
      frame->fill_rect(_x + 2, _y + y + 2, width - 4, 9, Color::green);
      tc = Color::black;
    }
    frame->draw_string(_x + 3, _y + 3 + y, items[item].name, tc);
    item++;
  }
}

bool
ListSavedFiles::handle_click_left(int x, int y) {
  own_focus();
  y -= 3;
  if (y >= 0) {
    y = first_visible_item + (y / 9);
    if ((selected_item != y) && (y >= 0) &&
        (y < static_cast<int>(items.size()))) {
      selected_item = y;
      if (selection_handler != nullptr) {
        std::string file_path = items[selected_item].path;
        selection_handler(file_path);
      }
    }
  }
  return true;
}

bool
ListSavedFiles::handle_drag(int dx, int dy) {
  if (!is_focused()) {
    return false;
  }

  int nfvi = static_cast<int>(first_visible_item) + (dy / 32);
  if (nfvi < 0) {
    nfvi = 0;
  }
  if ((((items.size()) - nfvi) * 9) <= (height - 8)) {
    return true;
  }
  if (static_cast<int>(first_visible_item) != nfvi) {
    first_visible_item = nfvi;
    invalidate();
  }
  return true;
}

bool
ListSavedFiles::handle_key_pressed(char key, int modifier) {
  return true;
}

void
ListSavedFiles::handle_focus_loose() {
  invalidate();
}
