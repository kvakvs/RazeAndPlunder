#include "CommandCenterAgent.h"
#include "../../Commander/Commander.h"
#include "Managers/AgentManager.h"
#include "Managers/Constructor.h"
#include "Glob.h"

using namespace BWAPI;

CommandCenterAgent::CommandCenterAgent(Unit mUnit) {
  unit_ = mUnit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();

  has_sent_workers_ = false;
  if (rnp::agent_manager()->countNoUnits(UnitTypes::Terran_Command_Center) == 0) {
    //We dont do this for the first Command Center.
    has_sent_workers_ = true;
  }

  agent_type_ = "CommandCenterAgent";
  rnp::constructor()->commandCenterBuilt();
}

void CommandCenterAgent::computeActions() {
  if (not has_sent_workers_) {
    if (not unit_->isBeingConstructed()) {
      sendWorkers();
      has_sent_workers_ = true;

      rnp::constructor()->addRefinery();
    }
  }

  if (not unit_->isIdle()) return;

  //Build comsat addon
  if (unit_->getAddon() == nullptr) {
    if (Broodwar->canMake(UnitTypes::Terran_Comsat_Station, unit_)) {
      unit_->buildAddon(UnitTypes::Terran_Comsat_Station);
      return;
    }
  }

  if (rnp::agent_manager()->countNoUnits(Broodwar->self()->getRace().getWorker()) < rnp::commander()->get_preferred_workers_count()) {
    UnitType worker = Broodwar->self()->getRace().getWorker();
    if (canBuild(worker)) {
      unit_->train(worker);
    }
  }

  if (rnp::commander()->is_unit_needed(UnitTypes::Terran_SCV)) {
    if (canBuild(UnitTypes::Terran_SCV)) {
      unit_->train(UnitTypes::Terran_SCV);
    }
  }
}
