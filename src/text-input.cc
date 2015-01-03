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
  input_enabled = false;
  max_length = 0;
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
  gfx_fill_rect(0, 0, width, height, 1, frame);
  int ch_width = width/8;
  std::string str = text;
  int cx = 0;
  int cy = 0;
  while (str.length()) {
    std::string substr = str.substr(0, ch_width);
    str.erase(0, ch_width);
    gfx_draw_string(cx, cy, 31, 0, frame, substr.c_str());
    cy += 8;
  }
}

int
text_input_t::handle_click_left(int x, int y)
{
  input_enabled = true;
  text.clear();
  set_redraw();
  return 1;
}

int
text_input_t::handle_key_pressed(char key, int modifier)
{
  if (!input_enabled) {
    return 0;
  }

  if ((max_length != 0) && (text.length() >= max_length)) {
    return 1;
  }

  if (key < '1' || key > '8') {
    return 0;
  }

  if (text.length() > 16) {
    return 0;
  }

  text += key;

  set_redraw();

  return 1;
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

