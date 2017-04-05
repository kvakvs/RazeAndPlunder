#include "Commander/Commander.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include "Influencemap/MapManager.h"
#include "Managers/Constructor.h"
#include "Managers/Upgrader.h"
#include "MainAgents/WorkerAgent.h"
#include "Utils/Profiler.h"

#include <algorithm>
#include <iso646.h>

#include "RnpUtil.h"
#include "Glob.h"
#include "RnpConst.h"
#include "RnpRandom.h"
#include "SquadMsg.h"

using namespace BWAPI;

Commander::Commander()
  : AttackDefendFsm(CommanderAttackState::DEFEND)
  , m_find_attack_pos_(), m_time_to_engage_(), build_plan_()
  , squads_()
{
  workers_per_refinery_ = 2;
  workers_num_ = 5;
}

Commander::~Commander() {
}

void Commander::check_buildplan() {
  int have_supply = Broodwar->self()->supplyUsed() / 2;
  auto for_buildings_and_units =
      [](const BuildplanEntry& b) -> bool { // For units/buildings
        UnitType to_build(b.unit_type());
        Constructor::modify([=](Constructor* c) {
            c->add_building(to_build);
          });
        return true;
      };
  auto for_upgrades =
      [](const BuildplanEntry& b) -> bool { // For upgrades
        rnp::upgrader()->add_upgrade(b.upgrade_type());
        return true;
      };
  auto for_tech =
      [](const BuildplanEntry& b) -> bool { // and for the tech
        rnp::upgrader()->add_tech(b.tech_type());
        return true;
      };
  build_plan_.for_each(have_supply,
                       for_buildings_and_units,
                       for_upgrades,
                       for_tech);
}

void Commander::cut_workers_production() {
  workers_num_ = rnp::agent_manager()->get_workers_count();
  Broodwar << "Worker production halted" << std::endl;
}

//void Commander::tick() {
//  rnp::profiler()->start("OnFrame_Commander");
//  on_frame();
//  rnp::profiler()->end("OnFrame_Commander");
//}

size_t Commander::get_preferred_workers_count() const {
  return workers_num_;
}

size_t Commander::get_workers_per_refinery() const {
  return workers_per_refinery_;
}

bool Commander::is_time_to_engage() {
  // Optimize to not have recalculation very often
  if (m_time_to_engage_.value_up_to_date(rnp::seconds(10))) {
    return m_time_to_engage_.value();
  }

  // Check if required squads are ready
  auto loop_result = act::interruptible_for_each_in<Squad>(
      squads_,
      [](const Squad* s) {
          if (s->is_required_for_attack() && not s->is_active()) {
            return act::ForEach::Break;
          }
          return act::ForEach::Continue;
      });
  if (loop_result == act::ForEachResult::Interrupted) {
    return m_time_to_engage_.update(false);
  }

  // Check where to attack
  auto to_attack = find_attack_position();
  if (not rnp::is_valid_position(to_attack)) {
    //No enemy sighted. Dont launch attack.
    return m_time_to_engage_.update(false);
  }

  return m_time_to_engage_.update(true);
}

void Commander::update_squad_goals() {
  TilePosition def_spot(find_chokepoint());

  if (not rnp::is_valid_position(def_spot)) {
    act::for_each_in<Squad>(
        squads_,
        [&def_spot](const Squad* s) {
            msg::squad::defend(s->self(), def_spot);
        });
  }
}

void Commander::debug_show_goal() const {
  act::for_each_in<Squad>(
      squads_,
      [](const Squad* s) {
          s->debug_show_goal();
      });
}

void Commander::tick_base_commander_attack_maybe_defend() {
  bool active_found = std::any_of(
    squads_.begin(), squads_.end(),
    [](auto& s_id) {
      auto s = act::whereis<Squad>(s_id);
      return (s->is_required_for_attack() && s->is_active());
    });

  //No active required squads found.
  //Go back to defend.
  if (not active_found) {
    fsm_set_state(CommanderAttackState::DEFEND);
    TilePosition def_spot = find_chokepoint();
    for (auto& squad_id : squads_) {
      act::modify_actor<Squad>(squad_id, 
                               [=](Squad* s) { s->set_goal(def_spot); });
    }
  }
}

