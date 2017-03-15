#include "Commander/Commander.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include "Influencemap/MapManager.h"
#include "Managers/Constructor.h"
#include "Managers/Upgrader.h"
#include "MainAgents/WorkerAgent.h"

#include <algorithm>
#include <iso646.h>

#include "BWEMUtil.h"
#include "Glob.h"

using namespace BWAPI;

Commander::Commander() {
  last_call_frame_ = Broodwar->getFrameCount();
  workers_per_refinery_ = 2;
  workers_num_ = 5;
}

Commander::~Commander() {
}

void Commander::checkBuildplan() {
  int cSupply = Broodwar->self()->supplyUsed() / 2;

  for (int i = 0; i < (int)buildplan_.size(); i++) {
    if (cSupply >= buildplan_.at(i).supply && rnp::constructor()->buildPlanLength() == 0) {
      if (buildplan_.at(i).type == BuildplanEntry::BUILDING) {
        rnp::constructor()->addBuilding(buildplan_.at(i).unittype);
        buildplan_.erase(buildplan_.begin() + i);
        i--;
      }
      else if (buildplan_.at(i).type == BuildplanEntry::UPGRADE) {
        rnp::upgrader()->addUpgrade(buildplan_.at(i).upgradetype);
        buildplan_.erase(buildplan_.begin() + i);
        i--;
      }
      else if (buildplan_.at(i).type == BuildplanEntry::TECH) {
        rnp::upgrader()->addTech(buildplan_.at(i).techtype);
        buildplan_.erase(buildplan_.begin() + i);
        i--;
      }
    }
  }
}

void Commander::cutWorkers() {
  workers_num_ = rnp::agent_manager()->getNoWorkers();
  Broodwar << "Worker production halted" << std::endl;
}

int Commander::getNoWorkers() const {
  return workers_num_;
}

int Commander::getWorkersPerRefinery() const {
  return workers_per_refinery_;
}

bool Commander::shallEngage() {
  TilePosition toAttack = findAttackPosition();
  if (toAttack.x == -1) {
    //No enemy sighted. Dont launch attack.
    return false;
  }

  for (auto& s : squads_) {
    if (s->isRequired() && !s->isActive()) {
      return false;
    }
  }
  return true;
}

void Commander::updateGoals() {
  TilePosition defSpot = findChokePoint();

  if (defSpot.x != -1) {
    for (auto& s : squads_) {
      s->defend(defSpot);
    }
  }
}

void Commander::debug_showGoal() {
  for (auto& s : squads_) {
    s->debug_showGoal();
  }
}

void Commander::computeActionsBase() {
  checkBuildplan();

  //Dont call too often
  int cFrame = Broodwar->getFrameCount();
  if (cFrame - last_call_frame_ < 5) {
    return;
  }
  last_call_frame_ = cFrame;

  //See if we need to assist a base or worker that is under attack
  if (assistBuilding()) return;
  if (assistWorker()) return;

  //Check if we shall launch an attack
  if (current_state_ == State::DEFEND) {
    if (shallEngage()) {
      forceAttack();
    }
  }

  //Check if we shall go back to defend
  if (current_state_ == State::ATTACK) {
    bool activeFound = false;
    for (auto& s : squads_) {
      if (s->isRequired() && s->isActive()) {
        activeFound = true;
      }
    }

    //No active required squads found.
    //Go back to defend.
    if (not activeFound) {
      current_state_ = State::DEFEND;
      TilePosition defSpot = findChokePoint();
      for (auto& s : squads_) {
        s->setGoal(defSpot);
      }
    }
  }

  if (current_state_ == State::DEFEND) {
    //Check if we need to attack/kite enemy workers in the base
    checkWorkersAttack(rnp::agent_manager()->getClosestBase(Broodwar->self()->getStartLocation()));

    TilePosition defSpot = findChokePoint();
    for (auto& s : squads_) {
      if (not s->hasGoal()) {
        if (defSpot.x != -1) {
          s->defend(defSpot);
        }
      }
    }
  }

  if (current_state_ == State::ATTACK) {
    for (auto& s : squads_) {
      if (s->isOffensive()) {
        if (not s->hasGoal() && s->isActive()) {
          TilePosition toAttack = findAttackPosition();
          if (toAttack.x >= 0) {
            s->attack(toAttack);
          }
        }
      }
      else {
        TilePosition defSpot = findChokePoint();
        if (defSpot.x != -1) {
          s->defend(defSpot);
        }
      }
    }
  }

  //Compute Squad actions.
  for (auto& sq : squads_) {
    sq->computeActions();
  }

  //Attack if we have filled all supply spots
  if (current_state_ == State::DEFEND) {
    int supplyUsed = Broodwar->self()->supplyUsed() / 2;
    if (supplyUsed >= 198) {
      forceAttack();
    }
  }

  //Check if there are obstacles we can remove. Needed for some maps.
  checkRemovableObstacles();

  //Terran only: Check for repairs and finish unfinished buildings
  if (Constructor::isTerran()) {
    //Check if there are unfinished buildings we need
    //to complete.
    checkDamagedBuildings();
  }

  //Check for units not belonging to a squad
  checkNoSquadUnits();
}

