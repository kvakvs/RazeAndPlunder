#pragma once
#if 1

#include "MainAgents/BaseAgent.h"
#include "BWEM/graph.h"

/** Helper class for the threaded Pathfinder agent. Each PathObj contains the path between two
 * two positions, start and end. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class PathObj {

private:
  BWAPI::TilePosition start_;
  BWAPI::TilePosition end_;
  BWEM::CPPath path_;
  bool finished_;

public:
  // Constructor 
  PathObj(BWAPI::TilePosition cStart, BWAPI::TilePosition cEnd);

  // Destructor 
  ~PathObj();

  // Checks if this path object matches the start and end positions. 
  bool matches(BWAPI::TilePosition cStart, BWAPI::TilePosition cEnd);

  // Checks if this path has been calculated. 
  bool is_finished();

  // Calculates the path. 
  void calculate_path();

  // Returns the path. 
  const BWEM::CPPath& get_path();
};

#endif //0
