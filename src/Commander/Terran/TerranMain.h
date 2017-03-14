#pragma once

#include "../Commander.h"
#include "../Squad.h"

using namespace BWAPI;



/**  This is the Commander class for a defensive Marine/Siege Tank/Goliath
 * based strategy.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class TerranMain : public Commander {

private:
  Squad* mainSquad;
  Squad* secondarySquad;
  Squad* backupSquad1;
  Squad* backupSquad2;
  Squad* sc1;
  Squad* sc2;

public:
  TerranMain();

  /** Destructor. */
  ~TerranMain();

  /** Called each update to issue orders. */
  void computeActions() override;

  /** Returns the unique id for this strategy. */
  static std::string getStrategyId() {
    return "TerranMain";
  }
};
