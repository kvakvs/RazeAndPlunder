#include "RefineryAgent.h"
#include "MainAgents/WorkerAgent.h"
#include "Managers/AgentManager.h"
#include "Commander/Commander.h"
#include "Glob.h"
#include "RnpUtil.h"

using namespace BWAPI;

RefineryAgent::RefineryAgent(Unit mUnit): assigned_workers_() {
  unit_ = mUnit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();
  agent_type_ = "RefineryAgent";
}

void RefineryAgent::tick() {
//  for (size_t i = 0; i < assigned_workers_.size(); i++) {
//    if (not assigned_workers_[i]->is_alive()) {
//      assigned_workers_.erase(assigned_workers_.begin() + i);
//      return;
//    }
//  }

  if ((int)assigned_workers_.size() < rnp::commander()->get_workers_per_refinery()) {
    if (not unit_->isBeingConstructed()
      && rnp::is_my_unit(unit_)) {

      auto worker = static_cast<const WorkerAgent*>(
        rnp::agent_manager()->find_closest_free_worker(unit_->getTilePosition())
        );
      if (worker) {
        auto& worker_id = worker->self();
        msg::worker::right_click_refinery(worker_id, unit_);
        assigned_workers_.insert(worker_id);
        ac_monitor(worker_id);
      }
    }
  }
}