void Commander::checkNoSquadUnits() {
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    bool notAssigned = true;
    if (not a->isAlive()) notAssigned = false;
    if (a->getUnitType().isWorker()) notAssigned = false;
    if (a->isOfType(UnitTypes::Zerg_Overlord)) notAssigned = false;
    if (a->getUnitType().isBuilding()) notAssigned = false;
    if (a->getUnitType().isAddon()) notAssigned = false;
    if (a->getSquadID() != -1) notAssigned = false;

    if (notAssigned) {
      assignUnit(a);
    }
  }
}

void Commander::assignUnit(BaseAgent* agent) {
  //Broodwar << agent->getUnitID() << " (" << agent->getTypeName().c_str() << ") is not assigned to a squad" << endl;
  for (auto& s : squads_) {
    if (s->needUnit(agent->getUnitType())) {
      s->addMember(agent);
      Broodwar << agent->getUnitID() << " (" << agent->getTypeName().c_str() << ") is assigned to SQ " << s->getID() << std::endl;;
      return;
    }
  }
}

TilePosition Commander::findAttackPosition() const {
  TilePosition regionPos = rnp::map_manager()->findAttackPosition();
  if (regionPos.x != -1) {
    TilePosition toAttack = rnp::exploration()->getClosestSpottedBuilding(regionPos);
    if (toAttack.x != -1) {
      return toAttack;
    }
    else {
      return regionPos;
    }
  }

  return TilePosition(-1, -1);
}

void Commander::removeSquad(int id) {
  for (int i = 0; i < (int)squads_.size(); i++) {
    auto& sq = squads_.at(i);
    if (sq->getID() == id) {
      sq->disband();
      squads_.erase(squads_.begin() + i);
      return;
    }
  }
}

Squad::Ptr Commander::getSquad(int id) {
  for (auto& s : squads_) {
    if (s->getID() == id) {
      return s;
    }
  }
  return nullptr;
}

void Commander::addSquad(Squad::Ptr sq) {
  squads_.push_back(sq);
}

void Commander::unitDestroyed(BaseAgent* agent) {
  int squadID = agent->getSquadID();
  if (squadID != -1) {
    auto squad = getSquad(squadID);
    if (squad) {
      squad->removeMember(agent);
    }
  }
}

void Commander::sortSquadList() {
  sort(squads_.begin(), squads_.end(), SortSquadList());
}

void Commander::unitCreated(BaseAgent* agent) {
  //Sort the squad list
  sortSquadList();

  for (auto& s : squads_) {
    if (s->addMember(agent)) {
      break;
    }
  }
}

