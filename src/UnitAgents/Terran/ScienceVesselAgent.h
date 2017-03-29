#pragma once

#include "../UnitAgent.h"

/** 
 * The ScienceVesselAgent handles abilities for Terran Science Vessels units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ScienceVesselAgent : public UnitAgent {

private:
  int last_irradiate_frame_ = 0;
  int last_shield_frame_ = 0;

  const BaseAgent* find_important_unit() const;
  static bool is_important_unit(const BaseAgent* agent);
  static bool is_emp_target(BWAPI::Unit e);

public:
  explicit ScienceVesselAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "ScienceVesselAgent";
  }

  bool use_abilities() override;
};
