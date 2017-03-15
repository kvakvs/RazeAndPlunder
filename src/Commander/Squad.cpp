#include "Squad.h"
#include "UnitAgents/UnitAgent.h"
#include "Managers/ExplorationManager.h"
#include "Influencemap/MapManager.h"
#include "Commander/Commander.h"
#include "Managers/Constructor.h"
#include "Pathfinding/Pathfinder.h"
#include "Utils/Profiler.h"
#include "Managers/AgentManager.h"
#include "Glob.h"

using namespace BWAPI;

Squad::Squad(int mId, SquadType mType, std::string mName, int mPriority) {
  id_ = mId;
  type_ = mType;
  name_ = mName;
  active_priority_ = priority_ = mPriority;
  goal_ = TilePosition(-1, -1);
}

std::string Squad::getName() const {
  return name_;
}

const Agentset& Squad::getMembers() const {
  return agents_;
}

UnitType Squad::morphsTo() const {
  return morphs_;
}

void Squad::setMorphsTo(UnitType type) {
  morphs_ = type;
}

int Squad::getID() const {
  return id_;
}

bool Squad::isRequired() const {
  return required_;
}

void Squad::setRequired(bool mRequired) {
  required_ = mRequired;
}

void Squad::setBuildup(bool mBuildup) {
  buildup_ = mBuildup;
}

int Squad::getPriority() const {
  return priority_;
}

void Squad::setPriority(int mPriority) {
  priority_ = mPriority;
}

void Squad::setActivePriority(int mPriority) {
  active_priority_ = mPriority;
}

bool Squad::isActive() {
  return active_;
}

void Squad::forceActive() {
  active_priority_ = priority_;
  active_ = true;
}

int Squad::size() {
  int no = 0;
  if (agents_.size() == 0) return 0;
  if (agents_.empty()) return 0;

  for (auto& a : agents_) {
    if (a->isAlive()) {
      no++;
    }
  }

  return no;
}

int Squad::maxSize() {
  int no = 0;
  for (int i = 0; i < (int)setup_.size(); i++) {
    no += setup_.at(i).no;
  }
  return no;
}

void Squad::addSetup(UnitType type, int no) {
  //First, check if we have the setup already
  for (int i = 0; i < (int)setup_.size(); i++) {
    if (setup_.at(i).type.getID() == type.getID()) {
      //Found, increase the amount
      setup_.at(i).no += no;
      return;
    }
  }

  //Not found, add as new
  UnitSetup us;
  us.type = type;
  us.no = no;
  us.current = 0;
  setup_.push_back(us);

  if (not type.isFlyer()) {
    move_type_ = MoveType::GROUND;
  }
}

void Squad::removeSetup(UnitType type, int no) {
  for (int i = 0; i < (int)setup_.size(); i++) {
    if (setup_.at(i).type.getID() == type.getID()) {
      //Found, reduce the amount
      setup_.at(i).no -= no;
      if (setup_.at(i).no < 0) setup_.at(i).no = 0;
      int toRemove = setup_.at(i).current - setup_.at(i).no;
      for (int j = 0; j < toRemove; j++) {
        removeMember(setup_.at(i).type);
      }
      return;
    }
  }
}

void Squad::debug_showGoal() {
  if (isBunkerDefend()) return;

  if (size() > 0 && goal_.x >= 0) {
    Position a = Position(goal_.x * 32 + 16, goal_.y * 32 + 16);

    Broodwar->drawCircleMap(a.x - 3, a.y - 3, 6, Colors::Grey, true);
    Broodwar->drawTextMap(a.x - 20, a.y - 5, "\x03SQ %d", id_);
  }
}

void Squad::computeActions() {
  if (not active_) {
    if (isFull() && !buildup_) {
      active_ = true;
    }
  }

  if (active_) {
    if (active_priority_ != priority_) {
      priority_ = active_priority_;
    }
  }

  //Check if we need workers in the squad
  for (int i = 0; i < (int)setup_.size(); i++) {
    if (setup_.at(i).current < setup_.at(i).no && setup_.at(i).type.isWorker()) {
      int no = setup_.at(i).no - setup_.at(i).current;
      for (int j = 0; j < no; j++) {
        BaseAgent* w = rnp::agent_manager()->findClosestFreeWorker(Broodwar->self()->getStartLocation());
        if (w != nullptr) addMember(w);
      }
    }
  }
}

