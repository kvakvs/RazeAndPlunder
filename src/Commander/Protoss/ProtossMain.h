#pragma once
#if 0

#include "Commander/Commander.h"
#include "../Squad.h"


/**  This is the Commander class for a defensive Protoss tactics.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ProtossMain : public Commander {

private:
  Squad::Ptr main_sq_;
  Squad::Ptr stealth_sq_;
  Squad::Ptr detector_sq_;

public:
  ProtossMain();

  // Destructor. 
  ~ProtossMain();

  // Called each update to issue orders. 
  void on_frame() override;

  // Returns the unique id for this strategy. 
  static std::string get_strategy_id() {
    return "ProtossMain";
  }
};

#endif // 0
