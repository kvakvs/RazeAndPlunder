#include "Constructor.h"
#include "StructureAgents/StructureAgent.h"
#include "MainAgents/BaseAgent.h"
#include "MainAgents/WorkerAgent.h"
#include "AgentManager.h"
#include "BuildingPlacer.h"
#include "ResourceManager.h"
#include "Glob.h"
#include "RnpUtil.h"
#include "Utils/Profiler.h"

using namespace BWAPI;
#define MODULE_PREFIX "(constructor) "

Constructor::Constructor(): plan_(), queue_() {
  last_call_frame_ = Broodwar->getFrameCount();
}

Constructor::~Constructor() {
}

void Constructor::on_building_destroyed(Unit building) {
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
    plan_.push_front(UnitTypes::Zerg_Spore_Colony);
    return;
  }
  plan_.push_front(building->getType());
}

void Constructor::tick_manage_buildplan(int frame) {
  if (rnp::agent_manager()->get_workers_count() == 0) {
    //No workers so cant do anything
    return;
  }

  //Check if we have possible "locked" items in the buildqueue
  for (size_t i = 0; i < queue_.size(); i++) {
    auto& qitem = queue_.item(i);
    int elapsed = frame - qitem.assigned_frame_;
    if (elapsed >= 2000) {
      //Reset the build request
      auto am = rnp::agent_manager();
      auto w_id = am->get_agent(qitem.assigned_worker_id_);
      auto worker = static_cast<const WorkerAgent*>(w_id);
      if (worker) {
        act::modify_actor<WorkerAgent>(
          worker->self(),
          [](WorkerAgent* w) {
            rnp::log()->trace(MODULE_PREFIX "reset worker");
            w->reset();
          });
      }
      plan_.push_front(qitem.to_build_);
      rnp::resources()->unlock_resources(qitem.to_build_);
      queue_.remove(i);
      return;
    }
  }

  //Check if we can build next building in the buildplan
  if (not plan_.empty()) {
    execute_order(plan_.item(0));
  }
}

void Constructor::tick() {
  ProfilerAuto pa(*rnp::profiler(), "OnFrame_Constructor");

  //Check if we need more supply buildings
  if (is_terran() || is_protoss()) {
    if (shall_build_supply()) {
      auto supply_provider(Broodwar->self()->getRace().getSupplyProvider());
      plan_.push_front(supply_provider);
    }
  }

  //Check if we need to expand
  if (not has_resources_left()) {
    expand(Broodwar->self()->getRace().getResourceDepot());
  }

  if (plan_.empty() && queue_.empty()) {
    //Nothing to do
    return;
  }

  //Dont call too often
  int frame = Broodwar->getFrameCount();
  if (frame - last_call_frame_ < 10) {
    return;
  }
  last_call_frame_ = frame;

  tick_manage_buildplan(frame);
}

bool Constructor::has_resources_left() {
  size_t total_minerals_left = 0;

  act::for_each_actor<BaseAgent>(
    [this,&total_minerals_left](const BaseAgent* a) {
      if (a->unit_type().isResourceDepot()) {
        total_minerals_left += minerals_nearby(a->get_unit()->getTilePosition());
      }
    });

  if (total_minerals_left <= 5000) {
    return false;
  }
  return true;
}

size_t Constructor::minerals_nearby(TilePosition center) const {
  size_t mineral_cnt = 0;

  for (auto& u : Broodwar->getMinerals()) {
    if (u->exists()) {
      float dist = rnp::distance(center, u->getTilePosition());
      if (dist <= 10.0f) {
        mineral_cnt += u->getResources();
      }
    }
  }

  return mineral_cnt;
}

bool Constructor::shall_build_supply() {
  UnitType supply = Broodwar->self()->getRace().getSupplyProvider();

  //Check if we need supplies
  int supply_total = Broodwar->self()->supplyTotal() / 2;
  int supply_used = Broodwar->self()->supplyUsed() / 2;

  int pre_diff = 2;
  //Speed up supply production in middle/late game
  if (supply_used > 30) pre_diff = 4;

  if (supply_used <= supply_total - pre_diff) {
    return false;
  }
  //Don't use automatic supply adding in the early game
  //to make it a bit more controlled.
  if (supply_used <= 30) {
    return false;
  }

  //Check if we have reached max supply
  if (supply_total >= 200) {
    return false;
  }

  //Check if there already is a supply in the list
  if (is_next_of_type(supply)) {
    return false;
  }

  //Check if we are already building a supply
  if (supply_being_built()) {
    return false;
  }

  return true;
}

