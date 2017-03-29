#include "WorkerAgent.h"
#include "Managers/AgentManager.h"
#include "Pathfinding/NavigationAgent.h"
#include "Managers/BuildingPlacer.h"
#include "Managers/Constructor.h"
#include "Commander/Commander.h"
#include "Managers/ResourceManager.h"
#include "Glob.h"
#include "RnpUtil.h"

using namespace BWAPI;

WorkerAgent::WorkerAgent(Unit mUnit) {
  unit_ = mUnit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();
  set_state(WorkerState::GATHER_MINERALS);
  start_build_frame_ = 0;
  start_spot_ = rnp::make_bad_position();
  agent_type_ = "WorkerAgent";

  to_build_ = UnitTypes::None;
}

void WorkerAgent::destroyed() {
  if (current_state_ == WorkerState::MOVE_TO_SPOT
      || current_state_ == WorkerState::CONSTRUCT
      || current_state_ == WorkerState::FIND_BUILDSPOT)
  {
    if (not Constructor::is_zerg()) {
      Constructor::modify([=](Constructor* c) {
          c->handle_worker_destroyed(to_build_, unit_id_);
        });
      rnp::building_placer()->clear_temp(to_build_, build_spot_);
      set_state(WorkerState::GATHER_MINERALS);
    }
  }
}

void WorkerAgent::debug_print_info() const {
  int e = Broodwar->getFrameCount() - info_update_frame_;
  if (e >= info_update_time_ || (sx_ == 0 && sy_ == 0)) {
    info_update_frame_ = Broodwar->getFrameCount();
    sx_ = unit_->getPosition().x;
    sy_ = unit_->getPosition().y;
  }

  Broodwar->drawBoxMap(sx_ - 2, sy_, sx_ + 152, sy_ + 90, Colors::Black, true);
  Broodwar->drawTextMap(sx_ + 4, sy_, "\x03%s", 
                        unit_->getType().getName().c_str());
  Broodwar->drawLineMap(sx_, sy_ + 14, sx_ + 150, sy_ + 14, Colors::Blue);

  Broodwar->drawTextMap(sx_ + 2, sy_ + 15, "Id: \x11%d", unit_id_);
  Broodwar->drawTextMap(sx_ + 2, sy_ + 30, "Position: \x11(%d,%d)", 
                        unit_->getTilePosition().x, unit_->getTilePosition().y);
  Broodwar->drawTextMap(sx_ + 2, sy_ + 45, "Goal: \x11(%d,%d)", goal_.x, goal_.y);
  if (squad_id_.is_valid() == false) {
    Broodwar->drawTextMap(sx_ + 2, sy_ + 60, "Squad: \x15None");
  } 
  else {
    auto sq = act::whereis<Squad>(squad_id_);
    Broodwar->drawTextMap(sx_ + 2, sy_ + 60, "\x11%s", sq->string().c_str());
  }
  Broodwar->drawTextMap(sx_ + 2, sy_ + 75, "State: \x11%s",
                        get_state_as_text().c_str());

  Broodwar->drawLineMap(sx_, sy_ + 89, sx_ + 150, sy_ + 89, Colors::Blue);
}