bool Squad::isAttacking() {
  if (isExplorer()) return false;

  try {
    for (auto& a : agents_) {
      if (a->isAlive()) {
        if (a->getUnit()->isAttacking()) return true;
        if (a->getUnit()->isStartingAttack()) return true;
      }
    }
  }
  catch (std::exception) {

  }

  return false;
}

bool Squad::isUnderAttack() {
  try {
    for (auto& a : agents_) {
      if (a->isAlive()) {
        if (a->getUnit()->isAttacking()) return true;
        if (a->getUnit()->isStartingAttack()) return true;
      }
    }
  }
  catch (std::exception) {

  }

  return false;
}

bool Squad::needUnit(UnitType type) {
  //1. Check if prio is set to Inactive squad.
  if (priority_ >= 1000) {
    return false;
  }

  int noCreated = 1;
  if (Constructor::isZerg()) {
    if (type.isTwoUnitsInOneEgg()) {
      noCreated = 2;
    }
  }

  //2. Check setup
  for (int i = 0; i < (int)setup_.size(); i++) {
    if (setup_.at(i).equals(type)) {
      //Found a matching setup, see if there is room
      if (setup_.at(i).current + rnp::constructor()->noInProduction(type) + noCreated <= setup_.at(i).no) {
        return true;
      }
    }
  }

  return false;
}

bool Squad::addMember(BaseAgent* agent) {
  if (priority_ >= 1000) {
    //Check if prio is above Inactive squad.
    return false;
  }

  //Step 1. Check if the agent already is in the squad
  for (auto& a : agents_) {
    if (a->getUnitID() == agent->getUnitID()) {
      //Remove it, and add again to update the types.
      //Needed for morphing units like Lurkers.
      removeMember(agent);
      break;
    }
  }

  //Step 2. Check if we have room for this type of agent.
  for (int i = 0; i < (int)setup_.size(); i++) {
    if (setup_.at(i).equals(agent->getUnitType())) {
      //Found a matching setup, see if there is room
      if (setup_.at(i).current < setup_.at(i).no) {
        //Yes we have, add it to the squad
        agents_.insert(agent);
        agent->setSquadID(id_);
        setup_.at(i).current++;

        if (goal_.x >= 0) {
          agent->setGoal(goal_);
        }

        return true;
      }
    }
  }

  return false;
}

void Squad::printInfo() {
  int sx = 440;
  int sy = 30;
  int w = 180;
  int h = 120 + 15 * setup_.size();

  Broodwar->drawBoxScreen(sx - 2, sy, sx + w + 2, sy + h, Colors::Black, true);
  Broodwar->drawTextScreen(sx + 4, sy, "\x03%s", name_.c_str());
  Broodwar->drawLineScreen(sx, sy + 14, sx + w, sy + 14, Colors::Orange);

  Broodwar->drawTextScreen(sx + 2, sy + 15, "Id: \x11%d", id_);
  Broodwar->drawTextScreen(sx + 2, sy + 30, "Goal: \x11(%d,%d)", goal_.x, goal_.y);

  std::string str1 = "Ground ";
  if (isAir()) str1 = "Air ";
  if ((int)setup_.size() == 0) str1 = "";

  std::string str2 = "";
  if (isOffensive()) str2 = "Offensive";
  if (isDefensive()) str2 = "Defensive";
  if (isBunkerDefend()) str2 = "Bunker";
  if (isExplorer()) str2 = "Explorer";
  if (isRush()) str2 = "Rush";

  Broodwar->drawTextScreen(sx + 2, sy + 45, "Type: \x11%s%s", str1.c_str(), str2.c_str());
  Broodwar->drawTextScreen(sx + 2, sy + 60, "Priority: \x11%d", active_priority_);

  if (required_) Broodwar->drawTextScreen(sx + 2, sy + 75, "Required: \x07Yes");
  else Broodwar->drawTextScreen(sx + 2, sy + 75, "Required: \x18No");

  if (isFull()) Broodwar->drawTextScreen(sx + 2, sy + 90, "Full: \x07Yes");
  else Broodwar->drawTextScreen(sx + 2, sy + 90, "Full: \x18No");

  if (isActive()) Broodwar->drawTextScreen(sx + 2, sy + 105, "Active: \x07Yes");
  else Broodwar->drawTextScreen(sx + 2, sy + 105, "Active: \x18No");

  Broodwar->drawLineScreen(sx, sy + 119, sx + w, sy + 119, Colors::Orange);
  int no = 0;
  for (int i = 0; i < (int)setup_.size(); i++) {
    std::string name = Commander::format(setup_.at(i).type.getName());
    Broodwar->drawTextScreen(sx + 2, sy + 120 + 15 * no, "%s \x11(%d/%d)", name.c_str(), setup_.at(i).current, setup_.at(i).no);
    no++;
  }
  Broodwar->drawLineScreen(sx, sy + 119 + 15 * no, sx + w, sy + 119 + 15 * no, Colors::Orange);
}

