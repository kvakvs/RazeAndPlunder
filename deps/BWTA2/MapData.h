#pragma once

#include <BWTA.h>
#include "TileType.h"

using TileID = uint16_t;

namespace BWTA {
  typedef std::list<Chokepoint*> ChokePath;
  typedef std::set<std::pair<Chokepoint*, int>> ChokeCost;
  typedef std::map<Chokepoint*, ChokeCost> ChokepointGraph;

  typedef std::pair<BWAPI::UnitType, BWAPI::Position> UnitTypePosition;

  //	typedef std::pair<BWAPI::UnitType, BWAPI::WalkPosition> UnitTypeWalkPosition;
  //	typedef std::pair<BWAPI::UnitType, BWAPI::TilePosition> UnitTypeTilePosition;

  struct unitTypeTilePos_t {
    BWAPI::UnitType type;
    BWAPI::TilePosition pos;

    unitTypeTilePos_t(BWAPI::UnitType t, BWAPI::TilePosition p): type(t), pos(p) {
    };
  };

  namespace MapData {
    extern RectangleArray<bool> walkability;
    extern RectangleArray<bool> rawWalkability;
    extern RectangleArray<bool> lowResWalkability;
    extern RectangleArray<bool> buildability;
    extern RectangleArray<int> distanceTransform;
    extern BWAPI::TilePosition::list startLocations;
    extern std::string hash;
    extern std::string mapFileName;

    extern int32_t mapWidthPixelRes;
    extern int32_t mapWidthWalkRes;
    extern int32_t mapWidthTileRes;
    extern int32_t mapHeightPixelRes;
    extern int32_t mapHeightWalkRes;
    extern int32_t mapHeightTileRes;

    extern uint16_t maxDistanceTransform;
    // data for HPA*
    extern ChokepointGraph chokeNodes;

    // offline map data
    extern TileID* TileArray;
    extern TileType* TileSet;

    /** Direct mapping of mini tile flags array */
    struct MiniTileMaps_type {
      struct MiniTileFlagArray {
        uint16_t miniTile[16];
      };

      MiniTileFlagArray tile[0x10000];
    };

    extern MiniTileMaps_type* MiniTileFlags;
    extern std::vector<UnitTypePosition> staticNeutralBuildings;
    //		extern std::vector<UnitTypeWalkPosition> resourcesWalkPositions;

    extern std::vector<unitTypeTilePos_t> resources;
  }
}
