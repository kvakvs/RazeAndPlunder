#pragma once

#include "../Commander.h"
#include "../Squad.h"

using namespace BWAPI;



/**  This is the Commander class for a defensive Protoss tactics.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ProtossMain : public Commander {

private:
  Squad* mainSquad;
  Squad* stealthSquad;
  Squad* detectorSquad;

public:
  ProtossMain();

  /** Destructor. */
  ~ProtossMain();

  /** Called each update to issue orders. */
  void computeActions() override;

  /** Returns the unique id for this strategy. */
  static std::string getStrategyId() {
    return "ProtossMain";
  }
};