void Squad::removeDestroyed() {
  for (auto& a : agents_) {
    if (not a->isAlive()) {
      agents_.erase(a);
      return removeDestroyed();
    }
  }
}


bool Squad::isFull() {
  if ((int)setup_.size() == 0) return false;

  //1. Check setup
  for (int i = 0; i < (int)setup_.size(); i++) {
    if (setup_.at(i).current < setup_.at(i).no) {
      return false;
    }
  }

  //2. Check that all units are alive and ready
  try {
    for (auto& a : agents_) {
      if (a->isAlive()) {
        if (a->getUnit() == nullptr) return false;
        if (a->getUnit()->isBeingConstructed()) return false;
      }
      else {
        return false;
      }
    }
  }
  catch (std::exception) {
    return false;
  }

  //3. Check if some morphing is needed
  if (morphs_.getID() != UnitTypes::Unknown.getID()) {
    for (auto& a : agents_) {
      if (morphs_.getID() == UnitTypes::Zerg_Lurker.getID() && a->isOfType(UnitTypes::Zerg_Hydralisk)) {
        return false;
      }
      if (morphs_.getID() == UnitTypes::Zerg_Devourer.getID() && a->isOfType(UnitTypes::Zerg_Mutalisk)) {
        return false;
      }
      if (morphs_.getID() == UnitTypes::Zerg_Guardian.getID() && a->isOfType(UnitTypes::Zerg_Mutalisk)) {
        return false;
      }
    }
  }

  return true;
}

void Squad::removeMember(BaseAgent* agent) {
  //Step 1. Remove the agent instance
  for (auto& a : agents_) {
    if (a->getUnitID() == agent->getUnitID()) {
      a->setSquadID(-1);
      a->setGoal(Broodwar->self()->getStartLocation());
      agents_.erase(a);
      break;
    }
  }

  //Step 2. Update the setup list
  for (int i = 0; i < (int)setup_.size(); i++) {
    if (setup_.at(i).equals(agent->getUnitType())) {
      setup_.at(i).current--;

    }
  }

  //Step 3. If Explorer, set destination as explored (to avoid being killed at the same
  //place over and over again).
  if (isExplorer()) {
    TilePosition goal = agent->getGoal();
    if (goal.x >= 0) {
      rnp::exploration()->setExplored(goal);
    }
  }

  //See if squad should be set to inactive (due to too many dead units)
  if (active_) {
    int noAlive = 0;
    for (auto& a : agents_) {
      if (a->isAlive()) {
        noAlive++;
      }
    }

    if (noAlive <= maxSize() / 10) {
      active_ = false;
    }
  }
}

void Squad::disband() {
  //Remove setup
  for (int i = 0; i < (int)setup_.size(); i++) {
    setup_.at(i).no = 0;
    setup_.at(i).current = 0;
  }

  //Remove agents (if not Bunker Squad)
  if (not this->isBunkerDefend()) {
    for (auto& a : agents_) {
      a->setSquadID(-1);
      a->setGoal(Broodwar->self()->getStartLocation());
    }
  }
  agents_.clear();
}

