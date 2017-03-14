#include "MapData.h"

namespace BWTA {
  namespace MapData {
    RectangleArray<bool> walkability;
    RectangleArray<bool> rawWalkability;
    RectangleArray<bool> lowResWalkability;
    RectangleArray<bool> buildability;
    RectangleArray<int> distanceTransform;
    BWAPI::TilePosition::list startLocations;
    std::string hash;
    std::string mapFileName;

    int32_t mapWidthPixelRes = 1;
    int32_t mapWidthWalkRes = 1;
    int32_t mapWidthTileRes = 1;
    int32_t mapHeightPixelRes = 1;
    int32_t mapHeightWalkRes = 1;
    int32_t mapHeightTileRes = 1;

    uint16_t maxDistanceTransform;
    // data for HPA*
    ChokepointGraph chokeNodes;

    // offline map data
    RectangleArray<bool> isWalkable;
    TileID* TileArray = nullptr;
    TileType* TileSet = nullptr;
    MiniTileMaps_type* MiniTileFlags = nullptr;
    std::vector<UnitTypePosition> staticNeutralBuildings;
    //		std::vector<UnitTypeWalkPosition> resourcesWalkPositions;

    std::vector<unitTypeTilePos_t> resources;
  }
}
