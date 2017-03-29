#include "GhostAgent.h"
#include "Managers/AgentManager.h"
#include "Glob.h"
#include <iso646.h>

using namespace BWAPI;

bool GhostAgent::use_abilities() {
  //Cloaking
  TechType cloak = TechTypes::Personnel_Cloaking;
  if (Broodwar->self()->hasResearched(cloak) && not unit_->isCloaked() && unit_->getEnergy() > 25 && not is_enemy_detector_within_range(unit_->getTilePosition(), 192)) {
    //Check if enemy units are visible
    for (auto& u : Broodwar->enemy()->getUnits()) {
      if (u->exists()) {
        if (unit_->getDistance(u) <= unit_->getType().sightRange()) {
          unit_->useTech(cloak);
          Broodwar << "Ghost used cloaking" << std::endl;
          return true;
        }
      }
    }
  }

  //Lockdown
  TechType lockdown = TechTypes::Lockdown;
  if (Broodwar->self()->hasResearched(lockdown)) {
    if (unit_->getEnergy() >= 100) {
      Unit target = findLockdownTarget();
      if (target != nullptr) {
        Broodwar << "Used Lockdown on " << target->getType().getName() << std::endl;
        unit_->useTech(lockdown, target);
        return true;
      }
    }
  }

  return false;
}

Unit GhostAgent::findLockdownTarget() const {
  int fCnt = friendlyUnitsWithinRange(224);
  if (fCnt < 2) {
    //If we dont have any attacking units nearby,
    //dont bother with lockdown.
    return nullptr;
  }

//  int maxRange = TechTypes::Lockdown.getWeapon().maxRange();

  Unit target = nullptr;
  int cTargetVal = 0;

  for (auto& u : Broodwar->enemy()->getUnits()) {
    if (u->getType().isMechanical() && not u->getLockdownTimer() == 0 && not u->getType().isBuilding()) {
      int targetVal = u->getType().destroyScore();
      if (targetVal >= 200 && targetVal > cTargetVal) {
        target = u;
        cTargetVal = targetVal;
      }
    }
  }

  return target;
}

int GhostAgent::friendlyUnitsWithinRange(int max_range) const {
  int f_cnt = 0;
  act::for_each_actor<BaseAgent>(
    [this,max_range,&f_cnt](const BaseAgent* a) {
      if (a->is_unit() && not a->is_of_type(UnitTypes::Terran_Medic)) {
        double dist = unit_->getDistance(a->get_unit());
        if (dist <= max_range) {
          f_cnt++;
        }
      }
    });
  return f_cnt;
}