BaseAgent* Squad::removeMember(UnitType type) {
  BaseAgent* agent = nullptr;

  for (auto& a : agents_) {
    if (UnitSetup::equals(a->getUnitType(), type)) {
      agent = a;
      break;
    }
  }

  if (agent != nullptr) {
    removeMember(agent);
  }

  return agent;
}

void Squad::defend(TilePosition mGoal) {
  if (mGoal.x == -1 || mGoal.y == -1) return;

  if (current_state_ != State::DEFEND) {
    if (current_state_ == State::ASSIST && !isUnderAttack()) {
      current_state_ = State::DEFEND;
    }
  }
  setGoal(mGoal);
}

void Squad::attack(TilePosition mGoal) {
  if (mGoal.x == -1 || mGoal.y == -1) return;

  if (current_state_ != State::ATTACK) {
    if (not isUnderAttack()) {
      if (isActive()) {
        current_state_ = State::ATTACK;
      }
    }
  }

  if (isActive()) {
    setGoal(mGoal);
  }
}

void Squad::assist(TilePosition mGoal) {
  if (mGoal.x == -1) return;

  if (current_state_ != State::ASSIST) {
    if (not isUnderAttack()) {
      Broodwar << "SQ " << id_ << " assist at (" << mGoal.x << "," << mGoal.y << ")" << std::endl;
      current_state_ = State::ASSIST;
      setGoal(mGoal);
    }
  }
}

void Squad::setGoal(TilePosition mGoal) {
  if (isAttacking()) {
    if (goal_.x != -1) {
      return;
    }
  }

  if (mGoal.x != goal_.x || mGoal.y != goal_.y) {
    goal_set_frame_ = Broodwar->getFrameCount();
    if (isGround()) {
      int d = (int)goal_.getDistance(mGoal);
      if (d >= 10) {
        if ((int)agents_.size() > 0) {
          rnp::pathfinder()->requestPath(getCenter(), mGoal);
          if (not rnp::pathfinder()->isReady(getCenter(), mGoal)) {
            return;
          }
          path_ = rnp::pathfinder()->getPath(getCenter(), mGoal);

          arrived_frame_ = -1;
          path_index_ = 20;
        }
      }
    }

    this->goal_ = mGoal;
    setMemberGoals(goal_);
  }
}

TilePosition Squad::nextFollowMovePosition() {
  if (path_.size() <= 0) {
    return goal_;
  }

  if (path_index_ >= (int)path_.size()) {
    return goal_;
  }

  int cPathIndex = path_index_ - 20;
  if (cPathIndex < 0) cPathIndex = 0;

  auto chokepath_step = path_.at(cPathIndex);
  TilePosition cGoal(chokepath_step->Center());

  return cGoal;
}

TilePosition Squad::nextMovePosition() {
  if (path_.size() <= 0) {
    return goal_;
  }
  if (isAir()) {
    return goal_;
  }

  if (path_index_ >= (int)path_.size()) {
    return goal_;
  }

  if (arrived_frame_ == -1) {
    for (auto& a : agents_) {
      //Check if we have arrived at a checkpoint. For mixed squads,
      //air units does not count as having arrived.
      bool check = false;
      if (isGround() && !a->getUnitType().isFlyer()) check = true;
      if (isAir()) check = true;

      if (check) {
        int seekDist = a->getUnitType().sightRange() / 2;
        auto path_step = path_.at(path_index_);
        Position path_step_pos(path_step->Center());
        int dist = (int)a->getUnit()->getPosition().getDistance(path_step_pos);

        if (dist <= seekDist) {
          arrived_frame_ = Broodwar->getFrameCount();
          break;
        }
      }
    }
  }

  if (arrived_frame_ != -1) {
    int cFrame = Broodwar->getFrameCount();
    if (cFrame - arrived_frame_ >= 200) //100
    {
      path_index_ += 20; //20
      if (path_index_ >= (int)path_.size()) {
        path_index_ = (int)path_.size() - 1;
      }
      arrived_frame_ = -1;
    }
  }

  auto path_step = path_.at(path_index_);
  TilePosition cGoal(path_step->Center());
  setMemberGoals(cGoal);

  return cGoal;
}

