#pragma once

#include "SpottedObject.h"
#include "Commander/Squad.h"
#include "BWEM/bwem.h"
#include "Glob.h"
#include "RnpUtil.h"
#include <functional>
#include <unordered_set>

class RegionItem {
  BWAPI::TilePosition location_;
  int frame_visited_ = 0;

public:
  using Ptr = std::unique_ptr < RegionItem > ;
  struct Hasher {
    size_t operator () (const Ptr &rgn) const { 
      return std::hash<int>()(rgn->location_.x)
        ^ std::hash<int>()(rgn->location_.y);
    }
  };
  using Set = std::unordered_set< Ptr >;

  explicit RegionItem(const BWEM::Area* region);
  explicit RegionItem(const BWAPI::TilePosition& pos);

  const BWAPI::TilePosition& location() const { return location_; };
  int frame_visited() const { return frame_visited_; }
  void visited() { frame_visited_ = BWAPI::Broodwar->getFrameCount(); }
};


// The ExplorationManager handles all tasks involving exploration of the 
// game world. It issue orders to a number of units that is used as explorers, 
// keep track of areas recently explored, and keep track of spotted resources 
// or enemy buildings.
// Author: Johan Hagelback (johan.hagelback@gmail.com)
class ExplorationManager : public act::Actor {

private:
  BWEM::Map& bwem_;
  SpottedObject::PointerSet enemy_buildings_;
  RegionItem::Set explore_;
  int last_call_frame_ = 0;
  int site_set_frame_ = 0;
  BWAPI::TilePosition expansion_site_;
  
  // Cached result for random unexplored base (possibly under enemy influence)
  //rnp::Memoize<BWAPI::TilePosition> m_random_explore_;

private:
  void cleanup();

public:
  ExplorationManager();
  ~ExplorationManager();

  BWAPI::TilePosition get_random_unexplored_base() const;

  // Returns the next position to explore for this squad. 
  BWAPI::TilePosition get_next_to_explore(Squad* squad) const;

  // Searches for the next position to expand the base to. 
  void search_expansion_site();

  // Returns the next position to expand the base to. 
  BWAPI::TilePosition get_expansion_site() const;

  // Sets the next position to expand the base to. 
  void set_expansion_site(BWAPI::TilePosition pos);

  // Shows all spotted objects as squares on the SC map. Use for debug purpose. 
  void debug_print() const;

  // Notifies about an enemy unit that has been spotted. 
  void on_unit_spotted(BWAPI::Unit unit);

  // Notifies that an enemy unit has been destroyed. If the destroyed unit was among
  // the spotted units, it is removed from the list. 
  void on_unit_destroyed(BWAPI::Unit unit);

  // Returns the closest enemy spotted building from a start position, 
  // or TilePosition(-1,-1) if none was found. 
  BWAPI::TilePosition 
  get_closest_spotted_building(BWAPI::TilePosition start) const;

  BWAPI::TilePosition get_random_spotted_building() const;

  // Calculates the influence of spotted enemy buildings within a specified region. 
  int get_spotted_influence_in_region(const BWEM::Area* region) const;

  // Returns true if a ground unit can reach position b from position a.
  // Uses BWTA. 
  static bool can_reach(BWAPI::TilePosition a, BWAPI::TilePosition b);

  // Returns true if an agent can reach position b. 
  bool can_reach(BaseAgent* agent, BWAPI::TilePosition b) const;

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

  //
  // Actor things
  //

  // Don't let lambdas capture references, use [=] instead
  static void modify(std::function<void(ExplorationManager*)> f) {
    act::modify_actor<ExplorationManager>(rnp::exploration_id(), f);
  }

  // Called each update to issue orders. 
  void tick() override;

  uint32_t ac_flavour() const override {
    return static_cast<uint32_t>(ActorFlavour::Singleton);
  }

  void handle_message(act::Message* incoming) override;

  act::ActorId::Set actors_in_range(const BWAPI::TilePosition& center,
                                    int radius) const;
};

namespace rnp {

// Iterates over actors of type Cls found in range
template <class Cls>
void for_each_in_range(const BWAPI::TilePosition& center, int radius,
                       std::function<void(const Cls*)> fun) {
  auto found_ids = rnp::exploration()->actors_in_range(center, radius);
  act::for_each_in<Cls>(found_ids, fun);
}

inline act::ActorId::Set
actors_in_range(const BWAPI::TilePosition& center, int radius) {
  return rnp::exploration()->actors_in_range(center, radius);
}

} // ns rnp
