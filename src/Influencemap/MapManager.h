#pragma once

#include <BWAPI.h>
#include "Utils/Sets.h"

#include "BWEM/bwem.h"

struct MRegion {
  using Ptr = std::unique_ptr < MRegion >;
  struct Hasher {
    size_t operator () (const Ptr &rgn) const {
      return std::hash<int>()(rgn->region_->Id());
    }
  };
  using Set = std::unordered_set < Ptr >;

  const BWEM::Area* region_;
  int inf_own_ground;
  int inf_own_air;
  int inf_own_buildings;
  int inf_en_ground;
  int inf_en_air;
  int inf_en_buildings;

  void resetInfluence() {
    inf_own_ground = 0;
    inf_own_air = 0;
    inf_own_buildings = 0;
    inf_en_ground = 0;
    inf_en_air = 0;
    inf_en_buildings = 0;
  }
};

class MRegionSet : public BWAPI::SetContainer<MRegion*, std::hash<void*>> {
public:

};

/** This class creates an influence map where own and enemy influence (buildings, ground strength,
 * air strength) are calculate for each BWTA region.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class MapManager {
  MRegionSet map_;
  BaseLocation::Set bases_;
  int last_call_frame_ = 0;
  BWEM::Map& bwem_;

private:
  MRegion* get_mregion_for(BWAPI::Position p);

  const MRegion* get_mregion(const BWEM::Area* r);
  const BWEM::ChokePoint* find_guard_chokepoint(const MRegion* mr);
  static bool is_valid_chokepoint(const BWEM::ChokePoint* cp);

public:
  MapManager();
  ~MapManager();

  // Updates the influence map. 
  void update_influences();

  // Returns a good chokepoint to place defensive forces at. 
  const BWEM::ChokePoint* get_defense_location();

  // Checks if any region with enemy influence has been found. 
  bool has_enemy_influence();

  // Returns a suitable position to attack the enemy at. 
  BWAPI::TilePosition find_suitable_attack_position();

  // Checks if the player has infuence in the specified position. 
  bool is_under_my_influence(BWAPI::TilePosition pos);

  // Checks if the enemy has influence in the specified position. 
  bool is_under_enemy_influence(BWAPI::TilePosition pos);

  // Returns the player ground unit infuence in the specified position. 
  int get_my_influence_at(BWAPI::TilePosition pos);

  // Returns the enemy ground influence in the specified position. 
  int get_ground_influence_at(BWAPI::TilePosition pos);

  // Prints debug info to screen. 
  void debug_print_info();
};
