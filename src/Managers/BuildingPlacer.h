#pragma once

#include "MainAgents/BaseAgent.h"
#include "BWEM/bwem.h"
#include "RnpUtil.h"
#include "MainAgents/AgentFactory.h"
#include "MainAgents/AgentFactory.h"

struct BuildingBox {
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;

  BuildingBox() {}
  BuildingBox(int a1, int b1, int a2, int b2) 
  : x1(a1), y1(b1), x2(a2), y2(b2) 
  {
  }
};

enum class TileType {
  // Tile is buildable. 
  BUILDABLE = 0,
  // Tile is blocked and cannot be built on. 
  BLOCKED = 1,
  // Tile is temporary blocked and cannot be built on. 
  TEMPBLOCKED = 4,
  // Tile contains a mineral vein. 
  MINERAL = 2,
  // Tile contains a gas vein. 
  GAS = 3
};

class CoverMap {
private:
  std::vector<TileType> data_;
  size_t width_ = 0;
  size_t height_ = 0;

public:
  CoverMap(size_t xsize, size_t ysize);
  void set(size_t x, size_t y, TileType tt) {
    RNP_ASSERT(x < width_);
    RNP_ASSERT(y < height_);
    data_[x + y * width_] = tt;
  }
  TileType get(size_t x, size_t y) const {
    RNP_ASSERT(x < width_);
    RNP_ASSERT(y < height_);
    return data_[x + y * width_];
  }

  bool is_buildable(const BuildingBox& bbox) const;
};

/** The BuildingPlacer class is used to find positions to place new buildings at. It keeps track
 * of which map tiles are occupied (by own buildings) or unavailable (blocked by terrain, minerals patches etc.).
 *
 * Internally a matrix of the same size as the map is used. If a Tile is occupied or cant be reached by ground
 * units, the value if the tile is 0. If the Tile can be built on, the value is 1. 
 * Buildings typically use up more space in the matrix than their actual size since we want some free space
 * around each building. Different types of buildings have different space requirements.
 *
 * The BuildingPlacer is implemented as a singleton class. Each class that needs to access BuildingPlacer can request an instance,
 * and all classes shares the same BuildingPlacer instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BuildingPlacer {
private:
  size_t range_ = 0;
  size_t map_width_ = 0;
  size_t map_height_ = 0;
  CoverMap cover_map_;
  BWEM::Map& bwem_;

public:
  BuildingPlacer();
  ~BuildingPlacer();

  BuildingPlacer(const BuildingPlacer&) = delete;
  BuildingPlacer& operator=(const BuildingPlacer&) = delete;

  // Adds a newly constructed building to the cover map. 
  void add_constructed_building(BWAPI::Unit unit);

  // Used by WorkerAgent when constructing builds. 
  void fill_temp(BWAPI::UnitType toBuild, BWAPI::TilePosition buildSpot);

  // Used by WorkerAgent when constructing builds. 
  void clear_temp(BWAPI::UnitType toBuild, const BWAPI::TilePosition& buildSpot);

  // Called when a building is destroyed, to free up the space. 
  void on_building_destroyed(BWAPI::Unit unit);

  // Checks if the specified building type can be built at the buildSpot. True if it can,
  // false otherwise. 
  bool can_build(BWAPI::UnitType toBuild, BWAPI::TilePosition buildSpot) const;

  // Checks if a position is free. 
  bool is_position_available(BWAPI::TilePosition pos) const;

  // Blocks a position from being used as a valid buildSpot. Used when a worker is timedout when
  // moving towards the buildSpot. 
  void mark_position_blocked(BWAPI::TilePosition buildSpot);

  // Finds and returns a buildSpot for the specified building type.
  // If no buildspot is found, a TilePosition(-1,-1) is returned. 
  BWAPI::TilePosition find_build_spot(BWAPI::UnitType toBuild);

  // Searches for the closest vespene gas that is not in use. If no gas is sighted,
  // the ExplorationManager is queried. 
  BWAPI::TilePosition find_refinery_build_spot(
    BWAPI::UnitType toBuild, const BWAPI::TilePosition& start) const;

  // Finds and returns the position of the closest free vespene gas around the specified start position.
  // If no gas vein is found, a rnp::invalid_pos() is returned. 
  BWAPI::TilePosition find_closest_gas_without_refinery(
    BWAPI::UnitType toBuild, const BWAPI::TilePosition& start) const;

  // Searches for a spot to build a refinery at. Returns rnp::invalid_pos() if no spot was found.
  BWAPI::TilePosition search_refinery_spot() const;

  // Returns a position of a suitable site for expansion, i.e. new bases. 
  BWAPI::TilePosition find_expansion_site() const;

  // Finds a mineral to gather from. 
  BWAPI::Unit find_closest_mineral(BWAPI::TilePosition workerPos) const;

  // Shows debug info on screen. 
  void show_debug() const;  

private:
  BWAPI::TilePosition find_spot_at_side(
    BWAPI::UnitType toBuild, BWAPI::TilePosition start, BWAPI::TilePosition end) const;

  bool can_build_at(BWAPI::UnitType toBuild, BWAPI::TilePosition pos) const;

  void fill(BuildingBox c);

  void clear(const BuildingBox& c);

  static BWAPI::Unit find_worker(BWAPI::TilePosition spot);

  static bool suitable_for_detector(BWAPI::TilePosition pos);

  static bool base_under_construction(const BaseAgent* base);

  static bool is_defense_building(BWAPI::UnitType toBuild);

  BWAPI::TilePosition find_build_spot(
    BWAPI::UnitType toBuild, BWAPI::TilePosition start) const;

  static BWAPI::Unit has_mineral_near(BWAPI::TilePosition pos);

  BuildingBox get_building_box(BWAPI::Unit unit) const;

  BuildingBox get_building_box(BWAPI::UnitType type, const BWAPI::TilePosition& center) const;
};

