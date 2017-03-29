#include "ResourceManager.h"

using namespace BWAPI;

ResourceManager::ResourceManager() {
  locks_.push_back(ResourceLock(Broodwar->self()->getRace().getResourceDepot()));
}

ResourceManager::~ResourceManager() {
}

bool ResourceManager::has_resources(UnitType type) const {
  int n_minerals = type.mineralPrice();
  int n_gas = type.gasPrice();

  return has_resources(n_minerals, n_gas);
}

bool ResourceManager::has_resources(UpgradeType type) const {
  int n_minerals = type.mineralPrice();
  int n_gas = type.gasPrice();

  return has_resources(n_minerals, n_gas);
}

bool ResourceManager::has_resources(TechType type) const {
  int n_minerals = type.mineralPrice();
  int n_gas = type.gasPrice();

  return has_resources(n_minerals, n_gas);
}

bool ResourceManager::has_resources(int neededMinerals, int neededGas) const {
  if (Broodwar->self()->minerals() - calc_locked_minerals() >= neededMinerals) {
    if (Broodwar->self()->gas() - calc_locked_gas() >= neededGas) {
      return true;
    }
  }

  return false;
}

void ResourceManager::lock_resources(UnitType type) {
  locks_.push_back(ResourceLock(type));
}

void ResourceManager::unlock_resources(UnitType type) {
  for (int i = 0; i < (int)locks_.size(); i++) {
    if (locks_[i].unit_.getID() == type.getID()) {
      locks_.erase(locks_.begin() + i);
      return;
    }
  }
}

int ResourceManager::calc_locked_minerals() const {
  int n_minerals = 0;

  for (int i = 0; i < (int)locks_.size(); i++) {
    n_minerals += locks_[i].mineral_cost_;
  }

  return n_minerals;
}

int ResourceManager::calc_locked_gas() const {
  int n_gas = 0;

  for (int i = 0; i < (int)locks_.size(); i++) {
    n_gas += locks_[i].gas_cost_;
  }

  return n_gas;
}

void ResourceManager::debug_print_info() const {
  Broodwar->drawTextScreen(5, 96, "Locked minerals: %d", calc_locked_minerals());
  Broodwar->drawTextScreen(5, 112, "Locked gas: %d", calc_locked_gas());
}
