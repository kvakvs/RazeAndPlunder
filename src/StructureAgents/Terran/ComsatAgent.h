#pragma once

#include "../StructureAgent.h"
using namespace BWAPI;


/** The ComsatAgent handles Terran Comsat Station buildings.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ComsatAgent : public StructureAgent {
  int friendlyUnitsWithinRange(Position pos);
  int lastSweepFrame;
  TilePosition lastSweepPos;
  bool anyHasSweeped(TilePosition pos);

public:
  explicit ComsatAgent(Unit mUnit);

  /** Called each update to issue orders. */
  void computeActions() override;

  /** Checks if this Comsat has sweeped the specified position within the previos 100 frames. */
  bool hasSweeped(TilePosition pos);
};