bool Commander::checkWorkersAttack(BaseAgent* base) const {
  int noAttack = 0;

  for (auto& u : Broodwar->enemy()->getUnits()) {
    if (u->exists() && u->getType().isWorker()) {
      double dist = u->getTilePosition().getDistance(base->getUnit()->getTilePosition());
      if (dist <= 12) {
        //Enemy unit discovered. Attack with some workers.
        auto& agents = rnp::agent_manager()->getAgents();
        for (auto& a : agents) {
          if (a->isAlive() && a->isWorker() && noAttack < 1) {
            WorkerAgent* wAgent = static_cast<WorkerAgent*>(a);
            wAgent->setState(WorkerAgent::ATTACKING);
            a->getUnit()->attack(u);
            noAttack++;
          }
        }
      }
    }
  }

  if (noAttack > 0) {
    return true;
  }
  return false;
}

void Commander::checkRemovableObstacles() {
  if (removal_done_) return;

  //This method is used to handle the removal of obstacles
  //that is needed on some maps.

  if (Broodwar->mapFileName() == "(2)Destination.scx") {
    Unit mineral = nullptr;
    if (Broodwar->self()->getStartLocation().x == 64) {
      for (auto& u : Broodwar->getAllUnits()) {
        if (u->getType().isResourceContainer() && u->getTilePosition().x == 40 && u->getTilePosition().y == 120) {
          mineral = u;
        }
      }
    }
    if (Broodwar->self()->getStartLocation().x == 31) {
      for (auto& u : Broodwar->getAllUnits()) {
        if (u->getType().isResourceContainer() && u->getTilePosition().x == 54 && u->getTilePosition().y == 6) {
          mineral = u;
        }
      }
    }
    if (mineral != nullptr) {
      if (not rnp::agent_manager()->workerIsTargeting(mineral)) {
        BaseAgent* worker = rnp::agent_manager()->findClosestFreeWorker(Broodwar->self()->getStartLocation());
        if (worker != nullptr) {
          worker->getUnit()->rightClick(mineral);
          removal_done_ = true;
        }
      }
    }
  }
}


TilePosition Commander::findChokePoint() {
  const BWEM::ChokePoint* bestChoke = rnp::map_manager()->getDefenseLocation();

  TilePosition guardPos = Broodwar->self()->getStartLocation();
  if (bestChoke != nullptr) {
    guardPos = findDefensePos(bestChoke);
  }

  return guardPos;
}

TilePosition Commander::findDefensePos(const BWEM::ChokePoint* choke) const {
  TilePosition defPos = TilePosition(choke->Center());
  TilePosition chokePos = defPos;

  double size = rnp::choke_width(choke);
  if (size <= 32 * 3) {
    //Very narrow chokepoint, dont crowd it
    double bestDist = 10000;
    TilePosition basePos = Broodwar->self()->getStartLocation();

    int maxD = 3;
    int minD = 2;

    //We found a chokepoint. Now we need to find a good place to defend it.
    for (int cX = chokePos.x - maxD; cX <= chokePos.x + maxD; cX++) {
      for (int cY = chokePos.y - maxD; cY <= chokePos.y + maxD; cY++) {
        TilePosition cPos = TilePosition(cX, cY);
        if (ExplorationManager::canReach(basePos, cPos)) {
          double chokeDist = chokePos.getDistance(cPos);
          double baseDist = basePos.getDistance(cPos);

          if (chokeDist >= minD && chokeDist <= maxD) {
            if (baseDist < bestDist) {
              bestDist = baseDist;
              defPos = cPos;
            }
          }
        }
      }
    }
  }

  //Make defenders crowd around defensive structures.
  if (Broodwar->self()->getRace().getID() == Races::Zerg.getID()) {
    UnitType defType;
    if (Constructor::isZerg()) defType = UnitTypes::Zerg_Sunken_Colony;
    if (Constructor::isProtoss()) defType = UnitTypes::Protoss_Photon_Cannon;
    if (Constructor::isTerran()) defType = UnitTypes::Terran_Bunker;

    BaseAgent* turret = rnp::agent_manager()->getClosestAgent(defPos, defType);
    if (turret != nullptr) {
      TilePosition tPos = turret->getUnit()->getTilePosition();
      double dist = tPos.getDistance(defPos);
      if (dist <= 22) {
        defPos = tPos;
      }
    }
  }

  return defPos;
}

