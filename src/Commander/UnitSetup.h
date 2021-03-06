#pragma once

#include <BWAPI.h>

/** This class is used by the Squad class to handle the number of units of a specified type is
 * in the squad.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class UnitSetup {
public:
  // Default constructor. 
  UnitSetup();

  // Type of unit. 
  BWAPI::UnitType type_;
  // Wanted number of the unit. 
  size_t wanted_count_ = 0;
  // Current number of the unit. 
  size_t current_count_ = 0;

  // Checks if this setup equals to the specified type. 
  bool equals(BWAPI::UnitType mType) const;

  // Checks if unittypes matches. Needed for morphing units like Siege Tanks and Lurkers. 
  static bool equals(BWAPI::UnitType t1, BWAPI::UnitType t2);

private:
  static BWAPI::UnitType shrink(BWAPI::UnitType t);
};
