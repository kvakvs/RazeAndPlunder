#pragma once

#include "../Commander.h"
#include "../Squad.h"

using namespace BWAPI;


/** This is the Commander class for the Zerg Lurker Rush tactic.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class LurkerRush : public Commander {

private:
  Squad* mainSquad;
  Squad* l1;
  Squad* sc1;
  Squad* sc2;

public:
  LurkerRush();

  /** Destructor. */
  ~LurkerRush();

  /** Called each update to issue orders. */
  void computeActions() override;

  /** Returns the unique id for this strategy. */
  static std::string getStrategyId() {
    return "LurkerRush";
  }
};
