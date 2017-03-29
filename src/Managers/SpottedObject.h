#pragma once

#include <BWAPI.h>
#include <memory>

/** The SpottedObject class is a help class for the ExplorationManager. It contains all details about a spotted
 * enemy unit or neutral resource. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class SpottedObject {

private:
  BWAPI::UnitType type_;
  BWAPI::Position position_;
  BWAPI::TilePosition tile_position_;
  int unit_id_;

public:
  using Ptr = std::unique_ptr< SpottedObject >;
  using PointerSet = std::unordered_set < SpottedObject*, std::hash<void*> >;

  // Creates an object from a unit reference. 
  explicit SpottedObject(BWAPI::Unit mUnit);

  // Returns the unique id of the spotted unit. 
  int get_unit_id() const {
    return unit_id_;
  }

  // Returns the type of the spotted unit. 
  BWAPI::UnitType get_type() const {
    return type_;
  }

  // Returns the position of the spotted unit. 
  BWAPI::Position get_position() const {
    return position_;
  }

  // Returns the tileposition of the spotted unit. 
  BWAPI::TilePosition get_tile_position() const {
    return tile_position_;
  }

  // Returns true if the SpottedObject is at this TilePosition. 
  bool is_at(BWAPI::TilePosition tile_pos) const {
    return tile_pos == tile_position_;
  }

};
