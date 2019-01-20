/*
 * resource.h - Resources related definitions.
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

#ifndef SRC_RESOURCE_H_
#define SRC_RESOURCE_H_

#include <map>

class Resource {
 public:
  typedef enum Type {
    TypeNone = -1,

    TypeFish = 0,
    TypePig,
    TypeMeat,
    TypeWheat,
    TypeFlour,
    TypeBread,
    TypeLumber,
    TypePlank,
    TypeBoat,
    TypeStone,
    TypeIronOre,
    TypeSteel,
    TypeCoal,
    TypeGoldOre,
    TypeGoldBar,
    TypeShovel,
    TypeHammer,
    TypeRod,
    TypeCleaver,
    TypeScythe,
    TypeAxe,
    TypeSaw,
    TypePick,
    TypePincer,
    TypeSword,
    TypeShield,

    GroupFood
  } Type;
};

typedef std::map<Resource::Type, unsigned int> ResourceMap;

class Package {
 protected:
  Resource::Type resource;
  unsigned int dest;

 public:
  Package() : resource(Resource::TypeNone), dest(0) {}
  Package(Resource::Type _resource, unsigned int _dest)
  : resource(_resource), dest(_dest) {}

  Resource::Type get_resource() const { return resource; }
  unsigned int get_dest() const { return dest; }

  bool is_empty() const { return (resource == Resource::TypeNone); }
  bool is_lost() const { return (dest == 0); }
  bool is_routable() const;  // Can be reroute?

  void lost() { dest = 0; }
};

#endif  // SRC_RESOURCE_H_
