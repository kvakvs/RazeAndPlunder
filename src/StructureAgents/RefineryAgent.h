#pragma once

#include "StructureAgent.h"

/** The RefineryAgent handles Refinery buildings for all races.
 *
 * Implemented abilities:
 * - Makes sure each Refinery has 3 workers assigned to gather gas.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class RefineryAgent : public StructureAgent {

private:
  act::ActorId::Set assigned_workers_;

public:
  explicit RefineryAgent(BWAPI::Unit mUnit);

  // Called each update to issue orders. 
  void tick() override;
};
