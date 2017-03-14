#pragma once

#include "UnitAgent.h"
using namespace BWAPI;



/** The TransportAgent handles transport units (Terran Dropship and Protoss Shuttle).
 *
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class TransportAgent : public UnitAgent {

private:
  int maxLoad;
  int currentLoad;
  int getCurrentLoad();
  bool isValidLoadUnit(BaseAgent* a);
  BaseAgent* findUnitToLoad(int spaceLimit);

public:
  explicit TransportAgent(Unit mUnit);

  /** Called each update to issue orders. */
  void computeActions() override;
};
