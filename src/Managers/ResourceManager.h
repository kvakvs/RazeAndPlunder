#pragma once

#include <BWAPI.h>
#include <vector>

class ResourceLock {
public:
  BWAPI::UnitType unit_;
  int mineral_cost_ = 0;
  int gas_cost_ = 0;

  explicit ResourceLock(BWAPI::UnitType mUnit)
      : unit_(mUnit)
      , mineral_cost_(mUnit.mineralPrice())
      , gas_cost_(mUnit.gasPrice())
  {
  }
};

/** ResourceManager handles the resources and where to spend them. An agent must ask the ResourceManager for permission to build/upgrade/research
 * before issuing the order.
 *
 * The ResourceManager is implemented as a singleton class. Each class that needs to access ResourceManager can request an instance,
 * and all classes shares the same ResourceManager instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ResourceManager {
  std::vector<ResourceLock> locks_;
  
private:
  int calc_locked_minerals() const;
  int calc_locked_gas() const;

public:
  ResourceManager();
  ~ResourceManager();

  // Checks if we have enough resources free to build the specified unit. 
  bool has_resources(BWAPI::UnitType type) const;

  // Checks if we have enough the specified resources free. 
  bool has_resources(int nMinerals, int nGas) const;

  // Checks if we have enough resources free for the specified upgrade. 
  bool has_resources(BWAPI::UpgradeType type) const;

  // Checks if we have enough resources free for the specified research. 
  bool has_resources(BWAPI::TechType type) const;

  // Locks resources for use. 
  void lock_resources(BWAPI::UnitType type);

  // Unlocks resources for use. 
  void unlock_resources(BWAPI::UnitType type);

  // Shows some debug info on screen. 
  void debug_print_info() const;
};
