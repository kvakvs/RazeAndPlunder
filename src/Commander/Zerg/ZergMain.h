#pragma once

#include "../Commander.h"
#include "../Squad.h"

using namespace BWAPI;



/** This is the Commander class for a balanced Zerg tactic
 * based on Hydralisks and Mutalisks.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ZergMain : public Commander {

private:
  Squad* mainSquad;
  Squad* l1;
  Squad* sc1;
  Squad* sc2;

public:
  ZergMain();

  /** Destructor. */
  ~ZergMain();

  /** Called each update to issue orders. */
  void computeActions() override;

  /** Returns the unique id for this strategy. */
  static std::string getStrategyId() {
    return "ZergMain";
  }
};
