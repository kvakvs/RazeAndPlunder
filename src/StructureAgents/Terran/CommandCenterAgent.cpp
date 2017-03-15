#include "CommandCenterAgent.h"
#include "../../Commander/Commander.h"
#include "Managers/AgentManager.h"
#include "Managers/Constructor.h"
#include "Glob.h"

using namespace BWAPI;

CommandCenterAgent::CommandCenterAgent(Unit mUnit) {
  unit = mUnit;
  type = unit->getType();
  unitID = unit->getID();

  hasSentWorkers = false;
  if (rnp::agent_manager()->countNoUnits(UnitTypes::Terran_Command_Center) == 0) {
    //We dont do this for the first Command Center.
    hasSentWorkers = true;
  }

  agentType = "CommandCenterAgent";
  rnp::constructor()->commandCenterBuilt();
}

void CommandCenterAgent::computeActions() {
  if (not hasSentWorkers) {
    if (not unit->isBeingConstructed()) {
      sendWorkers();
      hasSentWorkers = true;

      rnp::constructor()->addRefinery();
    }
  }

  if (not unit->isIdle()) return;

  //Build comsat addon
  if (unit->getAddon() == nullptr) {
    if (Broodwar->canMake(UnitTypes::Terran_Comsat_Station, unit)) {
      unit->buildAddon(UnitTypes::Terran_Comsat_Station);
      return;
    }
  }

  if (rnp::agent_manager()->countNoUnits(Broodwar->self()->getRace().getWorker()) < rnp::commander()->getNoWorkers()) {
    UnitType worker = Broodwar->self()->getRace().getWorker();
    if (canBuild(worker)) {
      unit->train(worker);
    }
  }

  if (rnp::commander()->needUnit(UnitTypes::Terran_SCV)) {
    if (canBuild(UnitTypes::Terran_SCV)) {
      unit->train(UnitTypes::Terran_SCV);
    }
  }
}
