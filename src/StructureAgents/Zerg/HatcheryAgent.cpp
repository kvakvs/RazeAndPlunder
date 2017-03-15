#include "HatcheryAgent.h"
#include "Managers/AgentManager.h"
#include "Managers/Constructor.h"
#include "../../Commander/Commander.h"
#include "Managers/Upgrader.h"
#include "Managers/ResourceManager.h"
#include "Glob.h"

using namespace BWAPI;

HatcheryAgent::HatcheryAgent(Unit mUnit) {
  unit = mUnit;
  type = unit->getType();
  unitID = unit->getID();

  hasSentWorkers = false;
  if (rnp::agent_manager()->countNoBases() == 0) {
    //We dont do this for the first base.
    hasSentWorkers = true;
  }
  if (mUnit->getType().getID() != UnitTypes::Zerg_Hatchery.getID()) {
    //Upgrade. Dont send workers.
    hasSentWorkers = true;
  }

  agentType = "HatcheryAgent";
  rnp::constructor()->commandCenterBuilt();
}

void HatcheryAgent::computeActions() {
  if (not hasSentWorkers) {
    if (not unit->isBeingConstructed()) {
      sendWorkers();
      hasSentWorkers = true;
      rnp::constructor()->addRefinery();
    }
  }

  if (not unit->isIdle()) return;

  //Check for base upgrades
  if (isOfType(UnitTypes::Zerg_Hatchery) && rnp::agent_manager()->countNoUnits(UnitTypes::Zerg_Lair) == 0) {
    if (Broodwar->canMake(UnitTypes::Zerg_Lair, unit)) {
      if (rnp::resources()->hasResources(UnitTypes::Zerg_Lair)) {
        rnp::resources()->lockResources(UnitTypes::Zerg_Lair);
        unit->morph(UnitTypes::Zerg_Lair);
        return;
      }
    }
  }
  if (isOfType(UnitTypes::Zerg_Lair) && rnp::agent_manager()->countNoUnits(UnitTypes::Zerg_Hive) == 0) {
    if (Broodwar->canMake(UnitTypes::Zerg_Hive, unit)) {
      if (rnp::resources()->hasResources(UnitTypes::Zerg_Hive)) {
        rnp::resources()->lockResources(UnitTypes::Zerg_Hive);
        unit->morph(UnitTypes::Zerg_Hive);
        return;
      }
    }
  }

  //Build Overlords for supply
  bool buildOL = false;
  int totSupply = Broodwar->self()->supplyTotal() / 2;
  int cSupply = Broodwar->self()->supplyUsed() / 2;
  if (cSupply >= totSupply - 2) {
    if (rnp::constructor()->noInProduction(UnitTypes::Zerg_Overlord) == 0) buildOL = true;
  }
  if (buildOL) {
    if (canBuild(UnitTypes::Zerg_Overlord)) {
      unit->train(UnitTypes::Zerg_Overlord);
      return;
    }
  }

  //Build units
  if (checkBuildUnit(UnitTypes::Zerg_Queen)) return;
  if (checkBuildUnit(UnitTypes::Zerg_Mutalisk)) return;
  if (checkBuildUnit(UnitTypes::Zerg_Hydralisk)) return;
  if (checkBuildUnit(UnitTypes::Zerg_Zergling)) return;
  if (checkBuildUnit(UnitTypes::Zerg_Defiler)) return;
  if (checkBuildUnit(UnitTypes::Zerg_Ultralisk)) return;
  if (checkBuildUnit(UnitTypes::Zerg_Scourge)) return;

  //Create workers
  if (rnp::agent_manager()->countNoUnits(Broodwar->self()->getRace().getWorker()) < rnp::commander()->getNoWorkers()) {
    UnitType worker = Broodwar->self()->getRace().getWorker();
    if (canBuild(worker)) {
      unit->train(worker);
    }
  }

  //Check for upgrades
  rnp::upgrader()->checkUpgrade(this);
}

bool HatcheryAgent::checkBuildUnit(UnitType type) {
  if (canEvolveUnit(type)) {
    unit->train(type);
    return true;
  }
  return false;
}