bool Constructor::supply_being_built() {
  //Zerg
  if (is_zerg()) {
    if (get_in_production_count(UnitTypes::Zerg_Overlord) > 0) {
      return true;
    }
    return false;
  }

  //Terran and Protoss
  UnitType supply = Broodwar->self()->getRace().getSupplyProvider();

  //1. Check if we are already building a supply
  auto loop_result = act::interruptible_for_each_actor<BaseAgent>(
      [&supply](const BaseAgent* a) {
          if (a->unit_type().getID() == supply.getID()) {
            if (a->get_unit()->isBeingConstructed()) {
              //Found one that is being constructed
              return act::ForEach::Break;
            }
          }
          return act::ForEach::Continue;
      });
  if (loop_result == act::ForEachResult::Interrupted) { return true; }

  //2. Check if we have a supply in build queue
  for (size_t i = 0; i < queue_.size(); i++) {
    if (queue_.item(i).to_build_.getID() == supply.getID()) {
      return true;
    }
  }

  return false;
}

void BuildQueue::remove(size_t i) {
  auto& qitem = item(i);
  //rnp::log()->trace(MODULE_PREFIX "queue remove {}", 
  //                  rnp::remove_race(qitem.to_build_));
  q_items_.erase(q_items_.begin() + i);
}

void BuildQueue::emplace_back(const BWAPI::UnitType& tb, int assignedf, int workerid) {
  //rnp::log()->trace(MODULE_PREFIX "queue emp_back {}", rnp::remove_race(tb));
  q_items_.emplace_back(tb, assignedf, workerid);
}

void ConstructionPlan::push_front(const BWAPI::UnitType& ut) {
  //rnp::log()->trace(MODULE_PREFIX "plan push_front {}", rnp::remove_race(ut));
  units_.insert(units_.begin(), ut);
}

void ConstructionPlan::remove(size_t i) {
  auto& ut = item(i);
  //rnp::log()->trace(MODULE_PREFIX "plan remove {}", rnp::remove_race(ut));
  units_.erase(units_.begin() + i);
}

void Constructor::lock(int build_plan_index, int unit_id) {
  auto& type = plan_.item(build_plan_index);
  //rnp::log()->trace(MODULE_PREFIX "lock {}", type.toString());
  plan_.remove(build_plan_index);

  queue_.emplace_back(type, Broodwar->getFrameCount(), unit_id);
}

void Constructor::remove(UnitType type) {
  for (size_t i = 0; i < plan_.size(); i++) {
    if (plan_.item(i).getID() == type.getID()) {
      plan_.remove(i);
      return;
    }
  }
}

void Constructor::unlock(UnitType type) {
  for (size_t i = 0; i < queue_.size(); i++) {
    if (queue_.item(i).to_build_.getID() == type.getID()) {
      queue_.remove(i);
      return;
    }
  }
}

void Constructor::handle_worker_destroyed(UnitType type, int workerID) {
  for (size_t i = 0; i < queue_.size(); i++) {
    if (queue_.item(i).assigned_worker_id_ == workerID) {
      queue_.remove(i);
      plan_.push_front(type);
      rnp::resources()->unlock_resources(type);
    }
  }
}

bool Constructor::zerg_drone_morph(UnitType target, UnitType evolved) {
  auto start_loc = Broodwar->self()->getStartLocation();
  auto agent = rnp::agent_manager()->get_closest_agent(start_loc, target);
  if (agent) {
    auto s_agent = static_cast<const StructureAgent*>(agent);
    if (s_agent->can_morph_into(evolved)) {
      s_agent->get_unit()->morph(evolved);
      lock(0, s_agent->get_unit_id());
      return true;
    }
  }
  else {
    //No building available that can do this morph.
    remove(evolved);
  }
  return false;
}