void Commander::tick_base_commander_defend() {
  //Check if we need to attack/kite enemy workers in the base
  auto start_loc = Broodwar->self()->getStartLocation();
  check_workers_attack(rnp::agent_manager()->get_closest_base(start_loc));

  TilePosition def_spot = find_chokepoint();
  if (rnp::is_valid_position(def_spot)) {
    act::for_each_in<Squad>(
      squads_,
      [&def_spot](const Squad* s) {
        if (not s->has_goal()) {
          msg::squad::defend(s->self(), def_spot);
        }
      });
  }
}

void Commander::tick_base_commander_attack() {
  act::for_each_in<Squad>(
    squads_,
    [this](const Squad* s) {
      if (s->is_offensive_squad()) {
        if (not s->has_goal() && s->is_active()) {
          auto to_attack = find_attack_position();
          if (rnp::is_valid_position(to_attack)) {
            act::modify_actor<Squad>(s->self(),
                                     [=](Squad* s) { s->attack(to_attack); });
          }
        }
      } else {
        auto def_spot = find_chokepoint();
        if (rnp::is_valid_position(def_spot)) {
          msg::squad::defend(s->self(), def_spot);
        }
      }
    });
}

void Commander::tick_base_commander() {
  check_buildplan();

  //See if we need to assist a base or worker that is under attack
  if (assist_building()) return;
  if (assist_worker()) return;

  //Check if we shall launch an attack
  switch (fsm_state()) {
  case CommanderAttackState::DEFEND: {
      if (is_time_to_engage()) {
        force_begin_attack();
      }

      tick_base_commander_defend();

      // Attack if we have filled all supply spots
      int supply_used = Broodwar->self()->supplyUsed() / 2;
      if (supply_used >= 198) {
        force_begin_attack();
      }
    } break;
  case CommanderAttackState::ATTACK: {
      // Check if we shall go back to defend
      tick_base_commander_attack_maybe_defend();
      tick_base_commander_attack();
  } break;
  default: ;
  }

  //Check if there are obstacles we can remove. Needed for some maps.
  check_removable_obstacles();

  //Terran only: Check for repairs and finish unfinished buildings
  if (Constructor::is_terran()) {
    //Check if there are unfinished buildings we need to complete.
    check_damaged_buildings();
  }

  //Check for units not belonging to a squad
  check_no_squad_units();
}

void Commander::check_no_squad_units() {
  act::for_each_actor<BaseAgent>(
    [this](const BaseAgent* a) {
      if (a->unit_type().isWorker()) return;
      if (a->is_of_type(UnitTypes::Zerg_Overlord)) return;
      if (a->unit_type().isBuilding()) return;
      if (a->unit_type().isAddon()) return;
      if (a->get_squad_id().is_valid()) return;

      assign_unit(a);
    });
}

void Commander::assign_unit(const BaseAgent* agent) {
  if (squads_.empty()) {
    return;
  }

  //Broodwar << agent->getUnitID() << " (" << agent->getTypeName().c_str() << ") is not assigned to a squad" << endl;
  msg::squad::add_member(agent, squads_);
  
#if 0
  act::interruptible_for_each_actor<Squad>(
      [this,agent,&agent_type](const Squad* squad) {
          if (squad->need_unit(agent_type)) {
            // ask squad to take the agent, if it failed - we will get a 
            // AddMember_Failed message back and should try again
            msg::squad::add_member(squad->self(), 
                                   agent->self(),
                                   self());
            Broodwar << agent->get_unit_id() << " ("
                     << agent->get_type_name().c_str()
                     << ") is assigned to SQ " << squad->get_name() << std::endl;;
            return act::ForEach::Break;
          }
          return act::ForEach::Continue;
      });
#endif
}

TilePosition Commander::find_attack_position() {
  // Optimize: this is called too often
  if (m_find_attack_pos_.value_up_to_date(rnp::seconds(5))) {
    return m_find_attack_pos_.value();
  }

  return m_find_attack_pos_.update(rnp::exploration()->get_random_spotted_building());

//  auto suitable_pos = rnp::map_manager()->find_suitable_attack_position();
//
//  if (rnp::is_valid_position(suitable_pos)) {
//    auto building_pos = rnp::exploration()->get_random_spotted_building();
//
//    if (rnp::is_valid_position(building_pos)) {
////      std::cout << BOT_PREFIX_DEBUG "Attack: Closest spotted building " << building_pos << std::endl;
//      return m_find_attack_pos_.update(building_pos);
//    }
//
////    std::cout << BOT_PREFIX_DEBUG "Attack: Suitable pos " << suitable_pos << std::endl;
//    return m_find_attack_pos_.update(suitable_pos);
//  }
//
////  std::cout << BOT_PREFIX_DEBUG "Attack: no suitable pos" << std::endl;
//  return m_find_attack_pos_.update(rnp::make_bad_position());
}

