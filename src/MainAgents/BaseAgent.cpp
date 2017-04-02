#include "BaseAgent.h"
#include "Managers/Constructor.h"
#include "Managers/ExplorationManager.h"
#include "Glob.h"

using namespace BWAPI;

int BaseAgent::info_update_frame_ = 0;
int BaseAgent::debug_tooltip_x_ = 0;
int BaseAgent::debug_tooltip_y_ = 0;

BaseAgent::BaseAgent()
    : unit_(), type_(UnitTypes::Unknown)
    , movement_progress_()
    , goal_(rnp::make_bad_position())
    , squad_id_()
    , alive_(true)
    , agent_type_("What?")
    , info_update_time_(20)
    , trail_()
{
}

BaseAgent::BaseAgent(Unit mUnit)
    : unit_(mUnit), type_(mUnit->getType())
    , movement_progress_()
    , goal_(rnp::make_bad_position())
    , unit_id_(unit_->getID())
    , squad_id_()
    , alive_(true)
    , agent_type_("BaseAgent")
    , trail_()
{
}

BaseAgent::~BaseAgent() {

}

int BaseAgent::get_last_order_frame() const {
  return last_order_frame_;
}

const std::string& BaseAgent::get_type_name() const {
  return agent_type_;
}

int BaseAgent::get_unit_id() const {
  return unit_id_;
}

UnitType BaseAgent::unit_type() const {
  return type_;
}

Unit BaseAgent::get_unit() const {
  return unit_;
}

bool BaseAgent::matches(Unit mUnit) const {
  if (mUnit->getID() == unit_id_) {
    return true;
  }
  return false;
}

bool BaseAgent::is_of_type(UnitType type) const {
  return rnp::same_type(unit_, type);
}

bool BaseAgent::is_building() const {
  if (unit_->getType().isBuilding()) {
    return true;
  }
  return false;
}

bool BaseAgent::is_worker() const {
  if (unit_->getType().isWorker()) {
    return true;
  }
  return false;
}

bool BaseAgent::is_unit() const {
  if (unit_->getType().isBuilding() || unit_->getType().isWorker() || unit_->getType().isAddon()) {
    return false;
  }
  return true;
}

bool BaseAgent::is_under_attack() const {
  if (unit_ == nullptr) return false;
  if (not unit_->exists()) return false;

  if (unit_->isAttacking()) return true;
  if (unit_->isStartingAttack()) return true;

  auto ut = unit_->getType();
  float r = static_cast<float>(ut.seekRange());
  if (ut.sightRange() > r) {
    r = static_cast<float>(ut.sightRange());
  }

  // TODO: Rewrite this with actors in range (grid)
  for (auto& u : Broodwar->enemy()->getUnits()) {
    float dist = rnp::distance(unit_->getPosition(), u->getPosition());
    if (dist <= r) {
      return true;
    }
  }

  return false;
}

bool BaseAgent::is_alive() const {
  if (not unit_->exists()) {
    return false;
  }
  return alive_;
}

bool BaseAgent::is_damaged() const {
  if (unit_->isBeingConstructed()) return false;
  if (unit_->getRemainingBuildTime() > 0) return false;

  if (unit_->getHitPoints() < unit_->getType().maxHitPoints()) {
    return true;
  }
  return false;
}

bool BaseAgent::is_enemy_detector_within_range(TilePosition pos, int range) {
  for (auto& u : Broodwar->enemy()->getUnits()) {
    if (u->getType().isDetector()) {
      double dist = u->getDistance(Position(pos));
      if (dist <= range) {
        return true;
      }
    }
  }
  return false;
}

bool BaseAgent::any_enemy_units_visible() const {
  double r = unit_->getType().sightRange();

  for (auto& u : Broodwar->enemy()->getUnits()) {
    double dist = unit_->getPosition().getDistance(u->getPosition());
    if (dist <= r) {
      return true;
    }
  }
  return false;
}

void BaseAgent::set_goal(TilePosition goal) {
  if (unit_->getType().isFlyer() || unit_->getType().isFlyingBuilding()) {
    //Flyers, can always move to goals.
    goal_ = goal;
    movement_progress_.reset();
  }
  else {
    //Ground units, check if we can reach goal.
    if (rnp::exploration()->can_reach(this, goal)) {
      goal_ = goal;
      movement_progress_.reset();
    }
  }
}

void BaseAgent::clear_goal() {
  goal_ = rnp::make_bad_position();
}

const TilePosition& BaseAgent::get_goal() const {
  return goal_;
}

void BaseAgent::add_trail_position(WalkPosition wt) {
  //Check if position already is in trail
  if (not trail_.empty()) {
    auto& lwt = trail_.back();
    if (lwt == wt) return;
  }

  trail_.push_back(wt);
  if (trail_.size() > 20) {
    trail_.erase(trail_.begin());
  }
}

const std::vector<WalkPosition>& BaseAgent::get_trail() const {
  return trail_;
}

bool BaseAgent::can_target_air() const {
  return type_.groundWeapon().targetsAir()
    || type_.airWeapon().targetsAir();
}

bool BaseAgent::can_target_ground() const {
  return type_.groundWeapon().targetsGround()
    || type_.airWeapon().targetsGround();

}

void BaseAgent::handle_message(act::Message* incoming) {

  // TODO: distribute attack event to the squad
  if (auto trpos = dynamic_cast<msg::unit::AddTrailPos*>(incoming)) {
    add_trail_position(trpos->pos_);
  }
  if (auto atk_u = dynamic_cast<msg::unit::AttackBWUnit*>(incoming)) {
    handle_message_attack_bwunit(atk_u);
  } 
  else if (auto atk_a = dynamic_cast<msg::unit::Attack*>(incoming)) {
    handle_message_attack_actor(atk_a);
  }
  else if (auto sq_join = dynamic_cast<msg::unit::JoinedSquad*>(incoming)) {
    handle_message_squad_join(sq_join);
  }
  else if (auto leave = dynamic_cast<msg::unit::LeftSquad*>(incoming)) {
    if (leave->squad_ == this->get_squad_id()) {
      set_squad_id(act::ActorId());
      set_goal(Broodwar->self()->getStartLocation());
    }
  }
  else if (dynamic_cast<msg::unit::Destroyed*>(incoming)) {
    destroyed();
  }
  else if (auto setsq = dynamic_cast<msg::unit::SetSquad*>(incoming)) {
    set_squad_id(setsq->squad_);
  }
  else {
    // Since BaseAgent is base class for everything, we don't need to pass the
    // message to our base, instead create an error
    unhandled_message(incoming);
  }
}

void BaseAgent::handle_message_attack_bwunit(
  const msg::unit::AttackBWUnit* atk) const 
{
  get_unit()->attack(atk->target_);
}

void BaseAgent::handle_message_attack_actor(msg::unit::Attack* atk) const {
  auto target = act::whereis<BaseAgent>(atk->target_);
  if (target) {
    get_unit()->attack(target->get_unit());
  }
}

void BaseAgent::handle_message_squad_join(const msg::unit::JoinedSquad* sq_join) {
  set_squad_id(sq_join->squad_);
  if (rnp::is_valid_position(goal_)) {
      set_goal(sq_join->goal_);
    }
}

void BaseAgent::destroyed() {
  alive_ = false;
}
