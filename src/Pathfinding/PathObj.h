#pragma once

#include "MainAgents/BaseAgent.h"
#include "BWEM/graph.h"

/** Helper class for the threaded Pathfinder agent. Each PathObj contains the path between two
 * two positions, start and end. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class PathObj {

private:
  BWAPI::TilePosition start;
  BWAPI::TilePosition end;
  //std::vector<TilePosition> path;
  BWEM::CPPath path;
  bool finished;

public:
  // Constructor 
  PathObj(BWAPI::TilePosition cStart, BWAPI::TilePosition cEnd);

  // Destructor 
  ~PathObj();

  // Checks if this path object matches the start and end positions. 
  bool matches(BWAPI::TilePosition cStart, BWAPI::TilePosition cEnd);

  // Checks if this path has been calculated. 
  bool isFinished();

  // Calculates the path. 
  void calculatePath();

  // Returns the path. 
  const BWEM::CPPath& getPath();
};
