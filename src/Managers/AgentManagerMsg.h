#pragma once
#include "Glob.h"

namespace msg::agentmanager {

class UnitDestroyed : public act::Message {
public:
  BWAPI::Unit dead_;
  explicit UnitDestroyed(BWAPI::Unit d): dead_(d) {}
};

inline void unit_destroyed(const BWAPI::Unit d) {
  act::send_message<UnitDestroyed>(rnp::agent_manager_id(), d);
}

} // ns msg
