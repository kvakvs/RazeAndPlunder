#include "UnitAgent.h"
#include "Pathfinding/NavigationAgent.h"
#include "MainAgents/TargetingAgent.h"
#include "Glob.h"
#include "RnpUtil.h"
#include "Commander/Squad.h"

using namespace BWAPI;

UnitAgent::UnitAgent() {

}

UnitAgent::~UnitAgent() {

}

UnitAgent::UnitAgent(Unit m_unit) {
  unit_ = m_unit;
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
  if (not unit_->isLoaded() 
      && not unit_->isSieged()
      && not unit_->isBurrowed()) 
  {
    // Detect if the unit is stuck
    auto last_i = movement_progress_.get_frames_since_last_improvement();
    if (last_i > rnp::seconds(10)) {
      return on_unit_stuck();
    }

    // TODO: Check is this called every frame???
    if (rnp::navigation()->compute_move(this, goal_)) {
      last_order_frame_ = Broodwar->getFrameCount();
      return;
    }
  }
}

void UnitAgent::debug_print_info() const {
  int e = Broodwar->getFrameCount() - info_update_frame_;
  if (e >= info_update_time_ 
      || (debug_tooltip_x_ == 0 && debug_tooltip_y_ == 0))
  {
    info_update_frame_ = Broodwar->getFrameCount();
    debug_tooltip_x_ = unit_->getPosition().x;
    debug_tooltip_y_ = unit_->getPosition().y;
  }

  Broodwar->drawBoxMap(debug_tooltip_x_ - 2, debug_tooltip_y_, 
                       debug_tooltip_x_ + 242, debug_tooltip_y_ + 105, 
                       Colors::Black, true);
  Broodwar->drawTextMap(debug_tooltip_x_ + 4, debug_tooltip_y_, "\x03%s", 
                        rnp::remove_race(unit_->getType().getName()).c_str());
  Broodwar->drawLineMap(debug_tooltip_x_, debug_tooltip_y_ + 14,
                        debug_tooltip_x_ + 240, debug_tooltip_y_ + 14, 
                        Colors::Green);

  Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 15, 
                        "Id: \x11%d", unit_id_);
  Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 30, 
                        "Position: \x11(%d,%d)", 
                        unit_->getTilePosition().x,
                        unit_->getTilePosition().y);
  Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 45,
                        "Goal: \x11(%d,%d)", goal_.x, goal_.y);
  Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 60, 
                        "Squad: \x11%d", squad_id_);

  int range = unit_->getType().seekRange();
  if (unit_->getType().sightRange() > range) {
    range = unit_->getType().sightRange();
  }
  int enemyInRange = 0;
  for (auto& u : Broodwar->enemy()->getUnits()) {
    float dist = rnp::distance(unit_->getPosition(), u->getPosition());
    if (dist <= range) {
      enemyInRange++;
      break;
    }
  }

  Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 75,
                        "Range: \x11%d", range);
  if (enemyInRange == 0) {
    Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 90, 
                          "Enemies seen: \x11%d",
                          enemyInRange);
  }
  else {
    Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 90,
                          "Enemies seen: \x08%d",
                          enemyInRange);
  }

  std::string str = "\x07No";
  if (unit_->isAttacking() || unit_->isStartingAttack()) str = "\x08Yes";

  //Column two
  Broodwar->drawTextMap(debug_tooltip_x_ + 100, debug_tooltip_y_ + 15,
                        "Attacking: %s", str.c_str());
  int nsy = debug_tooltip_y_ + 30;
  if (type_.groundWeapon().targetsGround()) {
    std::stringstream ss;
    if (unit_->getGroundWeaponCooldown() == 0) ss << "\x07Ready";
    else {
      ss << "\x08";
      ss << unit_->getGroundWeaponCooldown();
    }

    Broodwar->drawTextMap(debug_tooltip_x_ + 100, nsy, 
                          "Ground CD: %s", ss.str().c_str());
    nsy += 15;
  }

  if (type_.airWeapon().targetsAir()) {
    std::stringstream ss;
    if (unit_->getAirWeaponCooldown() == 0) ss << "\x07Ready";
    else {
      ss << "\x08";
      ss << unit_->getAirWeaponCooldown();
    }

    Broodwar->drawTextMap(debug_tooltip_x_ + 100, nsy, 
                          "Air CD: %s", ss.str().c_str());
    nsy += 15;
  }

  Unit target = unit_->getTarget();
  if (not target) target = unit_->getOrderTarget();
  str = "";
  if (target) {
    str = rnp::remove_race(target->getType().getName());
  }
  Broodwar->drawTextMap(debug_tooltip_x_ + 100, nsy, 
                        "Target: \x11%s", str.c_str());
//  nsy += 15;

  Broodwar->drawLineMap(debug_tooltip_x_, debug_tooltip_y_ + 104, 
                        debug_tooltip_x_ + 240, debug_tooltip_y_ + 104, 
                        Colors::Green);
}

void UnitAgent::on_unit_stuck() {
  set_goal(rnp::make_bad_position());
  // Let the superiors know
  if (squad_id_.is_valid()) {
    act::ActorId this_id = self();
    act::modify_actor<Squad>(squad_id_,
                             [=](Squad* s) {
                               s->on_squad_member_stuck(this_id);
                             });
  } else {
    rnp::log()->trace("Unit {} {} got stuck on the move",
                      self().string(),
                      rnp::remove_race(unit_->getType().toString()));
  }
}
