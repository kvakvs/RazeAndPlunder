#pragma once

#include "../Commander.h"
#include "../Squad.h"

/**  This is the Commander class for a defensive Marine/Siege Tank/Goliath
 * based strategy.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class TerranMain : public Commander {

private:
  Squad::Ptr main_sq_;
  Squad::Ptr secondary_sq_;
  Squad::Ptr backup1_sq_;
  Squad::Ptr backup2_sq_;
  Squad::Ptr scout1_sq_;
  Squad::Ptr scout2_sc_;

public:
  TerranMain();
  ~TerranMain();

  // Called each update to issue orders.
  void computeActions() override;

  // Returns the unique id for this strategy. 
  static std::string getStrategyId() {
    return "TerranMain";
  }
};
