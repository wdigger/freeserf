/*
 * flag.h - Flag related functions.
 *
 * Copyright (C) 2013-2018  Jon Lund Steffensen <jonlst@gmail.com>
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

#ifndef SRC_FLAG_H_
#define SRC_FLAG_H_

#include <vector>

#include "src/building.h"
#include "src/objects.h"
#include "src/resource.h"

typedef struct SerfPathInfo {
  int path_len;
  int serf_count;
  int flag_index;
  Direction flag_dir;
  int serfs[16];
} SerfPathInfo;

class Building;
class Player;
class SaveReaderBinary;
class SaveReaderText;
class SaveWriterText;

class Flag : public GameObject {
 protected:
  // Max number of resources waiting at a flag
  static const unsigned int maxResCount = 8;

  class ResourceSlot {
   public:
    Package package;
    Direction dir;
  };

  class DirInfo {
   public:
    DirInfo() : lenght(0), free_transporters(0), serf_requested(false),
      has_path(false), water_path(false), flag(nullptr)
    {}
    unsigned int lenght;
    unsigned int free_transporters;
    bool serf_requested;
    bool has_path;
    bool water_path;
    Flag *flag;
  };

 protected:
  unsigned int owner;
  MapPos pos;
  ResourceSlot slot[maxResCount];
  int transporter;
  int other_end_dir[6];
  DirInfo dirs[6];
  bool inventory;
  bool accepts_serfs;
  bool accepts_resources;
  Building *building;
  bool has_resources;

  int search_num;
  Direction search_dir;

 public:
  Flag(Game *game, unsigned int index);

  MapPos get_position() const { return pos; }
  void set_position(MapPos pos) { this->pos = pos; }

  void add_path(Direction dir, bool water);
  void del_path(Direction dir);
  // Whether a path exists in a given direction
  bool has_path(Direction dir) const { return dirs[dir].has_path; }

  void prioritize_pickup(Direction dir, Player *player);

  /* Owner of this flag. */
  unsigned int get_owner() const { return owner; }
  void set_owner(unsigned int _owner) { owner = _owner; }

  // Bitmap showing whether the outgoing paths are land paths
  bool has_land_paths() const;
  // Whether the path in the given direction is a water path
  bool is_water_path(Direction dir) const { return dirs[dir].water_path; }
  // Whether a building is connected to this flag. If so, the pointer to
  // the other endpoint is a valid building pointer.
  // (Always at UP LEFT direction)
  bool has_building() const { return (building != nullptr); }

  // Whether resources exist that are not yet scheduled
  bool is_has_resources() const { return has_resources; }
  std::vector<Resource::Type> get_resources() const;

  /* Bitmap showing whether the outgoing paths have transporters
   servicing them. */
  int transporters() const { return transporter & 0x3f; }
  /* Whether the path in the given direction has a transporter
   serving it. */
  bool has_transporter(Direction dir) const {
    return ((transporter & (1 << (dir))) != 0); }
  /* Whether this flag has tried to request a transporter without success. */
  bool serf_request_fail() const { return (transporter >> 7) & 1; }
  void serf_request_clear() { transporter &= ~BIT(7); }

  // Current number of transporters on path
  unsigned int free_transporter_count(Direction dir) const {
    return dirs[dir].free_transporters;
  }
  void transporter_to_serve(Direction dir) { dirs[dir].free_transporters--; }
  // Length category of path determining max number of transporters
  unsigned int length_category(Direction dir) const { return dirs[dir].lenght; }
  // Whether a transporter serf was successfully requested for this path
  bool serf_requested(Direction dir) const { return dirs[dir].serf_requested; }
  void cancel_serf_request(Direction dir) { dirs[dir].serf_requested = false; }
  void complete_serf_request(Direction dir) {
    dirs[dir].serf_requested = false;
    dirs[dir].free_transporters++;
  }

  /* The slot that is scheduled for pickup by the given path. */
  unsigned int scheduled_slot(Direction dir) const {
    return other_end_dir[dir] & 7; }
  /* The direction from the other endpoint leading back to this flag. */
  Direction get_other_end_dir(Direction dir) const {
    return (Direction)((other_end_dir[dir] >> 3) & 7); }
  Flag *get_other_end_flag(Direction dir) const { return dirs[dir].flag; }
  /* Whether the given direction has a resource pickup scheduled. */
  bool is_scheduled(Direction dir) const {
    return (other_end_dir[dir] >> 7) & 1; }
  Package pick_up_resource(unsigned int slot);
  bool drop_resource(Package package);
  bool has_empty_slot() const;
  void remove_all_resources();
  Resource::Type get_resource_at_slot(int slot) const;

  // Whether this flag has an inventory building
  bool has_inventory() const { return inventory; }
  // Whether this inventory accepts resources
  bool is_accepts_resources() const { return accepts_resources; }
  // Whether this inventory accepts serfs
  bool is_accepts_serfs() const { return accepts_serfs; }

  void init_inventory() { inventory = true; accepts_resources = true; accepts_serfs = true; }
  void set_accepts_resources(bool accepts) { accepts_resources = accepts; }
  void set_accepts_serfs(bool accepts) { accepts_serfs = accepts; }
  void clear_flags() { accepts_serfs = false; inventory = false; accepts_resources = false; }

  friend SaveReaderBinary&
    operator >> (SaveReaderBinary &reader, Flag &flag);
  friend SaveReaderText&
    operator >> (SaveReaderText &reader, Flag &flag);
  friend SaveWriterText&
    operator << (SaveWriterText &writer, Flag &flag);

  bool schedule_known_dest_cb_(Flag *src, Flag *dest, int slot);

  void reset_transport(Flag *other);

  void reset_destination_of_stolen_resources();

  void link_building(Building *building);
  void unlink_building();
  Building *get_building() { return building; }

  void invalidate_resource_path(Direction dir);

  int find_nearest_inventory_for_resource();
  int find_nearest_inventory_for_serf();

  void link_with_flag(Flag *dest_flag, bool water_path, size_t length,
                      Direction in_dir, Direction out_dir);

  void update();

  /* Get road length category value for real length.
   Determines number of serfs servicing the path segment.(?) */
  static size_t get_road_length_value(size_t length);

  void restore_path_serf_info(Direction dir, SerfPathInfo *data);

  void set_search_dir(Direction dir) { search_dir = dir; }
  Direction get_search_dir() const { return search_dir; }
  void clear_search_id() { search_num = 0; }

  bool can_demolish() const;

  void merge_paths(MapPos pos);

  static void fill_path_serf_info(Game *game, MapPos pos, Direction dir,
                                  SerfPathInfo *data);

 protected:
  void fix_scheduled();

  void schedule_slot_to_unknown_dest(int slot);
  void schedule_slot_to_known_dest(int slot, unsigned int res_waiting[4]);
  bool call_transporter(Direction dir, bool water);

  friend class FlagSearch;
};

typedef bool flag_search_func(Flag *flag, void *data);

class FlagSearch {
 protected:
  Game *game;
  std::vector<Flag*> queue;
  int id;

 public:
  explicit FlagSearch(Game *game);

  int get_id() { return id; }
  void add_source(Flag *flag);
  bool execute(flag_search_func *callback,
               bool land, bool transporter, void *data);

  static bool single(Flag *src, flag_search_func *callback,
                     bool land, bool transporter, void *data);
};

#endif  // SRC_FLAG_H_
