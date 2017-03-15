#include "GhostAgent.h"
#include "Managers/AgentManager.h"
#include "Glob.h"

using namespace BWAPI;

bool GhostAgent::useAbilities() {
  //Cloaking
  TechType cloak = TechTypes::Personnel_Cloaking;
  if (Broodwar->self()->hasResearched(cloak) && !unit->isCloaked() && unit->getEnergy() > 25 && !isDetectorWithinRange(unit->getTilePosition(), 192)) {
    //Check if enemy units are visible
    for (auto& u : Broodwar->enemy()->getUnits()) {
      if (u->exists()) {
        if (unit->getDistance(u) <= unit->getType().sightRange()) {
          unit->useTech(cloak);
          Broodwar << "Ghost used cloaking" << std::endl;
          return true;
        }
      }
    }
  }

  //Lockdown
  TechType lockdown = TechTypes::Lockdown;
  if (Broodwar->self()->hasResearched(lockdown)) {
    if (unit->getEnergy() >= 100) {
      Unit target = findLockdownTarget();
      if (target != nullptr) {
        Broodwar << "Used Lockdown on " << target->getType().getName() << std::endl;
        unit->useTech(lockdown, target);
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
    if (u->getType().isMechanical() && !u->getLockdownTimer() == 0 && !u->getType().isBuilding()) {
      int targetVal = u->getType().destroyScore();
      if (targetVal >= 200 && targetVal > cTargetVal) {
        target = u;
        cTargetVal = targetVal;
      }
    }
  }

  return target;
}

int GhostAgent::friendlyUnitsWithinRange(int maxRange) const {
  int fCnt = 0;
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isUnit() && !a->isOfType(UnitTypes::Terran_Medic)) {
      double dist = unit->getDistance(a->getUnit());
      if (dist <= maxRange) {
        fCnt++;
      }
    }
  }
  return fCnt;
}
