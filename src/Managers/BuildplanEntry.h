#pragma once

#include <BWAPI.h>

/** Helper class for storing buildings/techs/upgrades to construct.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BuildplanEntry {

public:
  int type_ = 0;
  BWAPI::UnitType unit_type_;
  BWAPI::UpgradeType upgrade_type_;
  BWAPI::TechType tech_type_;
  int supply_ = 0;

public:
  BuildplanEntry(BWAPI::UnitType cType, int cSupply);
  BuildplanEntry(BWAPI::UpgradeType cType, int cSupply);
  BuildplanEntry(BWAPI::TechType cType, int cSupply);

  static const int BUILDING = 0;
  static const int UPGRADE = 1;
  static const int TECH = 2;
};
