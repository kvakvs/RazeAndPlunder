#include "Pathfinder.h"
#include "Utils/Profiler.h"
#include "PathAB.h"

using namespace BWAPI;

Pathfinder::Pathfinder() {
}

Pathfinder::~Pathfinder() {
}

int Pathfinder::get_dist(const BWAPI::TilePosition& start,
                         const BWAPI::TilePosition& end) {
  rnp::PathAB path(start, end);
  if (path.is_good()) {
    return path.length();
  }
  return -1;
}

rnp::PathAB::Ptr Pathfinder::get_path(const BWAPI::TilePosition& start,
                                      const BWAPI::TilePosition& end) {
  return std::make_unique<rnp::PathAB>(start, end);
}