bool Commander::needUnit(UnitType type) {
  int prevPrio = 1000;

  for (auto& s : squads_) {
    if (not s->isFull()) {
      if (s->getPriority() > prevPrio) {
        return false;
      }

      if (s->needUnit(type)) {
        return true;
      }

      prevPrio = s->getPriority();
    }
  }
  return false;
}

bool Commander::assistBuilding() {
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive() && a->isBuilding() && a->isUnderAttack()) {
      for (auto& s : squads_) {
        bool ok = true;
        if (s->isExplorer()) ok = false;
        if (s->isBunkerDefend()) ok = false;
        if (s->isRush() && s->isActive()) ok = false;

        if (ok) {
          s->assist(a->getUnit()->getTilePosition());
          return true;
        }
      }
    }
  }

  return false;
}

bool Commander::assistWorker() {
  for (auto& a : rnp::agent_manager()->getAgents()) {
    if (a->isAlive() && a->isWorker() && a->isUnderAttack()) {
      for (auto& s : squads_) {
        bool ok = true;
        if (s->isExplorer()) ok = false;
        if (s->isBunkerDefend()) ok = false;
        if (s->isRush() && s->isActive()) ok = false;

        if (ok) {
          s->assist(a->getUnit()->getTilePosition());
          return true;
        }
      }
    }
  }

  return false;
}

void Commander::forceAttack() {
  TilePosition cGoal = findAttackPosition();
  Broodwar << "Launch attack at (" << cGoal.x << "," << cGoal.y << ")" << std::endl;
  if (cGoal.x == -1) {
    return;
  }

  for (auto& s : squads_) {
    if (s->isOffensive() || s->isSupport()) {
      if (cGoal.x >= 0) {
        s->forceActive();
        s->attack(cGoal);
      }
    }
  }

  current_state_ = State::ATTACK;
}

void Commander::finishBuild(BaseAgent* baseAgent) {
  //First we must check if someone is repairing this building
  if (rnp::agent_manager()->isAnyAgentRepairingThisAgent(baseAgent)) return;

  BaseAgent* repUnit = rnp::agent_manager()->findClosestFreeWorker(baseAgent->getUnit()->getTilePosition());
  if (repUnit != nullptr) {
    WorkerAgent* w = (WorkerAgent*)repUnit;
    w->assignToRepair(baseAgent->getUnit());
  }
}

bool Commander::checkDamagedBuildings() {
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive() && a->isBuilding() && a->isDamaged()) {
      Unit builder = a->getUnit()->getBuildUnit();
      if (builder == nullptr || !builder->isConstructing()) {
        finishBuild(a);
      }
    }
  }
  return false;
}

void Commander::toggleBuildplanDebug() {
  debug_bp_ = !debug_bp_;
}

void Commander::toggleSquadsDebug() {
  debug_sq_ = !debug_sq_;
}

std::string Commander::format(std::string str) {
  std::string res = str;

  std::string raceName = Broodwar->self()->getRace().getName();
  if (str.find(raceName) == 0) {
    int i = str.find("_");
    res = str.substr(i + 1, str.length());
  }

  if (res == "Siege Tank Tank Mode") res = "Siege Tank";

  return res;
}

