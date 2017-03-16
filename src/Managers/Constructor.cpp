#include "Constructor.h"
#include "StructureAgents/StructureAgent.h"
#include "MainAgents/BaseAgent.h"
#include "MainAgents/WorkerAgent.h"
#include "AgentManager.h"
#include "BuildingPlacer.h"
#include "ResourceManager.h"
#include "Glob.h"
#include "RnpUtil.h"

using namespace BWAPI;

Constructor::Constructor(): build_plan_(), build_queue_() {
  last_call_frame_ = Broodwar->getFrameCount();
}

Constructor::~Constructor() {
}

int Constructor::buildPlanLength() const {
  return (int)build_plan_.size();
}

void Constructor::buildingDestroyed(Unit building) {
  if (building->getType().getID() == UnitTypes::Protoss_Pylon.getID()) {
    return;
  }
  if (building->getType().getID() == UnitTypes::Terran_Supply_Depot.getID()) {
    return;
  }
  if (building->getType().isAddon()) {
    return;
  }
  if (building->getType().getID() == UnitTypes::Zerg_Sunken_Colony.getID()) {
    build_plan_.insert(build_plan_.begin(), UnitTypes::Zerg_Spore_Colony);
    return;
  }
  build_plan_.insert(build_plan_.begin(), building->getType());
}

void Constructor::computeActions() {
  //Check if we need more supply buildings
  if (isTerran() || isProtoss()) {
    if (shallBuildSupply()) {
      build_plan_.insert(build_plan_.begin(), Broodwar->self()->getRace().getSupplyProvider());
    }
  }

  //Check if we need to expand
  if (not hasResourcesLeft()) {
    expand(Broodwar->self()->getRace().getCenter());
  }

  if (build_plan_.size() == 0 && build_queue_.size() == 0) {
    //Nothing to do
    return;
  }

  //Dont call too often
  int cFrame = Broodwar->getFrameCount();
  if (cFrame - last_call_frame_ < 10) {
    return;
  }
  last_call_frame_ = cFrame;

  if (rnp::agent_manager()->getNoWorkers() == 0) {
    //No workers so cant do anything
    return;
  }

  //Check if we have possible "locked" items in the buildqueue
  for (int i = 0; i < (int)build_queue_.size(); i++) {
    int elapsed = cFrame - build_queue_.at(i).assigned_frame_;
    if (elapsed >= 2000) {
      //Reset the build request
      WorkerAgent* worker = (WorkerAgent*)rnp::agent_manager()->getAgent(build_queue_.at(i).assigned_worker_id_);
      if (worker != nullptr) {
        worker->reset();
      }
      build_plan_.insert(build_plan_.begin(), build_queue_.at(i).to_build_);
      rnp::resources()->unlockResources(build_queue_.at(i).to_build_);
      build_queue_.erase(build_queue_.begin() + i);
      return;
    }
  }

  //Check if we can build next building in the buildplan
  if ((int)build_plan_.size() > 0) {
    executeOrder(build_plan_.at(0));
  }
}

bool Constructor::hasResourcesLeft() {
  int totalMineralsLeft = 0;

  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->getUnitType().isResourceDepot()) {
      totalMineralsLeft += mineralsNearby(a->getUnit()->getTilePosition());
    }
  }

  if (totalMineralsLeft <= 5000) {
    return false;
  }
  return true;
}

int Constructor::mineralsNearby(TilePosition center) {
  int mineralCnt = 0;

  for (auto& u : Broodwar->getMinerals()) {
    if (u->exists()) {
      double dist = center.getDistance(u->getTilePosition());
      if (dist <= 10) {
        mineralCnt += u->getResources();
      }
    }
  }

  return mineralCnt;
}

