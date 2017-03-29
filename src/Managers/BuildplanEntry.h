#pragma once

#include <BWAPI.h>
#include <cassert>
#include "Glob.h"

/** Helper class for storing buildings/techs/upgrades to construct.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BuildplanEntry {
public:
  enum class Type {
    BUILDING = 0,
    UPGRADE = 1,
    TECH = 2
  };

private:
  Type type_ = Type::BUILDING;
  BWAPI::UnitType unit_type_;
  BWAPI::UpgradeType upgrade_type_;
  BWAPI::TechType tech_type_;
  int supply_ = 0;

public:
  BuildplanEntry(BWAPI::UnitType cType, int cSupply);
  BuildplanEntry(BWAPI::UpgradeType cType, int cSupply);
  BuildplanEntry(BWAPI::TechType cType, int cSupply);
  
  int trigger_at_supply() const { return supply_; }
  
  Type type() const { return type_; }
  
  const BWAPI::UnitType& unit_type() const {
    assert(type_ == Type::BUILDING);
    return unit_type_;
  }

  const BWAPI::UpgradeType& upgrade_type() const {
    assert(type_ == Type::UPGRADE);
    return upgrade_type_;
  }

  const BWAPI::TechType& tech_type() const {
    assert(type_ == Type::TECH);
    return tech_type_;
  }

  std::string to_string() const;
};

class Buildplan {
  std::vector<BuildplanEntry> plan_;
public:
  template <class Something>
  void add(const Something& unit_upgrade_or_tech, size_t supply) {
    rnp::log()->trace("buildplan: {} at {}",
                      unit_upgrade_or_tech.toString(), supply);
    plan_.emplace_back(unit_upgrade_or_tech, supply);
  }

  void drop(size_t i);

  // EntryHandler returns true if the build plan entry was taken, to erase it
  using EntryHandler = std::function<bool(const BuildplanEntry& unit)>;

  void for_each(size_t supply,
                EntryHandler on_unit,
                EntryHandler on_upgrade, 
                EntryHandler on_tech);

  bool empty() const { return plan_.empty(); }

  void debug_print_info() const;
};
