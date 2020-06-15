/*
 * dialog-building.h - Building dialog
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

#ifndef SRC_DIALOG_BUILDING_H_
#define SRC_DIALOG_BUILDING_H_

#include "src/dialog.h"

class Interface;
class Building;
class Inventory;

class DialogBuilding : public Dialog {
 protected:
  Interface *interface;
  Building *building;

 public:
  explicit DialogBuilding(Interface *interface, Building *building);
  virtual ~DialogBuilding();

 protected:
  PLayout create_layout_mine();
  void create_layout_inventory();
  PLayout create_layout_ordered();
  PLayout create_layout_military();
  PLayout create_layout_resources(Inventory *inventory);
  PLayout create_layout_serfs(Inventory *inventory);
  PLayout create_layout_control(Inventory *inventory);
  void add_flip_button(PLayout layout);
  void add_exit_button(PLayout layout);

  virtual void init();
};

#endif  // SRC_DIALOG_BUILDING_H_
