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

  BaseAgent* findImportantUnit() const;
  static bool isImportantUnit(BaseAgent* agent);
  static bool isEMPtarget(BWAPI::Unit e);

public:
  explicit ScienceVesselAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "ScienceVesselAgent";
  }

  bool useAbilities() override;
};
