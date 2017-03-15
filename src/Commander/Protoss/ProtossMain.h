#pragma once

#include "Commander/Commander.h"
#include "../Squad.h"


/**  This is the Commander class for a defensive Protoss tactics.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ProtossMain : public Commander {

private:
  Squad::Ptr mainSquad;
  Squad::Ptr stealthSquad;
  Squad::Ptr detectorSquad;

public:
  ProtossMain();

  // Destructor. 
  ~ProtossMain();

  // Called each update to issue orders. 
  void computeActions() override;

  // Returns the unique id for this strategy. 
  static std::string getStrategyId() {
    return "ProtossMain";
  }
};
