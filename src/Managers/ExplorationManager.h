#pragma once

#include "SpottedObject.h"
#include "Commander/Squad.h"

#include "BWEM/bwem.h"
#include "RnpUtil.h"

class SpottedObjectSet : public BWAPI::SetContainer<SpottedObject*, std::hash<void*>> {
};

class RegionItem {
public:
  explicit RegionItem(const BWEM::Area* region);

  BWAPI::TilePosition location_;
  int frame_visited_ = 0;
};

class RegionSet : public BWAPI::SetContainer<RegionItem*, std::hash<void*>> {
public:

};

/** The ExplorationManager handles all tasks involving exploration of the game world. It issue orders to a number of units
 * that is used as explorers, keep track of areas recently explored, and keep track of spotted resources or enemy buildings.
 *
 * The ExplorationManager is implemented as a singleton class. Each class that needs to access ExplorationManager can request an instance,
 * and all classes shares the same ExplorationManager instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ExplorationManager {

private:
  BWEM::Map& bwem_;
  SpottedObjectSet enemy_;
  RegionSet explore_;
  int last_call_frame_ = 0;
  int site_set_frame_ = 0;
  BWAPI::TilePosition expansion_site_;

private:
  void cleanup();

public:
  ExplorationManager();
  ~ExplorationManager();

  // Called each update to issue orders. 
  void on_frame();

  // Returns the next position to explore for this squad. 
  BWAPI::TilePosition get_next_to_explore(Squad* squad);

  // Searches for the next position to expand the base to. 
  BWAPI::TilePosition search_expansion_site();

  // Returns the next position to expand the base to. 
  BWAPI::TilePosition get_expansion_site();

  // Sets the next position to expand the base to. 
  void set_expansion_site(BWAPI::TilePosition pos);

  // Shows all spotted objects as squares on the SC map. Use for debug purpose. 
  void debug_print();

  // Notifies about an enemy unit that has been spotted. 
  void on_unit_spotted(BWAPI::Unit unit);

  // Notifies that an enemy unit has been destroyed. If the destroyed unit was among
  // the spotted units, it is removed from the list. 
  void on_unit_destroyed(BWAPI::Unit unit);

  // Returns the closest enemy spotted building from a start position, 
  // or TilePosition(-1,-1) if// none was found. 
  BWAPI::TilePosition get_closest_spotted_building(BWAPI::TilePosition start);

  // Calculates the influence of spotted enemy buildings within a specified region. 
  int get_spotted_influence_in_region(const BWEM::Area* region);

  // Returns true if a ground unit can reach position b from position a.
  // Uses BWTA. 
  bool can_reach(BWAPI::TilePosition a, BWAPI::TilePosition b) const;

  // Returns true if an agent can reach position b. 
  bool can_reach(BaseAgent* agent, BWAPI::TilePosition b) const;

  // Returns Chokepoint path, without length
  template <class Pos>
  BWEM::CPPath get_path(const Pos& a, const Pos& b) const {
    return bwem_.GetPath(BWAPI::Position(a), BWAPI::Position(b));
  }

  // Returns length without Chokepoint path
  template <class Pos>
  BWEM::CPPath get_distance(const Pos& a, const Pos& b) const {
    auto p = bwem_.GetPath(BWAPI::Position(a), BWAPI::Position(b));
    return rnp::tile_distance(p, BWAPI::TilePosition(a), BWAPI::TilePosition(b));
  }

  // Sets that a region is explored. The position must be the TilePosition for the center of the
  // region. 
  void set_explored(BWAPI::TilePosition goal);

  // Returns true if an enemy is Protoss. 
  static bool enemy_is_protoss();

  // Returns true if an enemy is Zerg. 
  static bool enemy_is_zerg();

  // Returns true if an enemy is Terran. 
  static bool enemy_is_terran();

  // All enemy races are currently unknown. 
  static bool enemy_is_unknown();
};
