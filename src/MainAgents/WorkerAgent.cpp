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

WorkerAgent::WorkerAgent(Unit unit)
: BaseFsmClass(WorkerState::GATHER_MINERALS) 
{
  unit_ = unit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();
  start_build_frame_ = 0;
  start_spot_ = rnp::make_bad_position();
  agent_type_ = "WorkerAgent";
  to_build_ = UnitTypes::None;
}

void WorkerAgent::destroyed() {
  if (fsm_state() == WorkerState::MOVE_TO_SPOT
      || fsm_state() == WorkerState::CONSTRUCT
      || fsm_state() == WorkerState::FIND_BUILDSPOT)
  {
    if (not Constructor::is_zerg()) {
      Constructor::modify([=](Constructor* c) {
          c->handle_worker_destroyed(to_build_, unit_id_);
        });
      rnp::building_placer()->clear_temp(to_build_, build_spot_);
      fsm_set_state(WorkerState::GATHER_MINERALS);
    }
  }
}

void WorkerAgent::debug_print_info() const {
  int e = Broodwar->getFrameCount() - info_update_frame_;
  if (e >= info_update_time_ || (debug_tooltip_x_ == 0 && debug_tooltip_y_ == 0)) {
    info_update_frame_ = Broodwar->getFrameCount();
    debug_tooltip_x_ = unit_->getPosition().x;
    debug_tooltip_y_ = unit_->getPosition().y;
  }

  // Far at the bottom popup can be clipped by the map edge
  auto mapsize = Position(BWEM::Map::Instance().Size());
  if (debug_tooltip_y_ + 110 > mapsize.y) {
    debug_tooltip_y_ = mapsize.y - 110;
  }

  Broodwar->drawBoxMap(debug_tooltip_x_ - 2, debug_tooltip_y_, 
                       debug_tooltip_x_ + 152, debug_tooltip_y_ + 90,
                       Colors::Black, true);
  Broodwar->drawTextMap(debug_tooltip_x_ + 4, debug_tooltip_y_, "\x03%s", 
                        unit_->getType().getName().c_str());
  Broodwar->drawLineMap(debug_tooltip_x_, debug_tooltip_y_ + 14, 
                        debug_tooltip_x_ + 150, debug_tooltip_y_ + 14,
                        Colors::Blue);

  Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 15,
                        "Id: \x11%d", unit_id_);
  Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 30,
                        "Position: \x11(%d,%d)", 
                        unit_->getTilePosition().x, unit_->getTilePosition().y);
  Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 45, 
                        "Goal: \x11(%d,%d)", goal_.x, goal_.y);
  if (squad_id_.is_valid() == false) {
    Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 60,
                          "Squad: \x15None");
  } 
  else {
    auto sq = act::whereis<Squad>(squad_id_);
    Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 60, 
                          "\x11%s", sq->string().c_str());
  }
  Broodwar->drawTextMap(debug_tooltip_x_ + 2, debug_tooltip_y_ + 75,
                        "State: \x11%s",
                        get_state_as_text().c_str());

  Broodwar->drawLineMap(debug_tooltip_x_, debug_tooltip_y_ + 89, 
                        debug_tooltip_x_ + 150, debug_tooltip_y_ + 89,
                        Colors::Blue);
}

