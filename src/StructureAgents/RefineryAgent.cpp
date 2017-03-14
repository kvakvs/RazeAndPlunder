#include "RefineryAgent.h"
#include "../MainAgents/WorkerAgent.h"
#include "../Managers/AgentManager.h"
#include "../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

RefineryAgent::RefineryAgent(Unit mUnit) {
  unit = mUnit;
  type = unit->getType();
  unitID = unit->getID();
  agentType = "RefineryAgent";
}

void RefineryAgent::computeActions() {
  for (int i = 0; i < (int)assignedWorkers.size(); i++) {
    if (not assignedWorkers.at(i)->isAlive()) {
      assignedWorkers.erase(assignedWorkers.begin() + i);
      return;
    }
  }

  if ((int)assignedWorkers.size() < rnp::commander()->getWorkersPerRefinery()) {
    if (not unit->isBeingConstructed() && unit->getPlayer()->getID() == Broodwar->self()->getID()) {
      WorkerAgent* worker = (WorkerAgent*)AgentManager::getInstance()->findClosestFreeWorker(unit->getTilePosition());
      if (worker != nullptr) {
        worker->getUnit()->rightClick(unit);
        worker->setState(WorkerAgent::GATHER_GAS);
        assignedWorkers.push_back(worker);
      }
    }
  }
}
