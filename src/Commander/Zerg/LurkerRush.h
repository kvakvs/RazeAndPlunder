#pragma once

#include "../Commander.h"
#include "../Squad.h"

/** This is the Commander class for the Zerg Lurker Rush tactic.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class LurkerRush : public Commander {

private:
  Squad::Ptr mainSquad;
  Squad::Ptr l1;
  Squad::Ptr sc1;
  Squad::Ptr sc2;

public:
  LurkerRush();

  // Destructor. 
  ~LurkerRush();

  // Called each update to issue orders. 
  void computeActions() override;

  // Returns the unique id for this strategy. 
  static std::string getStrategyId() {
    return "LurkerRush";
  }
};
