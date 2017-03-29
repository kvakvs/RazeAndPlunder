#pragma once

#include "Commander/Commander.h"

/**  This is the Commander class for a defensive Marine/Siege Tank/Goliath
 * based strategy.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class TerranMain : public Commander {

private:
  act::ActorId main_sq_;
  act::ActorId secondary_sq_;
  act::ActorId backup1_sq_;
  act::ActorId backup2_sq_;
  act::ActorId rush_sq_;
  act::ActorId scout2_sq_;

public:
  TerranMain();
  ~TerranMain();

  // Called each update to issue orders.
  void tick() override;

  // Returns the unique id for this strategy. 
  static std::string get_strategy_id() {
    return "TerranMain";
  }
};