void Commander::remove_squad(const act::ActorId& id) {
//  act::for_each_alive<Squad>(
//    [](const Squad* s) {
//      msg::squad::disband(s->get_ac_self());
//    });
  msg::squad::disband(id);
  squads_.erase(id);

//  for (size_t i = 0; i < squads_.size(); i++) {
//    auto& sq = squads_[i];
//    if (sq->get_id() == id) {
//      sq->disband();
//      squads_.erase(squads_.begin() + i);
//      return;
//    }
//  }
}

const Squad* Commander::get_squad(const act::ActorId& a) const {
  return act::whereis<Squad>(a);
}

void Commander::add_squad(const act::ActorId& sq) {
  squads_.insert(sq);
  ac_monitor(sq);
}

void Commander::on_unit_destroyed(const act::ActorId& agentid) {
  auto agent = act::whereis<BaseAgent>(agentid);
  if (agent) {
    auto squad_id = agent->get_squad_id();
    auto squad = act::whereis<Squad>(squad_id);
    if (squad) {
      msg::squad::member_destroyed(squad->self(), agentid);
    }
  }
}

struct SortSquadsByPrio {
  bool operator()(const act::ActorId& sq1_id, 
                  const act::ActorId& sq2_id) const {
    auto sq1 = act::whereis<Squad>(sq1_id);
    auto sq2 = act::whereis<Squad>(sq2_id);
    if (sq1->get_priority() != sq2->get_priority()) {
      return sq1->get_priority() < sq2->get_priority();
    }
    return (sq1->is_required_for_attack() && not sq2->is_required_for_attack());
  }
};

act::ActorId::Vector Commander::sort_squad_list() {
  act::ActorId::Vector result;
  result.reserve(squads_.size());
  std::copy(squads_.begin(), squads_.end(), std::back_inserter(result));
  std::sort(result.begin(), result.end(), SortSquadsByPrio());
  return result;
}

void Commander::handle_message(act::Message* incoming) {
    if (auto unew = dynamic_cast<msg::commander::UnitCreated*>(incoming)) {
      on_unit_created(unew->new_);
    }
    else if (auto udead = dynamic_cast<msg::commander::UnitDestroyed*>(incoming)) {
      on_unit_destroyed(udead->dead_);
    }
    else if (auto snew = dynamic_cast<msg::commander::SquadCreated*>(incoming)) {
      squads_.insert(snew->squad_);
    }
    else if (dynamic_cast<msg::commander::UpdateGoals*>(incoming)) {
      update_squad_goals();
    }
    else if (auto rsq = dynamic_cast<msg::commander::RemoveSquad*>(incoming)) {
      remove_squad(rsq->squad_);
    }
    else if (auto bunk = dynamic_cast<msg::commander::BunkerDestroyed*>(incoming)) {
      remove_bunker_squad(bunk->unit_id_);
    }
//    else if (auto amf = dynamic_cast<msg::squad::AddMember_Failed*>(incoming)) {
//      std::cout << "Commander: add sq member failed " 
//        << amf->member_id_.string() << std::endl;
//    }
    else {
      unhandled_message(incoming);
    }
}

void Commander::on_unit_created(const act::ActorId& agentid) {
  //Sort the squad list
  auto sorted_squads = sort_squad_list();

  if (not sorted_squads.empty()) {
    auto agent = act::whereis<BaseAgent>(agentid);
    msg::squad::add_member(agent, sorted_squads);
  }
}

bool Commander::check_workers_attack(const BaseAgent* base) const {
  int no_attack = 0;
  auto base_pos = base->get_unit()->getTilePosition();

  for (auto& u : Broodwar->enemy()->getUnits()) {
    if (u->exists() && u->getType().isWorker()) {
      float dist = rnp::distance(u->getTilePosition(), base_pos);

      if (dist <= 12.0f) {
        //Enemy unit discovered. Attack with some workers.
        act::for_each_actor<BaseAgent>(
          [&no_attack,u](const BaseAgent* a) {
            if (a->is_worker() && no_attack < 1) {
              msg::unit::attack_unit(a->self(), u);
              no_attack++;
            }
          });
      }
    }
  }

  return no_attack > 0;
}