void WorkerAgent::debug_show_goal() const {
  if (not is_alive()) return;
  if (not unit_->isCompleted()) return;

  if (current_state_ == WorkerState::GATHER_MINERALS || current_state_ == WorkerState::GATHER_GAS) {
    Unit target = unit_->getTarget();
    if (target != nullptr) {
      Position a = unit_->getPosition();
      Position b = target->getPosition();
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Teal);
    }
  }

  if (current_state_ == WorkerState::MOVE_TO_SPOT || current_state_ == WorkerState::CONSTRUCT) {
    if (build_spot_.x > 0) {
      int w = to_build_.tileWidth() * 32;
      int h = to_build_.tileHeight() * 32;

      Position a = unit_->getPosition();
      Position b = Position(build_spot_.x * 32 + w / 2, build_spot_.y * 32 + h / 2);
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Teal);

      Broodwar->drawBoxMap(build_spot_.x * 32, build_spot_.y * 32, build_spot_.x * 32 + w, build_spot_.y * 32 + h, Colors::Blue, false);
    }
  }

  if (unit_->isRepairing()) {
    Unit targ = unit_->getOrderTarget();
    if (targ != nullptr) {
      Position a = unit_->getPosition();
      Position b = targ->getPosition();
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Green);

      Broodwar->drawTextMap(unit_->getPosition().x, unit_->getPosition().y, "Repairing %s", targ->getType().getName().c_str());
    }
  }

  if (unit_->isConstructing()) {
    Unit targ = unit_->getOrderTarget();
    if (targ != nullptr) {
      Position a = unit_->getPosition();
      Position b = targ->getPosition();
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Green);

      Broodwar->drawTextMap(unit_->getPosition().x, unit_->getPosition().y, "Constructing %s", targ->getType().getName().c_str());
    }
  }
}

bool WorkerAgent::check_repair() {
  if (unit_->getType().getID() != UnitTypes::Terran_SCV.getID()) return false;
  if (unit_->isRepairing()) return true;

  //Find closest unit that needs repairing
  const BaseAgent* to_repair = nullptr;
  float best_dist = 1e+12f;

  act::for_each_actor<BaseAgent>(
    [this,&best_dist,&to_repair](const BaseAgent* a) {
      if (a->is_damaged()
          && a->unit_type().isMechanical()
          && a->get_unit_id() != unit_id_) 
      {
        auto a_pos = a->get_unit()->getPosition();
        float c_dist = rnp::distance(a_pos, unit_->getPosition());
        if (c_dist < best_dist) {
          best_dist = c_dist;
          to_repair = a;
        }
      }
    });

  //Repair it
  if (to_repair) {
    unit_->repair(to_repair->get_unit());
    return true;
  }

  return false;
}

void WorkerAgent::compute_squad_worker_actions() {
  //Repairing
  if (check_repair()) return;

  //No repairing. Gather minerals
  auto sq = rnp::commander()->get_squad(squad_id_);
  if (sq) {
    //If squad is not ative, let the worker gather
    //minerals while not doing any repairs
    if (not sq->is_active()) {
      if (unit_->isIdle()) {
        Unit mineral = rnp::building_placer()->find_closest_mineral(unit_->getTilePosition());
        if (mineral != nullptr) {
          unit_->rightClick(mineral);
          return;
        }
      }
    }
    else {
      rnp::navigation()->compute_move(this, goal_);
      return;
    }
  }
}

bool WorkerAgent::is_available_worker() const {
  if (current_state_ != WorkerState::GATHER_MINERALS) return false;
  if (to_build_.getID() != UnitTypes::None.getID()) return false;
  if (unit_->isConstructing()) return false;
  Unit b = unit_->getTarget();
  if (b != nullptr) if (b->isBeingConstructed()) return false;
  if (unit_->isRepairing()) return false;
  if (squad_id_.is_valid()) return false;

  return true;
}


void WorkerAgent::tick_attacking() {
  if (unit_->getTarget() != nullptr) {
    auto base = rnp::agent_manager()->get_closest_base(unit_->getTilePosition());
    if (base != nullptr) {
      auto base_pos = base->get_unit()->getTilePosition();
      float dist = rnp::distance(base_pos, unit_->getTilePosition());
      if (dist > 25.0f) {
        //Stop attacking. Return home
        unit_->stop();
        unit_->rightClick(base->get_unit());
        set_state(WorkerState::GATHER_MINERALS);
      }
    }
  }
  else {
    //No target, return to gather minerals
    set_state(WorkerState::GATHER_MINERALS);
  }
}

void WorkerAgent::tick_repairing() {
  Unit target = unit_->getTarget();
  if (not target
      || target->getHitPoints() >= target->getInitialHitPoints())
  {
    reset();
  }
}