bool Constructor::shallBuildSupply() {
  UnitType supply = Broodwar->self()->getRace().getSupplyProvider();

  //Check if we need supplies
  int supplyTotal = Broodwar->self()->supplyTotal() / 2;
  int supplyUsed = Broodwar->self()->supplyUsed() / 2;

  int preDiff = 2;
  //Speed up supply production in middle/late game
  if (supplyUsed > 30) preDiff = 4;

  if (supplyUsed <= supplyTotal - preDiff) {
    return false;
  }
  //Don't use automatic supply adding in the early game
  //to make it a bit more controlled.
  if (supplyUsed <= 30) {
    return false;
  }

  //Check if we have reached max supply
  if (supplyTotal >= 200) {
    return false;
  }

  //Check if there aready is a supply in the list
  if (nextIsOfType(supply)) {
    return false;
  }

  //Check if we are already building a supply
  if (supplyBeingBuilt()) {
    return false;
  }

  return true;
}

bool Constructor::supplyBeingBuilt() {
  //Zerg
  if (isZerg()) {
    if (noInProduction(UnitTypes::Zerg_Overlord) > 0) {
      return true;
    }
    else {
      return false;
    }
  }

  //Terran and Protoss
  UnitType supply = Broodwar->self()->getRace().getSupplyProvider();

  //1. Check if we are already building a supply
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive()) {
      if (a->getUnitType().getID() == supply.getID()) {
        if (a->getUnit()->isBeingConstructed()) {
          //Found one that is being constructed
          return true;
        }
      }
    }
  }

  //2. Check if we have a supply in build queue
  for (int i = 0; i < (int)build_queue_.size(); i++) {
    if (build_queue_.at(i).to_build_.getID() == supply.getID()) {
      return true;
    }
  }

  return false;
}

void Constructor::lock(int buildPlanIndex, int unitId) {
  UnitType type = build_plan_.at(buildPlanIndex);
  build_plan_.erase(build_plan_.begin() + buildPlanIndex);

  BuildQueueItem item;
  item.to_build_ = type;
  item.assigned_worker_id_ = unitId;
  item.assigned_frame_ = Broodwar->getFrameCount();

  build_queue_.push_back(item);
}

void Constructor::remove(UnitType type) {
  for (int i = 0; i < (int)build_plan_.size(); i++) {
    if (build_plan_.at(i).getID() == type.getID()) {
      build_plan_.erase(build_plan_.begin() + i);
      return;
    }
  }
}

void Constructor::unlock(UnitType type) {
  for (int i = 0; i < (int)build_queue_.size(); i++) {
    if (build_queue_.at(i).to_build_.getID() == type.getID()) {
      build_queue_.erase(build_queue_.begin() + i);
      return;
    }
  }
}

void Constructor::handleWorkerDestroyed(UnitType type, int workerID) {
  for (int i = 0; i < (int)build_queue_.size(); i++) {
    if (build_queue_.at(i).assigned_worker_id_ == workerID) {
      build_queue_.erase(build_queue_.begin() + i);
      build_plan_.insert(build_plan_.begin(), type);
      rnp::resources()->unlockResources(type);
    }
  }
}

bool Constructor::executeMorph(UnitType target, UnitType evolved) {
  BaseAgent* agent = rnp::agent_manager()->getClosestAgent(Broodwar->self()->getStartLocation(), target);
  if (agent != nullptr) {
    StructureAgent* sAgent = (StructureAgent*)agent;
    if (sAgent->canMorphInto(evolved)) {
      sAgent->getUnit()->morph(evolved);
      lock(0, sAgent->getUnitID());
      return true;
    }
  }
  else {
    //No building available that can do this morph.
    remove(evolved);
  }
  return false;
}

