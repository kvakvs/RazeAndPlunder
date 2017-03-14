#pragma once

#include <BWAPI.h>

/** Helper class for storing buildings/techs/upgrades to construct.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BuildplanEntry {

private:

public:
  BuildplanEntry(BWAPI::UnitType cType, int cSupply);
  BuildplanEntry(BWAPI::UpgradeType cType, int cSupply);
  BuildplanEntry(BWAPI::TechType cType, int cSupply);

  int type;
  BWAPI::UnitType unittype;
  BWAPI::UpgradeType upgradetype;
  BWAPI::TechType techtype;
  int supply;

  static const int BUILDING = 0;
  static const int UPGRADE = 1;
  static const int TECH = 2;
};