void Commander::printInfo() {
  if (debug_sq_) {
    int totLines = 0;
    for (auto& s : squads_) {
      bool vis = true;
      if (s->getTotalUnits() == 0) vis = false;
      if (s->isBunkerDefend()) vis = false;
      if (s->getPriority() == 1000 && !s->isActive()) vis = false;

      if (vis) {
        totLines++;
      }
    }
    if (totLines == 0) totLines++;

    Broodwar->drawBoxScreen(168, 25, 292, 41 + totLines * 16, Colors::Black, true);
    if (current_state_ == State::DEFEND) Broodwar->drawTextScreen(170, 25, "\x03Squads \x07(Defending)");
    if (current_state_ == State::ATTACK) Broodwar->drawTextScreen(170, 25, "\x03Squads \x08(Attacking)");
    Broodwar->drawLineScreen(170, 39, 290, 39, Colors::Orange);
    int no = 0;
    for (auto& s : squads_) {
      bool vis = true;
      if (s->getTotalUnits() == 0) vis = false;
      if (s->isBunkerDefend()) vis = false;
      if (s->getPriority() == 1000 && !s->isActive()) vis = false;

      if (vis) {
        int cSize = s->getSize();
        int totSize = s->getTotalUnits();

        if (s->isRequired()) {
          if (cSize < totSize) Broodwar->drawTextScreen(170, 41 + no * 16, "*SQ %d: \x18(%d/%d)", s->getID(), s->getSize(), s->getTotalUnits());
          else Broodwar->drawTextScreen(170, 41 + no * 16, "*SQ %d: \x07(%d/%d)", s->getID(), s->getSize(), s->getTotalUnits());
          no++;
        }
        else {
          if (cSize < totSize) Broodwar->drawTextScreen(170, 41 + no * 16, "SQ %d: \x18(%d/%d)", s->getID(), s->getSize(), s->getTotalUnits());
          else Broodwar->drawTextScreen(170, 41 + no * 16, "SQ %d: \x07(%d/%d)", s->getID(), s->getSize(), s->getTotalUnits());
          no++;
        }
      }
    }
    if (no == 0) no++;
    Broodwar->drawLineScreen(170, 40 + no * 16, 290, 40 + no * 16, Colors::Orange);
  }

  if (debug_bp_ && buildplan_.size() > 0) {
    int totLines = (int)buildplan_.size();
    if (totLines > 4) totLines = 4;
    if (totLines == 0) totLines = 1;

    Broodwar->drawBoxScreen(298, 25, 482, 41 + totLines * 16, Colors::Black, true);
    Broodwar->drawTextScreen(300, 25, "\x03Strategy Plan");
    Broodwar->drawLineScreen(300, 39, 480, 39, Colors::Orange);
    int no = 0;

    int max = (int)buildplan_.size();
    if (max > 4) max = 4;

    for (int i = 0; i < max; i++) {
      std::string name = "";
      if (buildplan_.at(i).type == BuildplanEntry::BUILDING) name = buildplan_.at(i).unittype.getName();
      if (buildplan_.at(i).type == BuildplanEntry::UPGRADE) name = buildplan_.at(i).upgradetype.getName();
      if (buildplan_.at(i).type == BuildplanEntry::TECH) name = buildplan_.at(i).techtype.getName();
      name = format(name);

      std::stringstream ss;
      ss << name;
      ss << " \x0F(@";
      ss << buildplan_.at(i).supply;
      ss << ")";

      Broodwar->drawTextScreen(300, 40 + no * 16, ss.str().c_str());
      no++;
    }
    if (no == 0) no++;
    Broodwar->drawLineScreen(300, 40 + no * 16, 480, 40 + no * 16, Colors::Orange);
    rnp::constructor()->printInfo();
  }
}

int Commander::addBunkerSquad() {
  auto bSquad = std::make_shared<Squad>(
    100 + rnp::agent_manager()->countNoUnits(UnitTypes::Terran_Bunker), 
    Squad::SquadType::BUNKER, 
    "BunkerSquad", 
    5);
  bSquad->addSetup(UnitTypes::Terran_Marine, 4);
  squads_.push_back(bSquad);

  //Try to fill from other squads.
  int added = 0;
  for (auto& s : squads_) {
    if (s->isOffensive() || s->isDefensive()) {
      for (int i = 0; i < 4 - added; i++) {
        if (s->hasUnits(UnitTypes::Terran_Marine, 1)) {
          if (added < 4) {
            BaseAgent* ma = s->removeMember(UnitTypes::Terran_Marine);
            if (ma != nullptr) {
              added++;
              bSquad->addMember(ma);
              ma->clearGoal();
            }
          }
        }
      }
    }
  }

  return bSquad->getID();
}

bool Commander::removeBunkerSquad(int unitID) {
  for (auto& s : squads_) {
    if (s->isBunkerDefend()) {
      if (s->getBunkerID() == unitID) {
        int sID = s->getID();
        removeSquad(sID);
        return true;
      }
    }
  }
  return false;
}