bool Constructor::executeOrder(UnitType type) {
  //Max 5 concurrent buildings allowed at the same time
  if ((int)build_queue_.size() >= 5) {
    return false;
  }

  //Check if we meet requirements for the building
  std::map<UnitType, int> reqs = type.requiredUnits();
  for (std::map<UnitType, int>::iterator j = reqs.begin(); j != reqs.end(); j++) {
    if (not rnp::agent_manager()->hasBuilding((*j).first)) {
      return false;
    }
  }

  if (type.isResourceDepot()) {
    TilePosition pos = rnp::building_placer()->find_expansion_site();
    if (not rnp::is_valid_position(pos)) {
      //No expansion site found.
      if ((int)build_plan_.size() > 0) build_plan_.erase(build_plan_.begin());
      return true;
    }
  }
  if (type.isRefinery()) {
    TilePosition rSpot = rnp::building_placer()->search_refinery_spot();
    if (rSpot.x < 0) {
      //No buildspot found
      if ((int)build_plan_.size() > 0) build_plan_.erase(build_plan_.begin());
      return true;
    }
  }
  if (isZerg()) {
    std::pair<UnitType, int> builder = type.whatBuilds();
    if (builder.first.getID() != UnitTypes::Zerg_Drone.getID()) {
      //Needs to be morphed
      if (executeMorph(builder.first, type)) {
        return true;
      }
      else {
        return false;
      }
    }
  }

  //Check if we have resources
  if (not rnp::resources()->hasResources(type)) {
    return false;
  }

  //Check if we have a free worker
  bool found = false;
  BaseAgent* a = rnp::agent_manager()->findClosestFreeWorker(Broodwar->self()->getStartLocation());
  if (a != nullptr) {
    WorkerAgent* w = (WorkerAgent*)a;
    found = true;
    if (w->assignToBuild(type)) {
      lock(0, a->getUnitID());
      return true;
    }
    else {
      //Unable to find a buildspot. Dont bother checking for all
      //other workers
      handleNoBuildspotFound(type);
      return false;
    }
  }

  return false;
}

bool Constructor::isTerran() {
  if (Broodwar->self()->getRace().getID() == Races::Terran.getID()) {
    return true;
  }
  return false;
}

bool Constructor::isProtoss() {
  if (Broodwar->self()->getRace().getID() == Races::Protoss.getID()) {
    return true;
  }
  return false;
}

bool Constructor::isZerg() {
  if (Broodwar->self()->getRace().getID() == Races::Zerg.getID()) {
    return true;
  }
  return false;
}

bool Constructor::nextIsExpand() {
  if ((int)build_plan_.size() > 0) {
    if (build_plan_.at(0).isResourceDepot()) return true;
  }
  return false;
}

void Constructor::addRefinery() {
  //Don't add if we already have enough.
  UnitType ref = Broodwar->self()->getRace().getRefinery();
  int no = rnp::agent_manager()->countNoUnits(ref);
  if (no >= 4) return;

  UnitType refinery = Broodwar->self()->getRace().getRefinery();

  if (not this->nextIsOfType(refinery)) {
    build_plan_.insert(build_plan_.begin(), refinery);
  }
}

void Constructor::commandCenterBuilt() {
  last_command_center_ = Broodwar->getFrameCount();
}

std::string Constructor::format(UnitType type) {
  std::string name = type.getName();
  int i = name.find("_");
  std::string fname = name.substr(i + 1, name.length());
  return fname;
}

void Constructor::printInfo() {
  int totLines = (int)build_plan_.size() + (int)build_queue_.size();
  if (build_plan_.size() == 0) totLines++;
  if (build_queue_.size() == 0) totLines++;

  if (totLines > 0) {
    Broodwar->drawBoxScreen(488, 25, 602, 62 + totLines * 16, Colors::Black, true);
    Broodwar->drawTextScreen(490, 25, "\x03Next to build");
    Broodwar->drawLineScreen(490, 39, 600, 39, Colors::Orange);

    int no = 0;
    for (int i = 0; i < (int)build_plan_.size(); i++) {
      Broodwar->drawTextScreen(490, 40 + no * 16, format(build_plan_.at(i)).c_str());
      no++;
    }
    if (no == 0) no++;
    Broodwar->drawLineScreen(490, 40 + no * 16, 600, 40 + no * 16, Colors::Orange);

    int s = 40 + no * 16;
    Broodwar->drawTextScreen(490, s + 2, "\x03In progress");
    Broodwar->drawLineScreen(490, s + 19, 600, s + 19, Colors::Orange);

    no = 0;
    for (int i = 0; i < (int)build_queue_.size(); i++) {
      Broodwar->drawTextScreen(490, s + 20 + no * 16, format(build_queue_.at(i).to_build_).c_str());
      no++;
    }
    if (no == 0) no++;
    Broodwar->drawLineScreen(490, s + 20 + no * 16, 600, s + 20 + no * 16, Colors::Orange);
  }
}