void WorkerAgent::debug_show_goal() const {
  if (not is_alive()) return;
  if (not unit_->isCompleted()) return;

  if (fsm_state() == WorkerState::GATHER_MINERALS
      || fsm_state() == WorkerState::GATHER_GAS)
  {
    Unit target = unit_->getTarget();
    if (target != nullptr) {
      Position a = unit_->getPosition();
      Position b = target->getPosition();
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Teal);
    }
  }

  if (fsm_state() == WorkerState::MOVE_TO_SPOT
      || fsm_state() == WorkerState::CONSTRUCT) {
    if (rnp::is_valid_position(build_spot_)) {
      int w = to_build_.tileWidth() * TILEPOSITION_SCALE;
      int h = to_build_.tileHeight() * TILEPOSITION_SCALE;

      Position a = unit_->getPosition();
      Position b = Position(build_spot_.x * TILEPOSITION_SCALE + w / 2,
                            build_spot_.y * TILEPOSITION_SCALE + h / 2);
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Teal);

      Broodwar->drawBoxMap(build_spot_.x * TILEPOSITION_SCALE, 
                           build_spot_.y * TILEPOSITION_SCALE, 
                           build_spot_.x * TILEPOSITION_SCALE + w, 
                           build_spot_.y * TILEPOSITION_SCALE + h,
                           Colors::Blue, false);
    }
  }

  if (unit_->isRepairing()) {
    Unit targ = unit_->getOrderTarget();
    if (targ != nullptr) {
      Position a = unit_->getPosition();
      Position b = targ->getPosition();
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Green);

      Broodwar->drawTextMap(unit_->getPosition().x, unit_->getPosition().y,
                            "Repairing %s", targ->getType().getName().c_str());
    }
  }

  if (unit_->isConstructing()) {
    Unit targ = unit_->getOrderTarget();
    if (targ != nullptr) {
      Position a = unit_->getPosition();
      Position b = targ->getPosition();
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Green);

      Broodwar->drawTextMap(unit_->getPosition().x, unit_->getPosition().y, 
                            "Constructing %s", targ->getType().getName().c_str());
    }
  }
}

bool WorkerAgent::check_repair() {
  if (unit_->getType().getID() != UnitTypes::Terran_SCV.getID()) return false;
  if (unit_->isRepairing()) return true;

  //Find closest unit that needs repairing
  const BaseAgent* to_repair = nullptr;
  float best_dist = LIKE_VERY_FAR;

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
  if (fsm_state() != WorkerState::GATHER_MINERALS) return false;
  if (to_build_.getID() != UnitTypes::None.getID()) return false;
  if (unit_->isConstructing()) return false;

  Unit b = unit_->getTarget();
  if (b != nullptr) if (b->isBeingConstructed()) return false;
  if (unit_->isRepairing()) return false;
  if (squad_id_.is_valid()) return false;

  return true;
}


void WorkerAgent::tick_attacking() {
  if (unit_->getTarget()) {
    auto base = rnp::agent_manager()->get_closest_base(unit_->getTilePosition());
    if (base != nullptr) {
      auto base_pos = base->get_unit()->getTilePosition();
      float dist = rnp::distance(base_pos, unit_->getTilePosition());
      if (dist > 25.0f) {
        //Stop attacking. Return home
        unit_->stop();
        unit_->rightClick(base->get_unit());
        fsm_set_state(WorkerState::GATHER_MINERALS);
      }
    }
  }
  else {
    //No target, return to gather minerals
    fsm_set_state(WorkerState::GATHER_MINERALS);
  }
}

void WorkerAgent::tick_repairing() {
  Unit target = unit_->getTarget();
  if (not target
      || target->getHitPoints() >= target->getInitialHitPoints())
  {
    reset();
  } else {
    act::suspend(self(), 8);
  }
}

void WorkerAgent::tick_gather() {
  if (unit_->isIdle()) {
    Unit mineral = rnp::building_placer()->find_closest_mineral(unit_->getTilePosition());
    if (mineral) {
      unit_->rightClick(mineral);
    }
  }
}

void WorkerAgent::tick_find_build_spot() {
  if (not rnp::is_valid_position(build_spot_)) {
    build_spot_ = rnp::building_placer()->find_build_spot(to_build_);
  }
  if (rnp::is_valid_position(build_spot_)) {
    fsm_set_state(WorkerState::MOVE_TO_SPOT);
    start_build_frame_ = Broodwar->getFrameCount();
    if (to_build_.isResourceDepot()) {
      msg::commander::update_goals();
    }
  }
}

void WorkerAgent::tick_move_to_spot() {
  // Detect if the unit is stuck
  auto last_i = movement_progress_.get_frames_since_last_improvement();
  if (last_i > rnp::seconds(10)) {
    return on_worker_stuck();
  }

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
      fsm_set_state(WorkerState::FIND_BUILDSPOT);
    }
  }

  if (unit_->isConstructing()) {
    fsm_set_state(WorkerState::CONSTRUCT);
    start_spot_ = rnp::make_bad_position();
  }
}

