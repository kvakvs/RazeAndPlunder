#include "NexusAgent.h"
#include "../../Managers/AgentManager.h"
#include "../../Managers/Constructor.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

NexusAgent::NexusAgent(Unit mUnit) {
  unit = mUnit;
  type = unit->getType();
  unitID = unit->getID();
  agentType = "NexusAgent";

  hasSentWorkers = false;
  if (AgentManager::getInstance()->countNoUnits(UnitTypes::Protoss_Nexus) == 0) {
    //We dont do this for the first Nexus.
    hasSentWorkers = true;
  }

  Constructor::getInstance()->commandCenterBuilt();
}

void NexusAgent::computeActions() {
  if (not hasSentWorkers) {
    if (not unit->isBeingConstructed()) {
      sendWorkers();
      hasSentWorkers = true;

      Constructor::getInstance()->addRefinery();
    }
  }

  if (not unit->isIdle()) return;

  if (AgentManager::getInstance()->countNoUnits(Broodwar->self()->getRace().getWorker()) < rnp::commander()->getNoWorkers()) {
    UnitType worker = Broodwar->self()->getRace().getWorker();
    if (canBuild(worker)) {
      unit->train(worker);
    }
  }
}
