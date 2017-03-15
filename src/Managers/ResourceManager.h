#pragma once

#include <BWAPI.h>
#include <vector>

struct ResourceLock {
  BWAPI::UnitType unit;
  int mineralCost;
  int gasCost;

  explicit ResourceLock(BWAPI::UnitType mUnit) {
    unit = mUnit;
    mineralCost = mUnit.mineralPrice();
    gasCost = mUnit.gasPrice();
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
  int calcLockedMinerals();
  int calcLockedGas();

public:
  ResourceManager();
  ~ResourceManager();

  // Checks if we have enough resources free to build the specified unit. 
  bool hasResources(BWAPI::UnitType type);

  // Checks if we have enough the specified resources free. 
  bool hasResources(int nMinerals, int nGas);

  // Checks if we have enough resources free for the specified upgrade. 
  bool hasResources(BWAPI::UpgradeType type);

  // Checks if we have enough resources free for the specified research. 
  bool hasResources(BWAPI::TechType type);

  // Locks resources for use. 
  void lockResources(BWAPI::UnitType type);

  // Unlocks resources for use. 
  void unlockResources(BWAPI::UnitType type);

  // Shows some debug info on screen. 
  void printInfo();
};