void WorkerAgent::tick_construct() {
  if (is_built()) {
    //Build finished.
    auto agent = rnp::agent_manager()->get_closest_base(unit_->getTilePosition());
    if (agent) {
      unit_->rightClick(agent->get_unit()->getPosition());
    }
    fsm_set_state(WorkerState::GATHER_MINERALS);
  }
}

void WorkerAgent::tick_gather_gas() {
  if (unit_->isIdle()) {
    //Not gathering gas. Reset.
    fsm_set_state(WorkerState::GATHER_MINERALS);
  }
}

void WorkerAgent::on_worker_stuck() {
  //rnp::log()->trace("Worker {} got stuck on the move", self().string());
  set_goal(rnp::make_bad_position());
}

void WorkerAgent::tick() {
  //To prevent order spamming
  last_order_frame_ = Broodwar->getFrameCount();

  if (squad_id_.is_valid()) {
    compute_squad_worker_actions();
    return;
  }

  //Check if workers are too far away from a base when attacking
  switch (fsm_state()) {
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
  if (b && b->isBeingConstructed()) return false;

  return true;
}

bool WorkerAgent::is_build_spot_explored() const {
  int sight_dist = 2 * TILEPOSITION_SCALE;
  if (to_build_.isRefinery()) {
    sight_dist = 5 * TILEPOSITION_SCALE; //5 tiles
  }

  double dist = rnp::distance(unit_->getPosition(), Position(build_spot_));
  if (dist > sight_dist) {
    return false;
  }
  return true;
}

void WorkerAgent::fsm_on_transition(WorkerState old_st, WorkerState new_st) {
  if (new_st == WorkerState::GATHER_MINERALS) {
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
  if (rnp::is_valid_position(build_spot_)) {
    rnp::resources()->lock_resources(to_build_);
    rnp::building_placer()->fill_temp(to_build_, build_spot_);
    fsm_set_state(WorkerState::FIND_BUILDSPOT);
    return true;
  }
  else {
    start_spot_ = rnp::make_bad_position();
    return false;
  }
}

void WorkerAgent::reset() {
  if (fsm_state() == WorkerState::MOVE_TO_SPOT) {
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

  fsm_set_state(WorkerState::GATHER_MINERALS);
  unit_->stop();

  auto base = rnp::agent_manager()->get_closest_base(unit_->getTilePosition());
  if (base) {
    unit_->rightClick(base->get_unit()->getPosition());
  }
}

bool WorkerAgent::is_constructing(UnitType type) const {
  if (fsm_state() == WorkerState::FIND_BUILDSPOT
      || fsm_state() == WorkerState::MOVE_TO_SPOT
      || fsm_state() == WorkerState::CONSTRUCT)
  {
    if (to_build_.getID() == type.getID()) {
      return true;
    }
  }
  return false;
}

// Returns the state of the agent as text. Good for printouts. 
std::string WorkerAgent::get_state_as_text() const {
  switch (fsm_state()) {
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
    fsm_set_state(WorkerState::REPAIRING);
    unit_->rightClick(building);
    return true;
  }
  return false;
}

bool WorkerAgent::assign_to_repair(Unit building) {
  if (is_available_worker()) {
    fsm_set_state(WorkerState::REPAIRING);
    unit_->repair(building);
//    unit_->rightClick(building);
    return true;
  }
  return false;
}

void WorkerAgent::handle_message(act::Message* incoming) {
  if (dynamic_cast<msg::unit::AttackBWUnit*>(incoming)) {
    // This event is also handled by parent baseunit which sets attack target
    fsm_set_state(WorkerState::ATTACKING);
  }
  else if (auto refi = dynamic_cast<msg::worker::RightClickRefinery*>(incoming)) {
    get_unit()->rightClick(refi->refinery_);
    fsm_set_state(WorkerState::GATHER_GAS);
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