void Commander::check_removable_obstacles() {
  if (removal_done_) return;

  //This method is used to handle the removal of obstacles
  //that is needed on some maps.

  if (Broodwar->mapFileName() == "(2)Destination.scx") {
    Unit mineral = nullptr;
    if (Broodwar->self()->getStartLocation().x == 64) {
      for (auto& u : Broodwar->getAllUnits()) {
        if (u->getType().isResourceContainer() 
            && u->getTilePosition().x == 40 
            && u->getTilePosition().y == 120) {
          mineral = u;
        }
      }
    }
    if (Broodwar->self()->getStartLocation().x == 31) {
      for (auto& u : Broodwar->getAllUnits()) {
        if (u->getType().isResourceContainer()
            && u->getTilePosition().x == 54 
            && u->getTilePosition().y == 6) {
          mineral = u;
        }
      }
    }
    if (mineral != nullptr) {
      if (not rnp::agent_manager()->is_worker_targeting_unit(mineral)) {
        auto start_loc = Broodwar->self()->getStartLocation();
        auto worker = rnp::agent_manager()->find_closest_free_worker(start_loc);
        if (worker != nullptr) {
          worker->get_unit()->rightClick(mineral);
          removal_done_ = true;
        }
      }
    }
  }
}


TilePosition Commander::find_chokepoint() {
  auto bestChoke = rnp::map_manager()->get_defense_location();

  auto guardPos = Broodwar->self()->getStartLocation();
  if (bestChoke != nullptr) {
    guardPos = find_defense_pos(bestChoke);
  }

  return guardPos;
}

TilePosition Commander::find_defense_pos(const BWEM::ChokePoint* choke) {
  TilePosition def_pos(choke->Center());
  TilePosition choke_pos(def_pos);
  TilePosition base_pos = Broodwar->self()->getStartLocation();

  double size = rnp::choke_width(choke);
  if (size <= 32 * 3) {
    //Very narrow chokepoint, dont crowd it
    float best_dist = LIKE_VERY_FAR;
    int max_d = 3;
    int min_d = 2;

    //We found a chokepoint. Now we need to find a good place to defend it.
    for (int c_x = choke_pos.x - max_d; c_x <= choke_pos.x + max_d; c_x++) {
      for (int c_y = choke_pos.y - max_d; c_y <= choke_pos.y + max_d; c_y++) {
        TilePosition c_pos = TilePosition(c_x, c_y);
        if (rnp::exploration()->can_reach(base_pos, c_pos)) {
          auto choke_dist = rnp::distance(choke_pos, c_pos);
          auto base_dist = rnp::distance(base_pos, c_pos);

          if (choke_dist >= min_d && choke_dist <= max_d) {
            if (base_dist < best_dist) {
              best_dist = base_dist;
              def_pos = c_pos;
            }
          }
        }
      }
    }
  } // if size < 

  //Make defenders crowd around defensive structures.
  if (Broodwar->self()->getRace().getID() == Races::Zerg.getID()) {
    UnitType defType;
    if (Constructor::is_zerg()) defType = UnitTypes::Zerg_Sunken_Colony;
    if (Constructor::is_protoss()) defType = UnitTypes::Protoss_Photon_Cannon;
    if (Constructor::is_terran()) defType = UnitTypes::Terran_Bunker;

    auto turret = rnp::agent_manager()->get_closest_agent(def_pos, defType);
    if (turret) {
      TilePosition t_pos = turret->get_unit()->getTilePosition();
      float dist = rnp::distance(t_pos, def_pos);
      if (dist <= 22.0f) {
        def_pos = t_pos;
      }
    }
  }

  return def_pos;
}

bool Commander::is_unit_needed(UnitType type) const {
  int prev_prio = 1000;
  bool result = false;
  auto loop_result = act::interruptible_for_each_in<Squad>(
    squads_,
    [&prev_prio,&result,&type](const Squad* s) {
      if (not s->is_full()) {
        if (s->get_priority() > prev_prio) {
          result = false;
          return act::ForEach::Break;
        }

        if (s->need_unit(type)) {
          result = true;
          return act::ForEach::Break;
        }

        prev_prio = s->get_priority();
      }
      return act::ForEach::Continue;
    });
  if (loop_result == act::ForEachResult::Interrupted) { return result; }
  return false;
}

