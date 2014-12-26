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
  text = NULL;
  input_enabled = false;
}

void
text_input_t::set_text(char *text)
{
  if (this->text != NULL) {
    free(this->text);
    this->text = NULL;
  }

  this->text = strdup(text);
}

char *
text_input_t::get_text()
{
  return strdup(text);
}

void
text_input_t::internal_draw()
{
  gfx_draw_string(0, 0, 31, 0, frame, text);
}

int
text_input_t::handle_click_left(int x, int y)
{
  input_enabled = true;
  return 1;
}
