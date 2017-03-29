#include "HatcheryAgent.h"
#include "Managers/AgentManager.h"
#include "Managers/Constructor.h"
#include "../../Commander/Commander.h"
#include "Managers/Upgrader.h"
#include "Managers/ResourceManager.h"
#include "Glob.h"

using namespace BWAPI;

HatcheryAgent::HatcheryAgent(Unit m_unit) {
  unit_ = m_unit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();

  has_sent_workers_ = false;
  if (rnp::agent_manager()->get_bases_count() == 0) {
    //We dont do this for the first base.
    has_sent_workers_ = true;
  }
  if (m_unit->getType().getID() != UnitTypes::Zerg_Hatchery.getID()) {
    //Upgrade. Dont send workers.
    has_sent_workers_ = true;
  }

  agent_type_ = "HatcheryAgent";
  Constructor::modify([](Constructor* c) { c->command_center_built(); });
}

void HatcheryAgent::tick() {
  if (not has_sent_workers_) {
    if (not unit_->isBeingConstructed()) {
      send_workers();
      has_sent_workers_ = true;
      Constructor::modify([](Constructor* c) { c->add_refinery(); });
    }
  }

  if (not unit_->isIdle()) return;

  //Check for base upgrades
  if (is_of_type(UnitTypes::Zerg_Hatchery) 
      && rnp::agent_manager()->get_units_of_type_count(UnitTypes::Zerg_Lair) == 0) {
    if (Broodwar->canMake(UnitTypes::Zerg_Lair, unit_)) {
      if (rnp::resources()->hasResources(UnitTypes::Zerg_Lair)) {
        rnp::resources()->lockResources(UnitTypes::Zerg_Lair);
        unit_->morph(UnitTypes::Zerg_Lair);
        return;
      }
    }
  }

  if (is_of_type(UnitTypes::Zerg_Lair) 
      && rnp::agent_manager()->get_units_of_type_count(UnitTypes::Zerg_Hive) == 0) {
    if (Broodwar->canMake(UnitTypes::Zerg_Hive, unit_)) {
      if (rnp::resources()->hasResources(UnitTypes::Zerg_Hive)) {
        rnp::resources()->lockResources(UnitTypes::Zerg_Hive);
        unit_->morph(UnitTypes::Zerg_Hive);
        return;
      }
    }
  }

  //Build Overlords for supply
  bool build_overlord = false;
  int tot_supply = Broodwar->self()->supplyTotal() / 2;
  int c_supply = Broodwar->self()->supplyUsed() / 2;
  if (c_supply >= tot_supply - 2) {
    auto overlords_in_prod = rnp::constructor()->get_in_production_count(UnitTypes::Zerg_Overlord);
    if (overlords_in_prod == 0) build_overlord = true;
  }
  if (build_overlord) {
    if (can_build(UnitTypes::Zerg_Overlord)) {
      unit_->train(UnitTypes::Zerg_Overlord);
      return;
    }
  }

  //Build units
  if (check_build_unit(UnitTypes::Zerg_Queen)) return;
  if (check_build_unit(UnitTypes::Zerg_Mutalisk)) return;
  if (check_build_unit(UnitTypes::Zerg_Hydralisk)) return;
  if (check_build_unit(UnitTypes::Zerg_Zergling)) return;
  if (check_build_unit(UnitTypes::Zerg_Defiler)) return;
  if (check_build_unit(UnitTypes::Zerg_Ultralisk)) return;
  if (check_build_unit(UnitTypes::Zerg_Scourge)) return;

  //Create workers
  auto wtype = Broodwar->self()->getRace().getWorker();
  auto worker_count = rnp::agent_manager()->get_units_of_type_count(wtype);
  if (worker_count < rnp::commander()->get_preferred_workers_count()) {
    UnitType worker = Broodwar->self()->getRace().getWorker();
    if (can_build(worker)) {
      unit_->train(worker);
    }
  }

  //Check for upgrades
  rnp::upgrader()->check_upgrade(this);
}

bool HatcheryAgent::check_build_unit(UnitType type) {
  if (can_evolve_unit(type)) {
    unit_->train(type);
    return true;
  }
  return false;
}
