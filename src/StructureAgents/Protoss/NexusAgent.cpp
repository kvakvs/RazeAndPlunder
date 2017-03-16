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

  hasSentWorkers = false;
  if (rnp::agent_manager()->countNoUnits(UnitTypes::Protoss_Nexus) == 0) {
    //We dont do this for the first Nexus.
    hasSentWorkers = true;
  }

  rnp::constructor()->commandCenterBuilt();
}

void NexusAgent::computeActions() {
  if (not hasSentWorkers) {
    if (not unit_->isBeingConstructed()) {
      sendWorkers();
      hasSentWorkers = true;

      rnp::constructor()->addRefinery();
    }
  }

  if (not unit_->isIdle()) return;

  if (rnp::agent_manager()->countNoUnits(Broodwar->self()->getRace().getWorker()) < rnp::commander()->get_preferred_workers_count()) {
    UnitType worker = Broodwar->self()->getRace().getWorker();
    if (canBuild(worker)) {
      unit_->train(worker);
    }
  }
}
