/*
 * config.h - Memory buffer declaration
 *
 * Copyright (C) 2019  Wicked_Digger <wicked_digger@mail.ru>
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

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <memory>
#include <string>
#include <sstream>

class Config {
 protected:
  template <typename T, T def> class Parameter {
   protected:
    T value = def;
    std::string name;

   public:
     std::string get_name() const { return name; }
     T get_value() const { return value; }
     operator T () const { return value; }
     operator std::string () const {
       std::stringstream str;
       str << value;
       return str.str();
     }
  };

 public:
  Config();
  virtual ~Config();
};

typedef std::shared_ptr<Config> PConfig;

#endif  // SRC_CONFIG_H_
