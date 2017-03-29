#include "NexusAgent.h"
#include "Managers/AgentManager.h"
#include "Managers/Constructor.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

NexusAgent::NexusAgent(Unit mUnit) {
  unit_ = mUnit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();
  agent_type_ = "NexusAgent";

  has_sent_workers_ = false;
  if (rnp::agent_manager()->get_units_of_type_count(UnitTypes::Protoss_Nexus) == 0) {
    //We dont do this for the first Nexus.
    has_sent_workers_ = true;
  }

  Constructor::modify([](Constructor* c) { c->command_center_built(); });
}

void NexusAgent::tick() {
  if (not has_sent_workers_) {
    if (not unit_->isBeingConstructed()) {
      send_workers();
      has_sent_workers_ = true;

      Constructor::modify([](Constructor* c) { c->add_refinery(); });
    }
  }

  if (not unit_->isIdle()) return;

  auto worker_count = rnp::agent_manager()->get_units_of_type_count(Broodwar->self()->getRace().getWorker());
  if (worker_count < rnp::commander()->get_preferred_workers_count()) {
    UnitType worker = Broodwar->self()->getRace().getWorker();
    if (can_build(worker)) {
      unit_->train(worker);
    }
  }
}
