#pragma once

#include "MainAgents/BaseAgent.h"
#include "bwem.h"

struct Corners {
  int x1;
  int y1;
  int x2;
  int y2;
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
public:
  enum class TileType {
    // Tile is buildable. 
    BUILDABLE = 1,
    // Tile is blocked and cannot be built on. 
    BLOCKED = 0,
    // Tile is temporary blocked and cannot be built on. 
    TEMPBLOCKED = 4,
    // Tile contains a mineral vein. 
    MINERAL = 2,
    // Tile contains a gas vein. 
    GAS = 3
  }; 

private:
  int range_;
  int w_;
  int h_;
  TileType** cover_map_;
  BWEM::Map& bwem_;

private:
  Corners getCorners(BWAPI::Unit unit) const;
  Corners getCorners(BWAPI::UnitType type, BWAPI::TilePosition center) const;

  BWAPI::TilePosition findSpotAtSide(BWAPI::UnitType toBuild, BWAPI::TilePosition start, BWAPI::TilePosition end);
  bool canBuildAt(BWAPI::UnitType toBuild, BWAPI::TilePosition pos);

  void fill(Corners c);
  void clear(Corners c);

  BWAPI::Unit findWorker(BWAPI::TilePosition spot);

  static bool suitableForDetector(BWAPI::TilePosition pos);

  static bool baseUnderConstruction(BaseAgent* base);
  static bool isDefenseBuilding(BWAPI::UnitType toBuild);
  BWAPI::TilePosition findBuildSpot(BWAPI::UnitType toBuild, BWAPI::TilePosition start);

  static BWAPI::Unit hasMineralNear(BWAPI::TilePosition pos);

public:
  BuildingPlacer();
  ~BuildingPlacer();

  // Adds a newly constructed building to the cover map. 
  void addConstructedBuilding(BWAPI::Unit unit);

  // Used by WorkerAgent when constructing builds. 
  void fillTemp(BWAPI::UnitType toBuild, BWAPI::TilePosition buildSpot);

  // Used by WorkerAgent when constructing builds. 
  void clearTemp(BWAPI::UnitType toBuild, BWAPI::TilePosition buildSpot);

  // Called when a building is destroyed, to free up the space. 
  void buildingDestroyed(BWAPI::Unit unit);

  // Checks if the specified building type can be built at the buildSpot. True if it can,
  // false otherwise. 
  bool canBuild(BWAPI::UnitType toBuild, BWAPI::TilePosition buildSpot);

  // Checks if a position is free. 
  bool positionFree(BWAPI::TilePosition pos);

  // Blocks a position from being used as a valid buildSpot. Used when a worker is timedout when
  // moving towards the buildSpot. 
  void blockPosition(BWAPI::TilePosition buildSpot);

  // Finds and returns a buildSpot for the specified building type.
  // If no buildspot is found, a TilePosition(-1,-1) is returned. 
  BWAPI::TilePosition findBuildSpot(BWAPI::UnitType toBuild);

  // Searches for the closest vespene gas that is not in use. If no gas is sighted,
  // the ExplorationManager is queried. 
  BWAPI::TilePosition findRefineryBuildSpot(BWAPI::UnitType toBuild, BWAPI::TilePosition start);

  // Finds and returns the position of the closest free vespene gas around the specified start position.
  // If no gas vein is found, a TilePosition(-1, -1) is returned. 
  BWAPI::TilePosition findClosestGasWithoutRefinery(BWAPI::UnitType toBuild, BWAPI::TilePosition start);

  // Searches for a spot to build a refinery at. Returns TilePosition(-1, -1) if no spot was found.
  BWAPI::TilePosition searchRefinerySpot() const;

  // Returns a position of a suitable site for expansion, i.e. new bases. 
  BWAPI::TilePosition findExpansionSite() const;

  // Finds a mineral to gather from. 
  BWAPI::Unit findClosestMineral(BWAPI::TilePosition workerPos);

  // Shows debug info on screen. 
  void debug() const;  
};

