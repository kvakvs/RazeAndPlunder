#pragma once
#if 0

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
  Squad::Ptr scout_sq_;
  Squad::Ptr rush_sq_;

public:
  ZergMain();

  // Destructor. 
  ~ZergMain();

  // Called each update to issue orders. 
  void on_frame() override;

  // Returns the unique id for this strategy. 
  static std::string get_strategy_id() {
    return "ZergMain";
  }
};

#endif // 0