void WorkerAgent::tick_gather() {
  if (unit_->isIdle()) {
    Unit mineral = rnp::building_placer()->find_closest_mineral(unit_->getTilePosition());
    if (mineral != nullptr) {
      unit_->rightClick(mineral);
    }
  }
}

void WorkerAgent::tick_find_build_spot() {
  if (not rnp::is_valid_position(build_spot_)) {
    build_spot_ = rnp::building_placer()->find_build_spot(to_build_);
  }
  if (build_spot_.x >= 0) {
    set_state(WorkerState::MOVE_TO_SPOT);
    start_build_frame_ = Broodwar->getFrameCount();
    if (to_build_.isResourceDepot()) {
      //rnp::commander()->update_squad_goals();
      msg::commander::update_goals();
    }
  }
}

void WorkerAgent::tick_move_to_spot() {
  if (not is_build_spot_explored()) {
    Position to_move(build_spot_.x * TILEPOSITION_SCALE + TILEPOSITION_SCALE/2,
                     build_spot_.y * TILEPOSITION_SCALE + TILEPOSITION_SCALE/2);
    if (to_build_.isRefinery()) to_move = Position(build_spot_);
    unit_->rightClick(to_move);
  }

  if (is_build_spot_explored() && not unit_->isConstructing()) {
    bool ok = unit_->build(to_build_, build_spot_);
    if (not ok) {
      rnp::building_placer()->mark_position_blocked(build_spot_);
      rnp::building_placer()->clear_temp(to_build_, build_spot_);
      //Cant build at selected spot, get a new one.
      set_state(WorkerState::FIND_BUILDSPOT);
    }
  }

  if (unit_->isConstructing()) {
    set_state(WorkerState::CONSTRUCT);
    start_spot_ = rnp::make_bad_position();
  }
}

void WorkerAgent::tick_construct() {
  if (is_built()) {
    //Build finished.
    auto agent = rnp::agent_manager()->get_closest_base(unit_->getTilePosition());
    if (agent != nullptr) {
      unit_->rightClick(agent->get_unit()->getPosition());
    }
    set_state(WorkerState::GATHER_MINERALS);
  }
}

void WorkerAgent::tick_gather_gas() {
  if (unit_->isIdle()) {
    //Not gathering gas. Reset.
    set_state(WorkerState::GATHER_MINERALS);
  }
}

void WorkerAgent::tick() {
  //To prevent order spamming
  last_order_frame_ = Broodwar->getFrameCount();

  if (squad_id_.is_valid()) {
    compute_squad_worker_actions();
    return;
  }
  //Check if workers are too far away from a base when attacking
  switch (current_state_) {
  case WorkerState::ATTACKING:        return tick_attacking();
  case WorkerState::GATHER_GAS:       return tick_gather_gas();
  case WorkerState::REPAIRING:        return tick_repairing();
  case WorkerState::GATHER_MINERALS:  return tick_gather();
  case WorkerState::FIND_BUILDSPOT:   return tick_find_build_spot();
  case WorkerState::MOVE_TO_SPOT:     return tick_move_to_spot();
  case WorkerState::CONSTRUCT:        return tick_construct();
  } // switch
}

bool WorkerAgent::is_built() const {
  if (unit_->isConstructing()) return false;

  Unit b = unit_->getTarget();
  if (b != nullptr) if (b->isBeingConstructed()) return false;

  return true;
}

bool WorkerAgent::is_build_spot_explored() const {
  int sightDist = 64;
  if (to_build_.isRefinery()) {
    sightDist = 160; //5 tiles
  }

  double dist = rnp::distance(unit_->getPosition(), Position(build_spot_));
  if (dist > sightDist) {
    return false;
  }
  return true;
}

void WorkerAgent::set_state(WorkerState state) {
  current_state_ = state;

  if (state == WorkerState::GATHER_MINERALS) {
    start_spot_ = build_spot_ = rnp::make_bad_position();
    to_build_ = UnitTypes::None;
  }
}