bool Constructor::execute_order(const UnitType& type) {
  //Max 5 concurrent buildings allowed at the same time
  if (queue_.size() >= 5) {
    return false;
  }

  //Check if we meet requirements for the building
  std::map<UnitType, int> reqs = type.requiredUnits();
  for (auto& j: reqs) {
    if (not rnp::agent_manager()->have_a_completed_building(j.first)) {
      return false;
    }
  }

  if (type.isResourceDepot()) {
    TilePosition pos = rnp::building_placer()->find_expansion_site();
    if (not rnp::is_valid_position(pos)) {
      //No expansion site found.
      if (not plan_.empty()) plan_.remove(0);
      return true;
    }
  }
  if (type.isRefinery()) {
    TilePosition rSpot = rnp::building_placer()->search_refinery_spot();
    if (not rnp::is_valid_position(rSpot)) {
      //No buildspot found
      if (not plan_.empty()) plan_.remove(0);
      return true;
    }
  }
  if (is_zerg()) {
    std::pair<UnitType, int> builder = type.whatBuilds();
    if (builder.first.getID() != UnitTypes::Zerg_Drone.getID()) {
      //Needs to be morphed
      if (zerg_drone_morph(builder.first, type)) {
        return true;
      }
      else {
        return false;
      }
    }
  }

  //Check if we have resources
  if (not rnp::resources()->has_resources(type)) {
    return false;
  }

  //Check if we have a free worker
  bool found = false;
  auto start_loc = Broodwar->self()->getStartLocation();
  auto a = rnp::agent_manager()->find_closest_free_worker(start_loc);
  if (a) {
    // the worker's message handler will handle both success and failure
    msg::worker::assign_build(a->self(), type);
  }

  return false;
}

bool Constructor::is_terran() {
  return Broodwar->self()->getRace().getID() == Races::Terran.getID();
}

bool Constructor::is_protoss() {
  return Broodwar->self()->getRace().getID() == Races::Protoss.getID();
}

bool Constructor::is_zerg() {
  return Broodwar->self()->getRace().getID() == Races::Zerg.getID();
}

void Constructor::handle_message(act::Message* incoming) {
  unhandled_message(incoming);
}

bool Constructor::is_next_building_expand() {
  if (not plan_.empty()) {
    if (plan_.item(0).isResourceDepot()) return true;
  }
  return false;
}

void Constructor::add_refinery() {
  //Don't add if we already have enough.
  UnitType ref = Broodwar->self()->getRace().getRefinery();
  int no = rnp::agent_manager()->get_units_of_type_count(ref);
  if (no >= 4) return;

  UnitType refinery = Broodwar->self()->getRace().getRefinery();

  if (not this->is_next_of_type(refinery)) {
    plan_.push_front(refinery);
  }
}

void Constructor::command_center_built() {
  last_command_center_ = Broodwar->getFrameCount();
}

void Constructor::debug_print_info() const {
  size_t tot_lines = plan_.size() + queue_.size();
  if (plan_.empty()) tot_lines++;
  if (queue_.empty()) tot_lines++;

  if (tot_lines > 0) {
    Broodwar->drawBoxScreen(488, 25, 602, 62 + tot_lines * 16, Colors::Black, true);
    Broodwar->drawTextScreen(490, 25, "\x03Next to build");
    Broodwar->drawLineScreen(490, 39, 600, 39, Colors::Orange);

    size_t no = 0;
    for (size_t i = 0; i < plan_.size(); i++) {
      Broodwar->drawTextScreen(490, 40 + no * 16, 
                               rnp::remove_race(plan_.item(i)).c_str());
      no++;
    }
    if (no == 0) no++;
    Broodwar->drawLineScreen(490, 40 + no * 16, 600, 40 + no * 16, Colors::Orange);

    size_t s = 40 + no * 16;
    Broodwar->drawTextScreen(490, s + 2, "\x03In progress");
    Broodwar->drawLineScreen(490, s + 19, 600, s + 19, Colors::Orange);

    no = 0;
    for (size_t i = 0; i < queue_.size(); i++) {
      Broodwar->drawTextScreen(490, s + 20 + no * 16, 
                               rnp::remove_race(queue_.item(i).to_build_).c_str());
      no++;
    }
    if (no == 0) no++;
    Broodwar->drawLineScreen(490, s + 20 + no * 16, 600, s + 20 + no * 16, Colors::Orange);
  }
}

