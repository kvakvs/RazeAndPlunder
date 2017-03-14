#pragma once

#include "../MainAgents/BaseAgent.h"

using namespace BWAPI;

/** Helper class for the threaded Pathfinder agent. Each PathObj contains the path between two
 * two positions, start and end. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class PathObj {

private:
  TilePosition start;
  TilePosition end;
  std::vector<TilePosition> path;
  bool finished;

public:
  /** Constructor */
  PathObj(TilePosition cStart, TilePosition cEnd);

  /** Destructor */
  ~PathObj();

  /** Checks if this path object matches the start and end positions. */
  bool matches(TilePosition cStart, TilePosition cEnd);

  /** Checks if this path has been calculated. */
  bool isFinished();

  /** Calculates the path. */
  void calculatePath();

  /** Returns the path. */
  std::vector<TilePosition> getPath();
};
