#include "RefineryAgent.h"
#include "MainAgents/WorkerAgent.h"
#include "../Managers/AgentManager.h"
#include "Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

RefineryAgent::RefineryAgent(Unit mUnit): assigned_workers_() {
  unit_ = mUnit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();
  agent_type_ = "RefineryAgent";
}

void RefineryAgent::computeActions() {
  for (int i = 0; i < (int)assigned_workers_.size(); i++) {
    if (not assigned_workers_.at(i)->isAlive()) {
      assigned_workers_.erase(assigned_workers_.begin() + i);
      return;
    }
  }

  if ((int)assigned_workers_.size() < rnp::commander()->get_workers_per_refinery()) {
    if (not unit_->isBeingConstructed()
        && unit_->getPlayer()->getID() == Broodwar->self()->getID()) {

      auto worker = (WorkerAgent*)rnp::agent_manager()->findClosestFreeWorker(unit_->getTilePosition());
      if (worker != nullptr) {
        worker->getUnit()->rightClick(unit_);
        worker->setState(WorkerAgent::GATHER_GAS);
        assigned_workers_.push_back(worker);
      }
    }
  }
}
