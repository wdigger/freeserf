/*
 * dialog.h - Dialog and controls GUI components header
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

#ifndef SRC_DIALOG_H_
#define SRC_DIALOG_H_

#include <memory>
#include <string>
#include <list>
#include <functional>
#include <utility>

#include "src/gui.h"

class Dialog;

class Control : public std::enable_shared_from_this<Control> {
 protected:
  unsigned int x;
  unsigned int y;
  unsigned int width;
  unsigned int height;
  std::weak_ptr<Dialog> parent;

 public:
  Control(unsigned int _width, unsigned int _height);
  virtual ~Control() {}

  unsigned int get_width() { return width; }
  unsigned int get_height() { return height; }

  void set_parent(std::weak_ptr<Dialog> _parent) { parent = _parent; }
  void own_focus();
  bool is_focused();

  void invalidate();

  virtual void draw(Frame *frame, unsigned int x, unsigned int y) = 0;
  virtual bool handle_click_left(int x, int y) { return false; }
  virtual bool handle_drag(int dx, int dy) { return false; }
  virtual bool handle_key_pressed(char key, int modifier) { return false; }
  virtual void handle_focus_loose() {}
};

typedef std::shared_ptr<Control> PControl;

class Button : public Control {
 protected:
  unsigned int sprite;
  std::function<void(int x, int y)> handler;
  std::function<unsigned int()> delegate;

 public:
  Button(unsigned int _width, unsigned int _height,
         unsigned int _sprite, std::function<void(int x, int y)> _handler);

  void set_delegate(std::function<unsigned int()> _delegate) {
    delegate = _delegate;
  }

 public:
  virtual void draw(Frame *frame, unsigned int x, unsigned int y);
  virtual bool handle_click_left(int x, int y);
};

typedef std::shared_ptr<Button> PButton;

class ButtonText : public Control {
 protected:
  std::string text;
  std::function<void(int x, int y)> handler;
  std::function<std::string()> delegate;

 public:
  ButtonText(unsigned int _width, unsigned int _height,
             const std::string &_text,
             std::function<void(int x, int y)> _handler);

  void set_delegate(std::function<std::string()> _delegate) {
    delegate = _delegate;
  }

 public:
  virtual void draw(Frame *frame, unsigned int x, unsigned int y);
  virtual bool handle_click_left(int x, int y);
};

typedef std::shared_ptr<ButtonText> PButtonText;

class Label : public Control {
 protected:
  std::string text;
  Color text_color;
  std::function<std::string()> delegate;
  unsigned int sprite;

 public:
  Label(unsigned int _width, unsigned int _height, const std::string &_text);
  Label(unsigned int _width, unsigned int _height, unsigned int sprite);
  Label(unsigned int _width, unsigned int _height,
        std::function<std::string()> _delegate);

  void set_text(const std::string &_text);
  void set_icon(unsigned int sprite);
  void set_color(const Color &color);

 public:
  virtual void draw(Frame *frame, unsigned int x, unsigned int y);
};

typedef std::shared_ptr<Label> PLabel;

class Layout {
 protected:
  typedef struct Item {
    unsigned int x;
    unsigned int y;
    PControl control;
  } Item;

  unsigned int width;
  unsigned int height;

  std::list<Item> items;

  int indent_left;
  int indent_top;

  unsigned int background;

  std::weak_ptr<Dialog> parent;

 public:
  Layout(unsigned int _width, unsigned int _height);

  void add_item(unsigned int x, unsigned int y, PControl control);
  template< class T, class... Args >
    std::shared_ptr<T> make_item(unsigned int x, unsigned int y,
                                 Args&&... args) {
    std::shared_ptr<T> control =
                               std::make_shared<T>(std::forward<Args>(args)...);
    add_item(x, y, control);
    return control;
  }
  void set_indents(int left, int top);

  void set_parent(std::shared_ptr<Dialog> _parent);

  void set_background(unsigned int sprite);

  void draw(Frame *frame, unsigned int x, unsigned int y);

  bool handle_click_left(int x, int y);
  bool handle_drag(int dx, int dy);

 private:
  void draw_items(Frame *frame, unsigned int x, unsigned int y);
};

typedef std::shared_ptr<Layout> PLayout;

class Dialog : public GuiObject {
 protected:
  std::list<PLayout> layouts;
  PLayout current_layout;
  unsigned int background_sprite;
  PControl focused;

 public:
  Dialog();

  void add_layout(PLayout layout);
  void next_layout();
  void set_background(unsigned int sprite);

  void set_focused(PControl control);
  bool is_focused(PControl control) { return focused == control; }

  virtual void set_parent(std::shared_ptr<GuiObject> _parent);

 protected:
  virtual void internal_draw();
  virtual bool handle_click_left(int x, int y);
  virtual bool handle_drag(int dx, int dy);
  virtual bool handle_key_pressed(char key, int modifier);

  virtual void draw_background();
};

typedef std::shared_ptr<Dialog> PDialog;

#endif  // SRC_DIALOG_H_
