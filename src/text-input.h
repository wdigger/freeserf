/*
 * text-input.h - Viewport GUI component
 *
 * Copyright (C) 2014  Wicked_Digger <wicked_digger@mail.ru>
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

#ifndef _TEXT_INPUT_H
#define _TEXT_INPUT_H

#include "gui.h"

#include <string>

class text_input_t
  : public gui_object_t
{
protected:
  std::string text;
  bool input_enabled;
  unsigned int max_length;

public:
  text_input_t();

  void set_text(const char *text);
  const char *get_text();
  unsigned int get_max_length() { return max_length; }
  void set_max_length(unsigned int max_length);

protected:
  virtual void internal_draw();

  virtual int handle_click_left(int x, int y);
  virtual int handle_key_pressed(char key, int modifier);
};

#endif /* ! _TEXT_INPUT_H */
