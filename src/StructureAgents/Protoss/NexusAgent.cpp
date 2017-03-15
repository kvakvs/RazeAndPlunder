#include "NexusAgent.h"
#include "Managers/AgentManager.h"
#include "Managers/Constructor.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

NexusAgent::NexusAgent(Unit mUnit) {
  unit = mUnit;
  type = unit->getType();
  unitID = unit->getID();
  agentType = "NexusAgent";

  hasSentWorkers = false;
  if (rnp::agent_manager()->countNoUnits(UnitTypes::Protoss_Nexus) == 0) {
    //We dont do this for the first Nexus.
    hasSentWorkers = true;
  }

  rnp::constructor()->commandCenterBuilt();
}

void NexusAgent::computeActions() {
  if (not hasSentWorkers) {
    if (not unit->isBeingConstructed()) {
      sendWorkers();
      hasSentWorkers = true;

      rnp::constructor()->addRefinery();
    }
  }

  if (not unit->isIdle()) return;

  if (rnp::agent_manager()->countNoUnits(Broodwar->self()->getRace().getWorker()) < rnp::commander()->getNoWorkers()) {
    UnitType worker = Broodwar->self()->getRace().getWorker();
    if (canBuild(worker)) {
      unit->train(worker);
    }
  }
}
