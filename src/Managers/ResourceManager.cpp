#include "ResourceManager.h"

using namespace BWAPI;

ResourceManager::ResourceManager() {
  locks_.push_back(ResourceLock(Broodwar->self()->getRace().getResourceDepot()));
}

ResourceManager::~ResourceManager() {
}

bool ResourceManager::hasResources(UnitType type) {
  int nMinerals = type.mineralPrice();
  int nGas = type.gasPrice();

  return hasResources(nMinerals, nGas);
}

bool ResourceManager::hasResources(UpgradeType type) {
  int nMinerals = type.mineralPrice();
  int nGas = type.gasPrice();

  return hasResources(nMinerals, nGas);
}

bool ResourceManager::hasResources(TechType type) {
  int nMinerals = type.mineralPrice();
  int nGas = type.gasPrice();

  return hasResources(nMinerals, nGas);
}

bool ResourceManager::hasResources(int neededMinerals, int neededGas) {
  if (Broodwar->self()->minerals() - calcLockedMinerals() >= neededMinerals) {
    if (Broodwar->self()->gas() - calcLockedGas() >= neededGas) {
      return true;
    }
  }

  return false;
}

void ResourceManager::lockResources(UnitType type) {
  locks_.push_back(ResourceLock(type));
}

void ResourceManager::unlockResources(UnitType type) {
  for (int i = 0; i < (int)locks_.size(); i++) {
    if (locks_[i].unit_.getID() == type.getID()) {
      locks_.erase(locks_.begin() + i);
      return;
    }
  }
}

int ResourceManager::calcLockedMinerals() {
  int nMinerals = 0;

  for (int i = 0; i < (int)locks_.size(); i++) {
    nMinerals += locks_[i].mineral_cost_;
  }

  return nMinerals;
}

int ResourceManager::calcLockedGas() {
  int nGas = 0;

  for (int i = 0; i < (int)locks_.size(); i++) {
    nGas += locks_[i].gas_cost_;
  }

  return nGas;
}

void ResourceManager::printInfo() {
  Broodwar->drawTextScreen(5, 96, "Locked minerals: %d", calcLockedMinerals());
  Broodwar->drawTextScreen(5, 112, "Locked gas: %d", calcLockedGas());
}