void Squad::clearGoal() {
  this->goal_ = TilePosition(-1, -1);
  setMemberGoals(goal_);
}

void Squad::setMemberGoals(TilePosition cGoal) {
  if (isBunkerDefend()) return;

  for (auto& a : agents_) {
    if (a->isAlive()) {
      a->setGoal(cGoal);
    }
  }
}

TilePosition Squad::getGoal() {
  return goal_;
}

bool Squad::hasGoal() {
  int elapsed = Broodwar->getFrameCount() - goal_set_frame_;
  if (elapsed >= 600) {
    if (not isAttacking()) {
      goal_ = TilePosition(-1, -1);
    }
  }

  if (goal_.x < 0 || goal_.y < 0) {
    return false;
  }
  return true;
}

TilePosition Squad::getCenter() {
  if (agents_.size() == 1) {
    BaseAgent* a = *(agents_.begin());
    return a->getUnit()->getTilePosition();
  }

  int cX = 0;
  int cY = 0;
  int cnt = 0;

  //Calculate sum (x,y)
  for (auto& a : agents_) {
    if (a->isAlive()) {
      cX += a->getUnit()->getTilePosition().x;
      cY += a->getUnit()->getTilePosition().y;
      cnt++;
    }
  }

  //Calculate average (x,y)
  if (cnt > 0) {
    cX = cX / cnt;
    cY = cY / cnt;
  }

  //To make sure the center is in a walkable tile, we need to
  //find the unit closest to center
  TilePosition c = TilePosition(cX, cY);
  TilePosition bestSpot = c;
  double bestDist = 10000;
  for (auto& a : agents_) {
    if (a->isAlive()) {
      if ((isAir() && a->getUnitType().isFlyer()) || (isGround() && !a->getUnitType().isFlyer())) {
        double dist = a->getUnit()->getTilePosition().getDistance(c);
        if (dist < bestDist) {
          bestDist = dist;
          bestSpot = a->getUnit()->getTilePosition();
        }
      }
    }
  }

  return bestSpot;
}

int Squad::getSize() {
  int no = 0;
  for (auto& a : agents_) {
    if (a->isAlive() && !a->getUnit()->isBeingConstructed()) {
      no++;
    }
  }
  return no;
}

int Squad::getTotalUnits() {
  int tot = 0;

  for (int i = 0; i < (int)setup_.size(); i++) {
    tot += setup_.at(i).no;
  }

  return tot;
}

int Squad::getStrength() {
  int str = 0;

  for (auto& a : agents_) {
    if (a->isAlive()) {
      str += a->getUnitType().destroyScore();
    }
  }

  return str;
}

bool Squad::isOffensive() const {
  if (type_ == SquadType::OFFENSIVE) return true;
  if (type_ == SquadType::SHUTTLE) return true;
  return false;
}

bool Squad::isDefensive() const {
  return type_ == SquadType::DEFENSIVE;
}

bool Squad::isExplorer() const {
  return type_ == SquadType::EXPLORER;
}

bool Squad::isSupport() const {
  return type_ == SquadType::SUPPORT;
}

bool Squad::isBunkerDefend() const {
  return type_ == SquadType::BUNKER;
}

bool Squad::isShuttle() const {
  return type_ == SquadType::SHUTTLE;
}

bool Squad::isKite() const {
  return type_ == SquadType::KITE;
}

bool Squad::isRush() const {
  return type_ == SquadType::RUSH;
}

bool Squad::isGround() const {
  return move_type_ == MoveType::GROUND;
}

bool Squad::isAir() const {
  return move_type_ == MoveType::AIR;
}

bool Squad::hasUnits(UnitType type, int no) {
  for (int i = 0; i < (int)setup_.size(); i++) {
    if (setup_.at(i).equals(type)) {
      if (setup_.at(i).current >= no) {
        //I have these units
        return true;
      }
    }
  }
  return false;
}

void Squad::setBunkerID(int unitID) {
  bunker_id_ = unitID;
}

int Squad::getBunkerID() const {
  return bunker_id_;
}
