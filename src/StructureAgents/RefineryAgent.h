#pragma once

#include "StructureAgent.h"
using namespace BWAPI;



/** The RefineryAgent handles Refinery buildings for all races.
 *
 * Implemented abilities:
 * - Makes sure each Refinery has 3 workers assigned to gather gas.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class RefineryAgent : public StructureAgent {

private:
  std::vector<BaseAgent*> assignedWorkers;

public:
  explicit RefineryAgent(Unit mUnit);

  /** Called each update to issue orders. */
  void computeActions() override;
};
