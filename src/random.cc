/*
 * random.cc - Random number generator
 *
 * Copyright (C) 2013  Jon Lund Steffensen <jonlst@gmail.com>
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

#include "src/random.h"

#include <ctime>
#include <sstream>

Random::Random() {
  srand((unsigned int)time(NULL));
  RandomProto::set_state_0(std::rand());
  RandomProto::set_state_1(std::rand());
  RandomProto::set_state_2(std::rand());
  random();
}

Random::Random(const uint16_t &value) {
  RandomProto::set_state_0(value);
  RandomProto::set_state_1(value);
  RandomProto::set_state_2(value);
}

Random::Random(const Random &random_state) {
  RandomProto::set_state_0(random_state.state_0());
  RandomProto::set_state_1(random_state.state_1());
  RandomProto::set_state_2(random_state.state_2());
}

Random::Random(const std::string &string) {
  uint64_t tmp = 0;

  for (int i = 15; i >= 0; i--) {
    tmp <<= 3;
    uint8_t c = string[i] - '0' - 1;
    tmp |= c;
  }

  RandomProto::set_state_0(tmp & 0xFFFF);
  tmp >>= 16;
  RandomProto::set_state_1(tmp & 0xFFFF);
  tmp >>= 16;
  RandomProto::set_state_2(tmp & 0xFFFF);
}

Random::Random(uint16_t base_0, uint16_t base_1, uint16_t base_2) {
  RandomProto::set_state_0(base_0);
  RandomProto::set_state_1(base_1);
  RandomProto::set_state_2(base_2);
}

uint16_t
Random::random() {
  uint16_t r = (state_0() + state_1()) ^ state_2();
  RandomProto::set_state_2(state_2() + state_1());
  RandomProto::set_state_1(state_1() ^ state_2());
  RandomProto::set_state_1((state_1() >> 1) | (state_1() << 15));
  RandomProto::set_state_2((state_2() >> 1) | (state_2() << 15));
  RandomProto::set_state_0(r);

  return r;
}

Random::operator std::string() const {
  uint64_t tmp0 = state_0();
  uint64_t tmp1 = state_1();
  uint64_t tmp2 = state_2();

  std::stringstream ss;

  uint64_t tmp = tmp0;
  tmp |= tmp1 << 16;
  tmp |= tmp2 << 32;

  for (int i = 0; i < 16; i++) {
    uint8_t c = tmp & 0x07;
    c += '1';
    ss << c;
    tmp >>= 3;
  }

  return ss.str();
}

Random&
operator^=(Random& left, const Random& right) {  // NOLINT
  left.set_state_0(left.state_0() ^ right.state_0());
  left.set_state_1(left.state_1() ^ right.state_1());
  left.set_state_2(left.state_2() ^ right.state_2());

  return left;
}
