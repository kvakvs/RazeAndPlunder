#include "CommandCenterAgent.h"
#include "Commander/Commander.h"
#include "Commander/RnpArmy.h"
#include "Glob.h"
#include "Managers/AgentManager.h"
#include "Managers/Constructor.h"

using namespace BWAPI;

CommandCenterAgent::CommandCenterAgent(Unit mUnit) {
  unit_ = mUnit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();

  has_sent_workers_ = false;
  if (rnp::agent_manager()->get_units_of_type_count(UnitTypes::Terran_Command_Center) == 0) {
    //We dont do this for the first Command Center.
    has_sent_workers_ = true;
  }

  agent_type_ = "CommandCenterAgent";
  Constructor::modify([](Constructor* c) { c->command_center_built(); });
}

void CommandCenterAgent::tick() {
  if (not has_sent_workers_) {
    if (not unit_->isBeingConstructed()) {
      send_workers();
      has_sent_workers_ = true;

      Constructor::modify([](Constructor* c) { c->add_refinery(); });
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

  if (rnp::agent_manager()->get_units_of_type_count(Broodwar->self()->getRace().getWorker()) < rnp::commander()->get_preferred_workers_count()) {
    UnitType worker = Broodwar->self()->getRace().getWorker();
    if (can_build(worker)) {
      unit_->train(worker);
    }
  }

  if (rnp::army()->need_unit(UnitTypes::Terran_SCV)) {
    if (can_build(UnitTypes::Terran_SCV)) {
      unit_->train(UnitTypes::Terran_SCV);
    }
  }
}
