#pragma once
#if 0

#include "Commander/Commander.h"
#include "../Squad.h"

/** This is the Commander class for the Zerg Lurker Rush tactic.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class LurkerRush : public Commander {

private:
  Squad::Ptr main_sq_;
  Squad::Ptr l1_sq_;
  Squad::Ptr sc1_sq_;
  Squad::Ptr sc2_sq_;

public:
  LurkerRush();

  // Destructor. 
  ~LurkerRush();

  // Called each update to issue orders. 
  void on_frame() override;

  // Returns the unique id for this strategy. 
  static std::string get_strategy_id() {
    return "LurkerRush";
  }
};

#endif // 0
