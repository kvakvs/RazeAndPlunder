#pragma once

#include "UnitAgent.h"

/** The TransportAgent handles transport units (Terran Dropship and Protoss Shuttle).
 *
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class TransportAgent : public UnitAgent {

private:
  int maxLoad = 0;
  int currentLoad = 0;

  int getCurrentLoad();
  bool isValidLoadUnit(BaseAgent* a);
  BaseAgent* findUnitToLoad(int spaceLimit);

public:
  explicit TransportAgent(BWAPI::Unit mUnit);

  // Called each update to issue orders. 
  void computeActions() override;
};
