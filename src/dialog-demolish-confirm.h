/*
 * dialog-demolish-confirm.h - Demolish confirmation dialog
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

#ifndef SRC_DIALOG_DEMOLISH_CONFIRM_H_
#define SRC_DIALOG_DEMOLISH_CONFIRM_H_

#include "src/dialog.h"

class Interface;

class DialogDemolishConfirm : public Dialog {
 protected:
  Interface *interface;

 public:
  explicit DialogDemolishConfirm(Interface *interface);
  virtual ~DialogDemolishConfirm();

 protected:
  PLayout create_layout_first();

  virtual void init();
};

#endif  // SRC_DIALOG_DEMOLISH_CONFIRM_H_