bool WorkerAgent::can_build(UnitType type) const {
  if (unit_->isIdle()) {
    return true;
  }
  if (unit_->isGatheringMinerals()) {
    return true;
  }
  return false;
}

bool WorkerAgent::assign_to_build(UnitType type) {
  to_build_ = type;
  build_spot_ = rnp::building_placer()->find_build_spot(to_build_);
  if (build_spot_.x >= 0) {
    rnp::resources()->lock_resources(to_build_);
    rnp::building_placer()->fill_temp(to_build_, build_spot_);
    set_state(WorkerState::FIND_BUILDSPOT);
    return true;
  }
  else {
    start_spot_ = rnp::make_bad_position();
    return false;
  }
}

void WorkerAgent::reset() {
  if (current_state_ == WorkerState::MOVE_TO_SPOT) {
    //The buildSpot is probably not reachable. Block it.	
    rnp::building_placer()->mark_position_blocked(build_spot_);
    rnp::building_placer()->clear_temp(to_build_, build_spot_);
  }

  if (unit_->isConstructing()) {
    unit_->cancelConstruction();
    rnp::building_placer()->clear_temp(to_build_, build_spot_);
  }

  if (unit_->isRepairing()) {
    unit_->cancelConstruction();
  }

  set_state(WorkerState::GATHER_MINERALS);
  unit_->stop();
  auto base = rnp::agent_manager()->get_closest_base(unit_->getTilePosition());
  if (base) {
    unit_->rightClick(base->get_unit()->getPosition());
  }
}

bool WorkerAgent::is_constructing(UnitType type) const {
  if (current_state_ == WorkerState::FIND_BUILDSPOT
      || current_state_ == WorkerState::MOVE_TO_SPOT
      || current_state_ == WorkerState::CONSTRUCT) {
    if (to_build_.getID() == type.getID()) {
      return true;
    }
  }
  return false;
}

// Returns the state of the agent as text. Good for printouts. 
std::string WorkerAgent::get_state_as_text() const {
  switch (current_state_) {
  case WorkerState::GATHER_MINERALS:  return "mineral";
  case WorkerState::GATHER_GAS:       return "gas";
  case WorkerState::FIND_BUILDSPOT:   return "findspot";
  case WorkerState::MOVE_TO_SPOT:     return "move";
  case WorkerState::CONSTRUCT:        return "constr";
  case WorkerState::REPAIRING:        return "repair";
  case WorkerState::ATTACKING:        return "attack";
  default: return "?";
  }
}

bool WorkerAgent::assign_to_finish_build(Unit building) {
  if (is_available_worker()) {
    set_state(WorkerState::REPAIRING);
    unit_->rightClick(building);
    return true;
  }
  return false;
}

bool WorkerAgent::assign_to_repair(Unit building) {
  if (is_available_worker()) {
    set_state(WorkerState::REPAIRING);
    unit_->repair(building);
//    unit_->rightClick(building);
    return true;
  }
  return false;
}

void WorkerAgent::handle_message(act::Message* incoming) {
  if (dynamic_cast<msg::unit::AttackBWUnit*>(incoming)) {
    // This event is also handled by parent baseunit which sets attack target
    set_state(WorkerState::ATTACKING);
  }
  else if (auto refi = dynamic_cast<msg::worker::RightClickRefinery*>(incoming)) {
    get_unit()->rightClick(refi->refinery_);
    set_state(WorkerState::GATHER_GAS);
  }
  else if (auto war = dynamic_cast<msg::worker::AssignRepair*>(incoming)) {
    assign_to_repair(war->unit_);
  }
  else if (auto wab = dynamic_cast<msg::worker::AssignBuild*>(incoming)) {
    if (assign_to_build(wab->type)) {
      auto unit_id = get_unit_id();
      Constructor::modify([=](Constructor* c) { c->lock(0, unit_id); });
    } else {
      UnitType type = wab->type;
      Constructor::modify([=](Constructor* c) {
          c->handle_no_buildspot_found(type);
        });
    }
  } 
  else BaseAgent::handle_message(incoming);
}
