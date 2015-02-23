//
//  text-input.cc
//  freeserf
//
//  Created by Yury Klushin on 25.12.14.
//  Copyright (c) 2014 freeserf. All rights reserved.
//

#include "text-input.h"

#include <cstdlib>

text_input_t::text_input_t()
  : gui_object_t()
{
  max_length = 0;
  filter = NULL;
  draw_focus = true;
  color_focus = 0;
  color_text = 31;
  color_background = 1;
}

void
text_input_t::set_text(const char *text)
{
  this->text = text;
  set_redraw();
}

const char *
text_input_t::get_text()
{
  return text.c_str();
}

void
text_input_t::internal_draw()
{
  frame->fill_rect(0, 0, width, height, color_background);
  if (draw_focus && focused) {
    frame->draw_rect(0, 0, width, height, color_focus);
  }
  int ch_width = width/8;
  std::string str = text;
  int cx = 0;
  int cy = 0;
  if (draw_focus) {
    cx = 1;
    cy = 1;
  }
  while (str.length()) {
    std::string substr = str.substr(0, ch_width);
    str.erase(0, ch_width);
    frame->draw_string(cx, cy, color_text, 0, substr.c_str());
    cy += 8;
  }
}

bool
text_input_t::handle_click_left(int x, int y)
{
  focused_object = this;
  focused = true;
  set_redraw();
  return true;
}

bool
text_input_t::handle_key_pressed(char key, int modifier)
{
  if (!focused) {
    return false;
  }

  if ((max_length != 0) && (text.length() >= max_length)) {
    return true;
  }

  if ((key == '\b') && (text.length() > 0)) {
    text = text.substr(0, text.length() - 1);
    set_redraw();
    return true;
  }

  if (filter != NULL) {
    if (!filter(key, this)) {
      return true;
    }
  }

  text += key;

  set_redraw();

  return true;
}

bool
text_input_t::handle_focus_loose()
{
  focused = false;
  set_redraw();
  return true;
}

void
text_input_t::set_max_length(unsigned int max_length)
{
  this->max_length = max_length;
  if (max_length != 0) {
    if (text.length() > max_length) {
      text = text.substr(0, max_length);
    }
  }
}

