/*
 * dialog.cc - Dialog and controls GUI components
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

#include "src/dialog.h"

#include <string>
#include <limits>

// Control

Control::Control(unsigned int _width, unsigned int _height) {
  width = _width;
  height = _height;
}

void
Control::own_focus() {
  if (parent.lock()) {
    parent.lock()->set_focused(shared_from_this());
    invalidate();
  }
}

bool
Control::is_focused() {
  if (parent.lock()) {
    return parent.lock()->is_focused(shared_from_this());
  }
  return false;
}

void
Control::invalidate() {
  if (parent.lock()) {
    parent.lock()->set_redraw();
  }
}

// Button

Button::Button(unsigned int _width, unsigned int _height,
               unsigned int _sprite, std::function<void(int x, int y)> _handler)
  : Control(_width, _height) {
  sprite = _sprite;
  handler = _handler;
  delegate = nullptr;
}

void
Button::draw(Frame *frame, unsigned int x, unsigned int y) {
  if (delegate) {
    sprite = delegate();
  }

  frame->draw_sprite(x, y, Data::AssetIcon, sprite);
}

bool
Button::handle_click_left(int x, int y) {
  own_focus();
  handler(x, y);
  return true;
}

// ButtonText

ButtonText::ButtonText(unsigned int _width, unsigned int _height,
                       const std::string &_text,
                       std::function<void(int x, int y)> _handler)
: Control(_width, _height) {
  text = _text;
  handler = _handler;
  delegate = nullptr;
}

void
ButtonText::draw(Frame *frame, unsigned int x, unsigned int y) {
  if (delegate) {
    text = delegate();
  }

  frame->draw_string(x, y, text, Color::green, Color::black);
}

bool
ButtonText::handle_click_left(int x, int y) {
  own_focus();
  handler(x, y);
  return true;
}

// Label

Label::Label(unsigned int _width, unsigned int _height,
             const std::string &_text)
  : Control(_width, _height) {
  text = _text;
  text_color = Color::green;
  delegate = nullptr;
  sprite = std::numeric_limits<unsigned int>::max();
}

Label::Label(unsigned int _width, unsigned int _height, unsigned int _sprite)
  : Control(_width, _height) {
  text_color = Color::green;
  delegate = nullptr;
  sprite = _sprite;
}

Label::Label(unsigned int _width, unsigned int _height,
             std::function<std::string()> _delegate)
  : Control(_width, _height) {
  text = std::string();
  text_color = Color::green;
  delegate = _delegate;
  sprite = std::numeric_limits<unsigned int>::max();
}

void
Label::set_text(const std::string &_text) {
  text = _text;
  invalidate();
}

void
Label::set_icon(unsigned int _sprite) {
  sprite = _sprite;
  invalidate();
}

void
Label::set_color(const Color &color) {
  text_color = color;
  invalidate();
}

void
Label::draw(Frame *frame, unsigned int x, unsigned int y) {
  if (delegate != nullptr) {
    text = delegate();
  }
  int cx = x;
  int cy = y;
  if (sprite != std::numeric_limits<unsigned int>::max()) {
    frame->draw_sprite(cx, cy, Data::AssetIcon, sprite);
    cx += 16;
    cy += 4;
  }
  frame->draw_string(cx, cy, text, text_color, Color::black);
}

// Layout

Layout::Layout(unsigned int _width, unsigned int _height) {
  width = _width;
  height = _height;
  background = 0;
}

void
Layout::add_item(unsigned int x, unsigned int y, PControl control) {
  Item item;
  item.x = x;
  item.y = y;
  item.control = control;
  items.push_back(item);
  control->set_parent(parent);
}

void
Layout::set_indents(int left, int top) {
  indent_left = left;
  indent_top = top;
}

void
Layout::set_parent(std::shared_ptr<Dialog> _parent) {
  parent = _parent;

  for (Item &item : items) {
    item.control->set_parent(parent);
  }
}

void
Layout::set_background(unsigned int sprite) {
  background = sprite;

  if (parent.lock()) {
    parent.lock()->set_redraw();
  }
}

void
Layout::draw(Frame *frame, unsigned int x, unsigned int y) {
  if (background) {
    for (unsigned int y = 0; y < height; y += 16) {
      for (unsigned int x = 0; x < width; x += 16) {
        frame->draw_sprite(x, y, Data::AssetIcon, background);
      }
    }
  }
  draw_items(frame, x, y);
}

void
Layout::draw_items(Frame *frame, unsigned int x, unsigned int y) {
  for (Item &item : items) {
    item.control->draw(frame,
                       x + indent_left + item.x, y + indent_top + item.y);
    frame->draw_rect(x + indent_left + item.x, y + indent_top + item.y,
                     item.control->get_width(), item.control->get_height(),
                     Color(0xff, 0x00, 0x00));
  }
}

bool
Layout::handle_click_left(int x, int y) {
  for (Item &item : items) {
    if ((x >= static_cast<int>(indent_left + item.x)) &&
        (y >= static_cast<int>(indent_top + item.y)) &&
        (x <= static_cast<int>(indent_left + item.x +
                               item.control->get_width())) &&
        (y <= static_cast<int>(indent_top + item.y +
                               item.control->get_height()))) {
      return item.control->handle_click_left(x - indent_left - item.x,
                                             y - indent_top - item.y);
    }
  }
  return false;
}

bool
Layout::handle_drag(int dx, int dy) {
  for (Item &item : items) {
    if (item.control->handle_drag(dx, dy)) {
      return true;
    }
  }
  return false;
}

// Dialog

Dialog::Dialog() {
  background_sprite = std::numeric_limits<unsigned int>::max();
}

void
Dialog::add_layout(PLayout layout) {
  layouts.push_back(layout);
  if (!current_layout) {
    current_layout = layout;
    layout->set_parent(std::dynamic_pointer_cast<Dialog>(shared_from_this()));
  }
  set_redraw();
}

void
Dialog::next_layout() {
  if (layouts.size() <= 1) {
    return;
  }
  PLayout old_layout = current_layout;
  std::list<PLayout>::iterator it = std::find(layouts.begin(),
                                              layouts.end(),
                                              current_layout);
  it++;
  if (it == layouts.end()) {
    it = layouts.begin();
  }
  current_layout = *it;
  if (old_layout) {
    old_layout->set_parent(nullptr);
  }
  current_layout->set_parent(std::dynamic_pointer_cast<Dialog>(
                                                           shared_from_this()));
  focused = nullptr;
}

void
Dialog::set_background(unsigned int sprite) {
  background_sprite = sprite;
  set_redraw();
}

void
Dialog::set_focused(PControl control) {
  if (focused) {
    focused->handle_focus_loose();
  }
  focused = control;
  set_redraw();
}

void
Dialog::internal_draw() {
  draw_background();

  for (int x = 0; x < width; x += 144) {
    frame->draw_sprite(x, 0, Data::AssetFramePopup, 0);  // Top
  }
  for (int y = 9; y < height; y += 144) {
    frame->draw_sprite(0, y, Data::AssetFramePopup, 2);  // Left
    frame->draw_sprite(width - 8, y, Data::AssetFramePopup, 3);  // Right
  }
  for (int x = 0; x < width; x += 144) {
    frame->draw_sprite(x, height - 7, Data::AssetFramePopup, 1);  // Bottom
  }

  if (current_layout) {
    current_layout->draw(frame, 0, 0);
  }
}

bool
Dialog::handle_click_left(int x, int y) {
  set_redraw();
  bool res = current_layout->handle_click_left(x, y);
  if (!res) {
    set_focused(nullptr);
  }
  return res;
}

bool
Dialog::handle_drag(int dx, int dy) {
  bool res = current_layout->handle_drag(dx, dy);
  if (res) {
    set_redraw();
  }
  return res;
}

bool
Dialog::handle_key_pressed(char key, int modifier) {
  if (key == 27) {
    close();
  }

  if (focused) {
    focused->handle_key_pressed(key, modifier);
    return true;
  }

  return false;
}

void
Dialog::draw_background() {
  if (background_sprite == std::numeric_limits<unsigned int>::max()) {
    return;
  }

  for (int y = 0; y < (height - 16); y += 16) {
    for (int x = 0; x < (width - 16); x += 16) {
      frame->draw_sprite(8 + x, 9 + y, Data::AssetIcon, background_sprite);
    }
  }
}

void
Dialog::set_parent(std::shared_ptr<GuiObject> _parent) {
  GuiObject::set_parent(_parent);
  if (_parent) {
    int w = 0;
    int h = 0;
    _parent->get_size(&w, &h);
    move_to((w - width) / 2, (h - height) / 2);
  }
}
