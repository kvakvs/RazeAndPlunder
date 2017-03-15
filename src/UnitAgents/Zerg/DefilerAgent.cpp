#include "DefilerAgent.h"
#include "Managers/AgentManager.h"
#include "Glob.h"

using namespace BWAPI;

bool DefilerAgent::useAbilities() {
  //Consume
  if (unit->getEnergy() >= 125 && Broodwar->self()->hasResearched(TechTypes::Consume)) {
    auto& agents = rnp::agent_manager()->getAgents();
    for (auto& a : agents) {
      if (a->isAlive() && a->isOfType(UnitTypes::Zerg_Zergling)) {
        double dist = a->getUnit()->getTilePosition().getDistance(unit->getTilePosition());
        if (dist <= 2) {
          if (unit->useTech(TechTypes::Consume, a->getUnit())) {
            Broodwar << "Used Consume on " << a->getUnitType().getName() << std::endl;
            return true;
          }
        }
      }
    }
  }

  //Dark Swarm
  if (unit->getEnergy() >= 100 && Broodwar->getFrameCount() - darkSwarmFrame > 100) {
    auto& agents = rnp::agent_manager()->getAgents();
    for (auto& a : agents) {
      if (a->isOfType(UnitTypes::Zerg_Mutalisk) && a->isAlive()) {
        if (a->isUnderAttack()) {
          //A Mutalisk is in combat. Cover it in Dark Swarm.
          if (unit->useTech(TechTypes::Dark_Swarm, a->getUnit()->getPosition())) {
            Broodwar << "Use Dark Swarm" << std::endl;
            darkSwarmFrame = Broodwar->getFrameCount();
            return true;
          }
        }
      }
    }
  }

  return false;
}
