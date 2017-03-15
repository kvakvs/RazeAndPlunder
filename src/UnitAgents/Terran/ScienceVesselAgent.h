#pragma once

#include "../UnitAgent.h"

/** 
 * The ScienceVesselAgent handles abilities for Terran Science Vessels units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ScienceVesselAgent : public UnitAgent {

private:
  BaseAgent* findImportantUnit() const;
  static bool isImportantUnit(BaseAgent* agent);
  static bool isEMPtarget(BWAPI::Unit e);
  int lastIrradiateFrame;
  int lastShieldFrame;

public:
  explicit ScienceVesselAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agentType = "ScienceVesselAgent";
    lastIrradiateFrame = 0;
    lastShieldFrame = 0;
  }

  bool useAbilities() override;
};