void Constructor::handle_no_buildspot_found(UnitType to_build) {
  bool remove_order = false;

  if (to_build.getID() == UnitTypes::Protoss_Photon_Cannon) remove_order = true;
  if (to_build.getID() == UnitTypes::Terran_Missile_Turret) remove_order = true;
  if (to_build.isAddon()) remove_order = true;
  if (to_build.getID() == UnitTypes::Zerg_Spore_Colony) remove_order = true;
  if (to_build.getID() == UnitTypes::Zerg_Sunken_Colony) remove_order = true;
  if (to_build.isResourceDepot()) remove_order = true;
  if (to_build.isRefinery()) remove_order = true;

  if (remove_order) {
    remove(to_build);
  }

  if (not remove_order) {
    if (is_protoss() && not supply_being_built()) {
      //Insert a pylon to increase PSI coverage
      if (not is_next_of_type(UnitTypes::Protoss_Pylon)) {
        rnp::log()->trace(MODULE_PREFIX "pylon inserted");
        plan_.push_front(UnitTypes::Protoss_Pylon);
      }
    }
  }
}

bool Constructor::is_next_of_type(UnitType type) const {
  if (plan_.empty()) {
    return false;
  }
  if (plan_.item(0).getID() == type.getID()) {
    return true;
  }
  return false;
}

bool Constructor::contains_type(UnitType type) const {
  for (size_t i = 0; i < plan_.size(); i++) {
    if (plan_.item(i).getID() == type.getID()) {
      return true;
    }
  }
  for (size_t i = 0; i < queue_.size(); i++) {
    if (queue_.item(i).to_build_.getID() == type.getID()) {
      return true;
    }
  }
  return false;
}

bool Constructor::is_covered_by_detector(TilePosition pos) {
  auto loop_result = act::interruptible_for_each_actor<BaseAgent>(
      [&pos](const BaseAgent* a) {
          UnitType type = a->unit_type();
          if (type.isDetector() && type.isBuilding()) {
            double range = type.sightRange() * 1.5;
            auto unit_pos = a->get_unit()->getPosition();
            auto dist = unit_pos.getDistance(Position(pos));
            if (dist <= range) {
              return act::ForEach::Break;
            }
          }
          return act::ForEach::Continue;
      });
  return loop_result == act::ForEachResult::Interrupted;
}

void Constructor::add_building(UnitType type) {
  //rnp::log()->trace(MODULE_PREFIX "add building {}", type.toString());
  plan_.push_back(type);
}

void Constructor::add_building_first(UnitType type) {
  //rnp::log()->trace(MODULE_PREFIX "add building (first) {}", type.toString());
  plan_.push_front(type);
}

void Constructor::expand(UnitType command_center_unit) {
  if (is_being_built(command_center_unit)) {
    return;
  }

  if (contains_type(command_center_unit)) {
    return;
  }

  TilePosition pos = rnp::building_placer()->find_expansion_site();
  if (not rnp::is_valid_position(pos)) {
    //No expansion site found.
    rnp::log()->trace(MODULE_PREFIX "no expansion site");
    return;
  }

  plan_.push_front(command_center_unit);
}

bool Constructor::need_building(UnitType type) const {
  if (rnp::agent_manager()->have_a_completed_building(type)) return false;
  if (is_being_built(type)) return false;
  if (contains_type(type)) return false;

  return true;
}

bool Constructor::is_being_built(UnitType type) const {
  auto loop_result = act::interruptible_for_each_actor<BaseAgent>(
      [&type](const BaseAgent* a) {
          if (a->is_of_type(type) && a->get_unit()->isBeingConstructed()) {
            return act::ForEach::Break;
          }
          return act::ForEach::Continue;
      });
  return loop_result == act::ForEachResult::Interrupted;
}

size_t Constructor::get_in_production_count(UnitType type) const {
  size_t no = 0;

  act::for_each_actor<BaseAgent>(
      [&no,&type](const BaseAgent* a) {
          if (a->unit_type().canProduce()
              && not a->get_unit()->isBeingConstructed()) {
            for (auto& u : a->get_unit()->getTrainingQueue()) {
              if (u.getID() == type.getID()) {
                no++;
              }
            }
          }
      });

  if (is_zerg()) {
    for (auto& u : Broodwar->self()->getUnits()) {
      if (u->exists()) {
        if (u->getType().getID() == UnitTypes::Zerg_Egg.getID()) {
          if (u->getBuildType().getID() == type.getID()) {
            no++;
            if (type.isTwoUnitsInOneEgg()) no++;
          }
        }
      } // if exists
    } // each unit
  } // if zerg

  return no;
}

#undef MODULE_PREFIX 
