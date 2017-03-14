#pragma once

#include <BWAPI.h>

/** The SpottedObject class is a help class for the ExplorationManager. It contains all details about a spotted
 * enemy unit or neutral resource. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class SpottedObject {

private:
  BWAPI::UnitType type;
  BWAPI::Position position;
  BWAPI::TilePosition tilePosition;
  int unitID;

public:
  // Creates an object from a unit reference. 
  explicit SpottedObject(BWAPI::Unit mUnit);

  // Returns the unique id of the spotted unit. 
  int getUnitID();

  // Returns the type of the spotted unit. 
  BWAPI::UnitType getType();

  // Returns the position of the spotted unit. 
  BWAPI::Position getPosition();

  // Returns the tileposition of the spotted unit. 
  BWAPI::TilePosition getTilePosition();

  // Returns true if the SpottedObject is at this TilePosition. 
  bool isAt(BWAPI::TilePosition tilePos);

};
