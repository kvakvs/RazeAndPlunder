#include "UnitAgent.h"
#include "Pathfinding/NavigationAgent.h"
#include "MainAgents/TargetingAgent.h"
#include "Glob.h"
#include "RnpUtil.h"

using namespace BWAPI;

UnitAgent::UnitAgent() {

}

UnitAgent::~UnitAgent() {

}

UnitAgent::UnitAgent(Unit mUnit) {
  unit_ = mUnit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();
  agent_type_ = "UnitAgent";
  goal_ = rnp::make_bad_position();
  info_update_frame_ = 0;
}

void UnitAgent::tick() {
  //Prio 1: Use abilities
  if (use_abilities()) {
    last_order_frame_ = Broodwar->getFrameCount();
    return;
  }

  //Prio 2: Attack enemy
  if (TargetingAgent::check_target(this)) {
    last_order_frame_ = Broodwar->getFrameCount();
    return;
  }

  //Prio 3: Move
  if (not unit_->isLoaded() && not unit_->isSieged() && not unit_->isBurrowed()) {
    if (rnp::navigation()->compute_move(this, goal_)) {
      last_order_frame_ = Broodwar->getFrameCount();
      return;
    }
  }
}

void UnitAgent::debug_print_info() const {
  int e = Broodwar->getFrameCount() - info_update_frame_;
  if (e >= info_update_time_ || (sx_ == 0 && sy_ == 0)) {
    info_update_frame_ = Broodwar->getFrameCount();
    sx_ = unit_->getPosition().x;
    sy_ = unit_->getPosition().y;
  }

  Broodwar->drawBoxMap(sx_ - 2, sy_, sx_ + 242, sy_ + 105, Colors::Black, true);
  Broodwar->drawTextMap(sx_ + 4, sy_, "\x03%s", 
                        format(unit_->getType().getName()).c_str());
  Broodwar->drawLineMap(sx_, sy_ + 14, sx_ + 240, sy_ + 14, Colors::Green);

  Broodwar->drawTextMap(sx_ + 2, sy_ + 15, "Id: \x11%d", unit_id_);
  Broodwar->drawTextMap(sx_ + 2, sy_ + 30, "Position: \x11(%d,%d)", 
                        unit_->getTilePosition().x, unit_->getTilePosition().y);
  Broodwar->drawTextMap(sx_ + 2, sy_ + 45, "Goal: \x11(%d,%d)",
                        goal_.x, goal_.y);
  Broodwar->drawTextMap(sx_ + 2, sy_ + 60, "Squad: \x11%d", squad_id_);

  int range = unit_->getType().seekRange();
  if (unit_->getType().sightRange() > range) {
    range = unit_->getType().sightRange();
  }
  int enemyInRange = 0;
  for (auto& u : Broodwar->enemy()->getUnits()) {
    double dist = unit_->getPosition().getDistance(u->getPosition());
    if (dist <= range) {
      enemyInRange++;
      break;
    }
  }

  Broodwar->drawTextMap(sx_ + 2, sy_ + 75, "Range: \x11%d", range);
  if (enemyInRange == 0) {
    Broodwar->drawTextMap(sx_ + 2, sy_ + 90, "Enemies seen: \x11%d",
                          enemyInRange);
  }
  else {
    Broodwar->drawTextMap(sx_ + 2, sy_ + 90, "Enemies seen: \x08%d",
                          enemyInRange);
  }

  std::string str = "\x07No";
  if (unit_->isAttacking() || unit_->isStartingAttack()) str = "\x08Yes";

  //Column two
  Broodwar->drawTextMap(sx_ + 100, sy_ + 15, "Attacking: %s", str.c_str());
  int nsy = sy_ + 30;
  if (type_.groundWeapon().targetsGround()) {
    std::stringstream ss;
    if (unit_->getGroundWeaponCooldown() == 0) ss << "\x07Ready";
    else {
      ss << "\x08";
      ss << unit_->getGroundWeaponCooldown();
    }

    Broodwar->drawTextMap(sx_ + 100, nsy, "Ground CD: %s", ss.str().c_str());
    nsy += 15;
  }

  if (type_.airWeapon().targetsAir()) {
    std::stringstream ss;
    if (unit_->getAirWeaponCooldown() == 0) ss << "\x07Ready";
    else {
      ss << "\x08";
      ss << unit_->getAirWeaponCooldown();
    }

    Broodwar->drawTextMap(sx_ + 100, nsy, "Air CD: %s", ss.str().c_str());
    nsy += 15;
  }

  Unit target = unit_->getTarget();
  if (target == nullptr) target = unit_->getOrderTarget();
  str = "";
  if (target != nullptr) {
    str = format(target->getType().getName());
  }
  Broodwar->drawTextMap(sx_ + 100, nsy, "Target: \x11%s", str.c_str());
  nsy += 15;

  Broodwar->drawLineMap(sx_, sy_ + 104, sx_ + 240, sy_ + 104, Colors::Green);
}
