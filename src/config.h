/*
 * config.h - Config file declaration
 *
 * Copyright (C) 2020  Wicked_Digger <wicked_digger@mail.ru>
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
#include <map>

template <typename T> class Property {
 protected:
  std::string name;
  T value;

 public:
  Property(std::string name, T value): name(name), value(value) {}
  virtual ~Property() {}
  virtual T& operator= (const T& f) { return value = f; }
  virtual const T& operator() () const { return value; }
  virtual explicit operator const T& () const { return value; }
  virtual T* operator->() { return &value; }
};

class Config {
 public:
  template <typename T, T def> class Parameter {
   protected:
    T value = def;

   public:
     T get_value() const { return value; }
     operator T () const { return value; }
     operator std::string () const {
       std::stringstream str;
       str << value;
       return str.str();
     }
  };

  class Section {
   protected:
    std::map<std::string, Section> sections;
    
   public:
    Section() {
    }

    virtual Section& operator[](const std::string &name) {
      return sections[name];
    }
  };

 protected:
  std::map<std::string, Section> sections;

 public:
  Config();
  virtual ~Config();

  virtual Section& operator[](const std::string &name) {
    return sections[name];
  }
};

typedef std::shared_ptr<Config> PConfig;

#endif  // SRC_CONFIG_H_
