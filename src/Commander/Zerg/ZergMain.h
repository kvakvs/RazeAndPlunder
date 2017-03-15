#pragma once

#include "Commander/Commander.h"
#include "../Squad.h"

/** This is the Commander class for a balanced Zerg tactic
 * based on Hydralisks and Mutalisks.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ZergMain : public Commander {

private:
  Squad::Ptr mainSquad;
  Squad::Ptr l1;
  Squad::Ptr sc1;
  Squad::Ptr sc2;

public:
  ZergMain();

  // Destructor. 
  ~ZergMain();

  // Called each update to issue orders. 
  void computeActions() override;

  // Returns the unique id for this strategy. 
  static std::string getStrategyId() {
    return "ZergMain";
  }
};