void Constructor::handleNoBuildspotFound(UnitType toBuild) {
  bool removeOrder = false;
  if (toBuild.getID() == UnitTypes::Protoss_Photon_Cannon) removeOrder = true;
  if (toBuild.getID() == UnitTypes::Terran_Missile_Turret) removeOrder = true;
  if (toBuild.isAddon()) removeOrder = true;
  if (toBuild.getID() == UnitTypes::Zerg_Spore_Colony) removeOrder = true;
  if (toBuild.getID() == UnitTypes::Zerg_Sunken_Colony) removeOrder = true;
  if (toBuild.isResourceDepot()) removeOrder = true;
  if (toBuild.isRefinery()) removeOrder = true;

  if (removeOrder) {
    remove(toBuild);
  }

  if (not removeOrder) {
    if (isProtoss() && not supplyBeingBuilt()) {
      //Insert a pylon to increase PSI coverage
      if (not nextIsOfType(UnitTypes::Protoss_Pylon)) {
        build_plan_.insert(build_plan_.begin(), UnitTypes::Protoss_Pylon);
      }
    }
  }
}

bool Constructor::nextIsOfType(UnitType type) {
  if ((int)build_plan_.size() == 0) {
    return false;
  }
  else {
    if (build_plan_.at(0).getID() == type.getID()) {
      return true;
    }
  }
  return false;
}

bool Constructor::containsType(UnitType type) {
  for (int i = 0; i < (int)build_plan_.size(); i++) {
    if (build_plan_.at(i).getID() == type.getID()) {
      return true;
    }
  }
  for (int i = 0; i < (int)build_queue_.size(); i++) {
    if (build_queue_.at(i).to_build_.getID() == type.getID()) {
      return true;
    }
  }
  return false;
}

bool Constructor::coveredByDetector(TilePosition pos) {
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive()) {
      UnitType type = a->getUnitType();
      if (type.isDetector() && type.isBuilding()) {
        double range = type.sightRange() * 1.5;
        double dist = a->getUnit()->getPosition().getDistance(Position(pos));
        if (dist <= range) {
          return true;
        }
      }
    }
  }
  return false;
}

void Constructor::addBuilding(UnitType type) {
  build_plan_.push_back(type);
}

void Constructor::addBuildingFirst(UnitType type) {
  build_plan_.insert(build_plan_.begin(), type);
}

void Constructor::expand(UnitType commandCenterUnit) {
  if (isBeingBuilt(commandCenterUnit)) {
    return;
  }

  if (containsType(commandCenterUnit)) {
    return;
  }

  TilePosition pos = rnp::building_placer()->find_expansion_site();
  if (not rnp::is_valid_position(pos)) {
    //No expansion site found.
    return;
  }

  build_plan_.insert(build_plan_.begin(), commandCenterUnit);
}

bool Constructor::needBuilding(UnitType type) {
  if (rnp::agent_manager()->hasBuilding(type)) return false;
  if (isBeingBuilt(type)) return false;
  if (containsType(type)) return false;

  return true;
}

bool Constructor::isBeingBuilt(UnitType type) {
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isOfType(type) && a->getUnit()->isBeingConstructed()) {
      return true;
    }
  }
  return false;
}

int Constructor::noInProduction(UnitType type) {
  int no = 0;

  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive()) {
      if (a->getUnitType().canProduce() && not a->getUnit()->isBeingConstructed()) {
        for (auto& u : a->getUnit()->getTrainingQueue()) {
          if (u.getID() == type.getID()) {
            no++;
          }
        }
      }
    }
  }

  if (isZerg()) {
    for (auto& u : Broodwar->self()->getUnits()) {
      if (u->exists()) {
        if (u->getType().getID() == UnitTypes::Zerg_Egg.getID()) {
          if (u->getBuildType().getID() == type.getID()) {
            no++;
            if (type.isTwoUnitsInOneEgg()) no++;
          }
        }
      }
    }
  }

  return no;
}