bool Commander::assist_building() {
  // TODO: Estimate threat power, choose adequate response?
  auto result = false;
  act::interruptible_for_each_actor<BaseAgent>(
    [this,&result](const BaseAgent* a) {
      if (a->is_building() && a->is_under_attack()) {
        
        auto inner_result = act::interruptible_for_each_in<Squad>(
          squads_,
          [a,&result](const Squad* s) {
          bool ok = true;
          if (s->is_explorer_squad()) ok = false;
          if (s->is_bunker_defend_squad()) ok = false;
          if (s->is_rush_squad() && s->is_active()) ok = false;

          if (ok) {
            msg::squad::assist(s->self(), a->get_unit()->getTilePosition());
            result = true;
            return act::ForEach::Break;
          }
          return act::ForEach::Continue;
        }); // each squad

        if (inner_result == act::ForEachResult::Interrupted) {
          return act::ForEach::Break;
        }

      } // is building under attack
      return act::ForEach::Continue;
    });

  if (result) rnp::log()->trace("assist building?");
  return result;
}

bool Commander::assist_worker() {
  auto result = false;
  act::interruptible_for_each_actor<BaseAgent>(
    [this,&result](const BaseAgent* a) {
      if (a->is_worker() && a->is_under_attack()) {
        auto inner_result = act::interruptible_for_each_in<Squad>(
          squads_,
          [a, &result](const Squad* s) {
            bool ok = true;
            if (s->is_explorer_squad()) ok = false;
            if (s->is_bunker_defend_squad()) ok = false;
            if (s->is_rush_squad() && s->is_active()) ok = false;

            if (ok) {
              msg::squad::assist(s->self(), a->get_unit()->getTilePosition());
              result = true;
              return act::ForEach::Break;
            }
            return act::ForEach::Continue;
          });

        if (inner_result == act::ForEachResult::Interrupted) {
          return act::ForEach::Break;
        }

      } // if worker under attack
      return act::ForEach::Continue;
    });

  if (result) rnp::log()->trace("assist worker?");
  return result;
}

void Commander::force_begin_attack() {
  auto attack_goal = find_attack_position();
  if (not rnp::is_valid_position(attack_goal)) {
    rnp::log()->debug("force_begin_attack but no goal");
    return;
  }

  Broodwar << "Launch attack at " << attack_goal << std::endl;
  rnp::log()->debug("Launch attack at {0};{1}", attack_goal.x, attack_goal.y);

  act::for_each_in<Squad>(
    squads_,
    [&attack_goal](const Squad* s) {
      if (s->is_offensive_squad() || s->is_support_squad()) {
        if (rnp::is_valid_position(attack_goal)) {

          act::modify_actor<Squad>(
            s->self(),
            [=](Squad* sq) {
              sq->force_active();
              sq->attack(attack_goal);
            });
        }
      }
    });

  fsm_set_state(CommanderAttackState::ATTACK);
}

void Commander::assist_unfinished_construction(const BaseAgent* base_agent) {
  //First we must check if someone is repairing this building
  if (rnp::agent_manager()->is_any_agent_repairing_this_agent(base_agent)) {
    return;
  }

  auto begin_pos = base_agent->get_unit()->getTilePosition();
  auto rep_unit = rnp::agent_manager()->find_closest_free_worker(begin_pos);
  if (rep_unit != nullptr) {
    auto w = static_cast<const WorkerAgent*>(rep_unit);
    msg::worker::assign_repair(w->self(), base_agent->get_unit());
  }
}

bool Commander::check_damaged_buildings() {
  act::for_each_actor<BaseAgent>(
    [](const BaseAgent* a) {
      if (a->is_building() && a->is_damaged()) {
        auto builder = a->get_unit()->getBuildUnit();
        if (builder == nullptr || !builder->isConstructing()) {
          assist_unfinished_construction(a);
        }
      }
    });
  return false;
}

void Commander::toggle_buildplan_debug() {
  debug_bp_ = !debug_bp_;
}

void Commander::toggle_squads_debug() {
  debug_sq_ = !debug_sq_;
}

void Commander::tick() {
  ProfilerAuto pa(*rnp::profiler(), "OnFrame_Commander");
  tick_base_commander();
}

void Commander::debug_print_info() const {
  if (debug_sq_) {
    int tot_lines = 0;
    auto fn_count_lines = [&tot_lines](const Squad* s) {
          if (s->get_total_units() == 0) return;
          if (s->is_bunker_defend_squad()) return;
          if (s->get_priority() == 1000
            && not s->is_active())
            return;

          tot_lines++;
        };
    act::for_each_in<Squad>(squads_, fn_count_lines);
    if (tot_lines == 0) tot_lines++;

    Broodwar->drawBoxScreen(168, 25, 292, 41 + tot_lines * 16, Colors::Black, true);
    switch (fsm_state()) {
    case CommanderAttackState::DEFEND:
      Broodwar->drawTextScreen(170, 25, "\x03Squads \x07(Defending)");
      break;
    case CommanderAttackState::ATTACK:
      Broodwar->drawTextScreen(170, 25, "\x03Squads \x08(Attacking)");
      break;
    }
    Broodwar->drawLineScreen(170, 39, 290, 39, Colors::Orange);
    int no = 0;
    act::for_each_in<Squad>(
      squads_,
      [&no](const Squad* s) {
        bool vis = true;
        if (s->get_total_units() == 0) vis = false;
        if (s->is_bunker_defend_squad()) vis = false;
        if (s->get_priority() == 1000 && not s->is_active()) vis = false;

        if (vis) {
          int cSize = s->get_squad_size();
          int totSize = s->get_total_units();

          if (s->is_required_for_attack()) {
            if (cSize < totSize) {
              Broodwar->drawTextScreen(
                170, 41 + no * 16, "*%s: \x18(%d/%d)",
                s->string().c_str(), s->get_squad_size(),
                s->get_total_units());
            } 
            else {
              Broodwar->drawTextScreen(
                170, 41 + no * 16, "*%s: \x07(%d/%d)", 
                s->string().c_str(), s->get_squad_size(), 
                s->get_total_units());
            }
            no++;
          }
          else {
            if (cSize < totSize) {
              Broodwar->drawTextScreen(
                170, 41 + no * 16, "%s: \x18(%d/%d)",
                s->string().c_str(), s->get_squad_size(),
                s->get_total_units());
            }
            else {
              Broodwar->drawTextScreen(
                170, 41 + no * 16, "%s: \x07(%d/%d)", 
                s->string().c_str(), s->get_squad_size(), 
                s->get_total_units());
            }
            no++;
          }
        }
      });
    if (no == 0) no++;
    Broodwar->drawLineScreen(170, 40 + no * 16, 290, 40 + no * 16, Colors::Orange);
  }

  if (debug_bp_ && not build_plan_.empty()) {
    build_plan_.debug_print_info();
    rnp::constructor()->debug_print_info();
  }
}

// static
act::ActorId Commander::add_bunker_squad() {
  auto b_squad = act::spawn<Squad>(ActorFlavour::Squad,
                                   Squad::SquadType::BUNKER, "Bunker", 5);
  msg::squad::add_setup(b_squad, UnitTypes::Terran_Marine, 4);
  msg::commander::squad_created(b_squad);

#if 0
  //Try to fill from other squads.
  int added = 0;
  act::for_each_in<Squad>(
    squads_,
    [](const Squad* s) {
      if (s->is_offensive_squad() || s->is_defensive_squad()) {
        for (int i = 0; i < 4 - added; i++) {
          if (s->has_units(UnitTypes::Terran_Marine, 1)) {
            if (added < 4) {
              BaseAgent* ma = s->remove_member(UnitTypes::Terran_Marine);
              if (ma != nullptr) {
                added++;
                b_squad->add_member(ma);
                ma->clear_goal();
              }
            }
          }
        }
      }
    });
#endif //0
  return b_squad;
}

bool Commander::remove_bunker_squad(int unit_id) {
  auto loop_result = act::interruptible_for_each_in<Squad>(
    squads_,
    [unit_id](const Squad* s) {
      if (s->is_bunker_defend_squad()) {
        if (s->get_bunker_id() == unit_id) {
          msg::commander::remove_squad(s->self());
          return act::ForEach::Break;
        }
      }
      return act::ForEach::Continue;
    });
  return loop_result == act::ForEachResult::Interrupted;
}
