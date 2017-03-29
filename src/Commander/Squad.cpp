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
#include "Commander/SquadMsg.h"

using namespace BWAPI;

Squad::Squad(SquadType mType, std::string mName, int mPriority)
: members_(), setup_(), goal_(), path_(), path_iter_(), name_("unnamed")
{
  type_ = mType;
  name_ = mName;
  active_priority_ = priority_ = mPriority;
  goal_ = rnp::make_bad_position();
}

size_t Squad::get_max_size() {
  size_t count = 0;
  for (size_t i = 0; i < setup_.size(); i++) {
    count += setup_[i].count_;
  }
  return count;
}

void Squad::add_setup(UnitType type, int no) {
  rnp::log()->debug("{}: add_setup {} x{}", string(), type.toString(), no);

  //First, check if we have the setup already
  for (int i = 0; i < (int)setup_.size(); i++) {
    if (setup_[i].type_.getID() == type.getID()) {
      //Found, increase the amount
      setup_[i].count_ += no;
      return;
    }
  }

  //Not found, add as new
  UnitSetup us;
  us.type_ = type;
  us.count_ = no;
  us.current_count_ = 0;
  setup_.push_back(us);

  if (not type.isFlyer()) {
    move_type_ = MoveType::GROUND;
  }
}

void Squad::remove_setup(UnitType type, int no) {
  for (size_t i = 0; i < setup_.size(); i++) {
    if (setup_[i].type_.getID() == type.getID()) {
      //Found, reduce the amount
      setup_[i].count_ -= no;
      if (setup_[i].count_ < 0) setup_[i].count_ = 0;
      
      int to_remove = setup_[i].current_count_ - setup_[i].count_;
      for (int j = 0; j < to_remove; j++) {
        remove_member(setup_[i].type_);
      }
      return;
    }
  }
}

void Squad::debug_show_goal() const {
  if (is_bunker_defend_squad()) return;

  if (not empty() && goal_.x >= 0) {
    Position a = Position(goal_.x * 32 + 16, goal_.y * 32 + 16);

    Broodwar->drawCircleMap(a.x - 3, a.y - 3, 6, Colors::Grey, true);
    Broodwar->drawTextMap(a.x - 20, a.y - 5, "\x03Goal %s", name_.c_str());
  }
}

void Squad::tick() {
  if (not active_) {
    if (is_full() && not buildup_) {
      active_ = true;
    }
  }

  if (active_) {
    if (active_priority_ != priority_) {
      priority_ = active_priority_;
    }
  }

  //Check if we need workers in the squad
  auto start_loc = Broodwar->self()->getStartLocation();
  auto agent_mgr = rnp::agent_manager();
  for (size_t i = 0; i < setup_.size(); i++) {
    if (setup_[i].current_count_ < setup_[i].count_ 
      && setup_[i].type_.isWorker()) 
    {
      int no = setup_[i].count_ - setup_[i].current_count_;
      for (int j = 0; j < no; j++) {
        auto w = agent_mgr->find_closest_free_worker(start_loc);
        if (w) {
          add_member(w->self());
        }
      }
    }
  }
}

bool Squad::is_attacking() const {
  if (is_explorer_squad()) return false;

  bool result = false;
  try {
    act::for_each_in<BaseAgent>(
        members_,
        [&result](const BaseAgent* a) {
            if (a->get_unit()->isAttacking()) { result = true; }
            else if (a->get_unit()->isStartingAttack()) { result = true; }
        });
  }
  catch (std::exception) {
  }

  return result;
}

bool Squad::is_under_attack() {
  // TODO: catch under attack event?
  return is_attacking();
}

bool Squad::need_unit(UnitType type) const {
  //1. Check if prio is set to Inactive squad.
  if (priority_ >= 1000) {
    return false;
  }

  int no_created = 1;
  if (Constructor::is_zerg()) {
    if (type.isTwoUnitsInOneEgg()) {
      no_created = 2;
    }
  }

  //2. Check setup
  for (size_t i = 0; i < setup_.size(); i++) {
    if (setup_[i].equals(type)) {
      //Found a matching setup, see if there is room
      if (setup_[i].current_count_
        + rnp::constructor()->get_in_production_count(type) 
        + no_created <= setup_[i].count_) 
      {
        return true;
      }
    }
  }

  return false;
}

bool Squad::add_member(const act::ActorId& agent_ac_id) {
  auto agent = act::whereis<BaseAgent>(agent_ac_id);
  if (not agent) {
    rnp::log()->error("{0} addm: not exists {1}", 
                      string(), agent_ac_id.string());
    return false;
  }

  if (priority_ >= 1000) {
    //Check if prio is above Inactive squad.
    rnp::log()->error("{0} addm: prio greater than inactive {1}",
                      string(), agent_ac_id.string());
    return false;
  }

  //Step 1. Check if the agent already is in the squad
  if (rnp::contains(members_, agent_ac_id)) {
    //Remove it, and add again to update the types.
    //Needed for morphing units like Lurkers.
    remove_member(agent);
  }

  //Step 2. Check if we have room for this type of agent.
  for (size_t i = 0; i < setup_.size(); i++) {
    if (setup_[i].equals(agent->unit_type())) {
      //Found a matching setup, see if there is room
      if (setup_[i].current_count_ < setup_[i].count_) {
        //Yes we have, add it to the squad
        members_.insert(agent_ac_id);
        ac_monitor(agent_ac_id);

        // Also inform the agent that it belongs to squad now
        msg::unit::joined_squad(agent->self(), this->self(), goal_);

        setup_[i].current_count_++;
        return true;
      }
    }
  }

  return false;
}

void Squad::debug_print_info() const {
  int sx = 440;
  int sy = 30;
  int w = 180;
  int h = 120 + 15 * setup_.size();

  Broodwar->drawBoxScreen(sx - 2, sy, sx + w + 2, sy + h, Colors::Black, true);
  Broodwar->drawTextScreen(sx + 4, sy, "\x03%s", name_.c_str());
  Broodwar->drawLineScreen(sx, sy + 14, sx + w, sy + 14, Colors::Orange);

  auto id_s = self().string();
  Broodwar->drawTextScreen(sx + 2, sy + 15, "Id: \x11%s", id_s.c_str());
  Broodwar->drawTextScreen(sx + 2, sy + 30, "Goal: \x11(%d,%d)", goal_.x, goal_.y);

  std::string str1 = "Ground ";
  if (is_air()) str1 = "Air ";
  if (setup_.empty()) str1 = "";

  std::string str2 = "";
  if (is_offensive_squad()) str2 = "Offensive";
  if (is_defensive_squad()) str2 = "Defensive";
  if (is_bunker_defend_squad()) str2 = "Bunker";
  if (is_explorer_squad()) str2 = "Explorer";
  if (is_rush_squad()) str2 = "Rush";

  Broodwar->drawTextScreen(sx + 2, sy + 45, "Type: \x11%s%s", str1.c_str(), str2.c_str());
  Broodwar->drawTextScreen(sx + 2, sy + 60, "Priority: \x11%d", active_priority_);

  if (required_) Broodwar->drawTextScreen(sx + 2, sy + 75, "Required: \x07Yes");
  else Broodwar->drawTextScreen(sx + 2, sy + 75, "Required: \x18No");

  if (is_full()) Broodwar->drawTextScreen(sx + 2, sy + 90, "Full: \x07Yes");
  else Broodwar->drawTextScreen(sx + 2, sy + 90, "Full: \x18No");

  if (is_active()) Broodwar->drawTextScreen(sx + 2, sy + 105, "Active: \x07Yes");
  else Broodwar->drawTextScreen(sx + 2, sy + 105, "Active: \x18No");

  Broodwar->drawLineScreen(sx, sy + 119, sx + w, sy + 119, Colors::Orange);
  int no = 0;
  for (size_t i = 0; i < setup_.size(); i++) {
    std::string name = rnp::remove_race(setup_[i].type_.getName());
    Broodwar->drawTextScreen(sx + 2, sy + 120 + 15 * no, 
      "%s \x11(%d/%d)", name.c_str(), setup_[i].current_count_, setup_[i].count_);
    no++;
  }
  Broodwar->drawLineScreen(sx, sy + 119 + 15 * no, sx + w, sy + 119 + 15 * no, Colors::Orange);
}

bool Squad::is_full() const {
  if (setup_.empty()) return false;

  //1. Check setup
  auto all_full = std::all_of(setup_.begin(), setup_.end(),
                              [](const UnitSetup& s) {
                                return s.current_count_ >= int(s.count_ * 0.85f);
                              });
  if (not all_full) { return false; }

  //2. Check that all units are alive and ready
  try {
    auto all_alive = std::all_of(members_.begin(), members_.end(),
                                 [](const act::ActorId& a_id) {
                                   auto a = act::whereis<BaseAgent>(a_id);
                                   return a->is_alive()
                                       && a->get_unit()
                                       && not a->get_unit()->isBeingConstructed();
                                 });
    if (not all_alive) { return false; }
  }
  catch (std::exception) {
    return false;
  }

  //3. Check if some morphing is needed
  if (morphs_.getID() != UnitTypes::Unknown.getID()) {
    for (auto& actor_id : members_) {
      auto actor = act::whereis<BaseAgent>(actor_id);
      if (not actor) continue;

      if (morphs_.getID() == UnitTypes::Zerg_Lurker.getID() 
        && actor->is_of_type(UnitTypes::Zerg_Hydralisk)) {
        return false;
      }
      if (morphs_.getID() == UnitTypes::Zerg_Devourer.getID() 
        && actor->is_of_type(UnitTypes::Zerg_Mutalisk)) {
        return false;
      }
      if (morphs_.getID() == UnitTypes::Zerg_Guardian.getID() 
        && actor->is_of_type(UnitTypes::Zerg_Mutalisk)) {
        return false;
      }
    }
  }

  return true;
}

void Squad::remove_member(const BaseAgent* agent) {
  //Step 1. Remove the agent instance

//  auto start_loc = Broodwar->self()->getStartLocation();
//  for (auto& actor_id : members_) {
//    if (actor->get_unit_id() == agent->get_unit_id()) {
//      actor->set_squad_id(-1);
//      actor->set_goal(start_loc);
//      members_.erase(actor_id);
//      break;
//    }
//  }
  auto& agent_id = agent->self();
  msg::unit::left_squad(agent_id, self());
  members_.erase(agent_id);

  //Step 2. Update the setup list
  for (size_t i = 0; i < setup_.size(); i++) {
    if (setup_[i].equals(agent->unit_type())) {
      setup_[i].current_count_--;

    }
  }

  //Step 3. If Explorer, set destination as explored (to avoid being killed at the same
  //place over and over again).
  if (is_explorer_squad()) {
    TilePosition goal = agent->get_goal();
    if (rnp::is_valid_position(goal)) {
      ExplorationManager::modify(
        [goal](ExplorationManager* e) { e->set_explored(goal); }
      );
    }
  }

  //See if squad should be set to inactive (due to too many dead units)
  if (active_) {
    size_t alive_count = 0;
    act::for_each_in<BaseAgent>(members_,
                                [&alive_count](const BaseAgent* actor) {
                                  if (actor->is_alive()) {
                                    alive_count++;
                                  }
                                });

    if (alive_count <= get_max_size() / 10) {
      active_ = false;
    }
  }
}

void Squad::disband() {
  //Remove setup
  for (int i = 0; i < (int)setup_.size(); i++) {
    setup_[i].count_ = 0;
    setup_[i].current_count_ = 0;
  }

  //Remove agents (if not Bunker Squad)
  if (not this->is_bunker_defend_squad()) {
    for (auto& agent_id : members_) {
      msg::unit::left_squad(agent_id, self());
    }
  }
  members_.clear();
}

act::ActorId Squad::remove_member(UnitType type) {
  const BaseAgent* agent = nullptr;

  act::interruptible_for_each_in<BaseAgent>(
    members_,
    [&type,&agent](const BaseAgent* actor) {
      if (UnitSetup::equals(actor->unit_type(), type)) {
        agent = actor;
        return act::ForEach::Break;
      }
      return act::ForEach::Continue;
    });

  if (agent) {
    remove_member(agent);
  }

  return agent->self();
}

void Squad::defend(TilePosition m_goal) {
  if (not rnp::is_valid_position(m_goal)) return;

  if (current_state_ != State::DEFEND) {
    if (current_state_ == State::ASSIST && not is_under_attack()) {
      current_state_ = State::DEFEND;
    }
  }
  set_goal(m_goal);
}

void Squad::attack(TilePosition m_goal) {
  if (not rnp::is_valid_position(m_goal)) return;

  if (current_state_ != State::ATTACK) {
    if (not is_under_attack()) {
      if (is_active()) {
        current_state_ = State::ATTACK;
      }
    }
  }

  if (is_active()) {
    set_goal(m_goal);
  }
}

void Squad::assist(TilePosition m_goal) {
  if (not rnp::is_valid_position(m_goal)) return;

  if (current_state_ != State::ASSIST) {
    if (not is_under_attack()) {
      Broodwar << "SQ " << name_ << " assist at " << m_goal << std::endl;
      current_state_ = State::ASSIST;
      set_goal(m_goal);
    }
  }
}

void Squad::set_goal(TilePosition m_goal) {
  if (is_attacking()) {
    if (not rnp::is_valid_position(goal_)) {
      return;
    }
  }

  if (m_goal.x != goal_.x || m_goal.y != goal_.y) {
    goal_set_frame_ = Broodwar->getFrameCount();
    if (is_ground()) {
      if (not members_.empty()) {
        path_ = Pathfinder::get_path(get_center(), m_goal);
        path_iter_ = path_->begin();
        arrived_frame_ = -1;
      }
    }

    this->goal_ = m_goal;
    set_member_goals(goal_);
  }
}

TilePosition Squad::next_follow_move_position() const {
  // TODO: implement proper memory of squad position X=20? tiles ago
  if (not path_iter_ || path_iter_->is_finished()) {
    return goal_;
  }

  return path_iter_->current_;
}

TilePosition Squad::next_move_position() const {
  if (not path_iter_ || path_iter_->is_finished()) {
    return goal_;
  }
  if (is_air()) {
    return goal_;
  }

  if (arrived_frame_ == -1) {
    auto loop_result = act::interruptible_for_each_in<BaseAgent>(
        members_,
        [this](const BaseAgent* a) {
            return act::ForEach::Continue;
            //Check if we have arrived at a checkpoint. For mixed squads,
            //air units does not count as having arrived.
            if (is_ground() 
                && not a->unit_type().isFlyer()
                || is_air()) 
            {
              float seek_dist = a->unit_type().sightRange() / 2.0f;
              Position path_step_pos(path_iter_->current_);
              auto unit_pos(a->get_unit()->getPosition());
              float dist = rnp::distance(path_step_pos, unit_pos);

              if (dist <= seek_dist) {
                auto now = Broodwar->getFrameCount();
                Squad::modify(self(),
                              [now](Squad* s) { s->set_arrived_frame(now); });
                return act::ForEach::Break;
              }
            }
            return act::ForEach::Continue;
        });
  }

//  if (arrived_frame_ != -1) {
//    int cFrame = Broodwar->getFrameCount();
//    if (cFrame - arrived_frame_ >= 200) //100
//    {
//      path_index_ += 20; //20
//      if (path_index_ >= (int)path_.size()) {
//        path_index_ = (int)path_.size() - 1;
//      }
//      arrived_frame_ = -1;
//    }
//  }

  auto member_goal = path_iter_->next();
  msg::squad::set_goal(self(), member_goal);

  return member_goal;
}

void Squad::clear_goal() {
  this->goal_ = rnp::make_bad_position();
  set_member_goals(goal_);
}

void Squad::set_member_goals(TilePosition c_goal) {
  if (is_bunker_defend_squad()) return;

  for (auto& m_id : members_) {
    msg::unit::set_goal(m_id, c_goal);
  }
}

bool Squad::has_goal() const {
  int elapsed = Broodwar->getFrameCount() - goal_set_frame_;
  if (elapsed >= 600) {
    if (not is_attacking()) {
      // tell self, that it is time to forget the goal
      msg::squad::set_goal(self(), rnp::make_bad_position());
    }
  }

  return rnp::is_valid_position(goal_);
}

TilePosition Squad::get_center() const {
  if (members_.size() == 1) {
    auto a = act::whereis<BaseAgent>(*members_.begin());
    return a->get_unit()->getTilePosition();
  }

  int c_x = 0;
  int c_y = 0;
  int cnt = 0;

  //Calculate sum (x,y)
  act::for_each_in<BaseAgent>(
      members_,
      [&cnt,&c_x,&c_y](const BaseAgent* a) {
          if (a->is_alive()) {
            auto a_position = a->get_unit()->getTilePosition();
            c_x += a_position.x;
            c_y += a_position.y;
            cnt++;
          }
      });

  //Calculate average (x,y)
  if (cnt > 0) {
    c_x = c_x / cnt;
    c_y = c_y / cnt;
  }

  //To make sure the center is in a walkable tile, we need to
  //find the unit closest to center
  auto c = TilePosition(c_x, c_y);
  auto best_spot = c;
  auto best_dist = 10000.0;
  act::for_each_in<BaseAgent>(
      members_,
      [this,&best_dist,&best_spot,&c](const BaseAgent* a) {
          if (a->is_alive()) {
            if ((is_air() && a->unit_type().isFlyer())
                || (is_ground() && not a->unit_type().isFlyer()))
            {
              auto a_position = a->get_unit()->getTilePosition();
              auto dist = a_position.getDistance(c);
              if (dist < best_dist) {
                best_dist = dist;
                best_spot = a_position;
              }
            }
          }
      });

  return best_spot;
}

int Squad::get_squad_size() const {
  int no = 0;
  act::for_each_in<BaseAgent>(
      members_,
      [&no](const BaseAgent* a) {
          if (a->is_alive() && not a->get_unit()->isBeingConstructed()) {
            no++;
          }
      });
  return no;
}

int Squad::get_total_units() const {
  int tot = 0;

  for (int i = 0; i < (int)setup_.size(); i++) {
    tot += setup_[i].count_;
  }

  return tot;
}

int Squad::get_strength() const {
  int strength = 0;

  act::for_each_in<BaseAgent>(
      members_,
      [&strength](const BaseAgent* a) {
          if (a->is_alive()) {
            strength += a->unit_type().destroyScore();
          }
      });

  return strength;
}

bool Squad::is_offensive_squad() const {
  return type_ == SquadType::SHUTTLE || type_ == SquadType::OFFENSIVE;
}

bool Squad::is_defensive_squad() const {
  return type_ == SquadType::DEFENSIVE;
}

bool Squad::is_explorer_squad() const {
  return type_ == SquadType::EXPLORER;
}

bool Squad::is_support_squad() const {
  return type_ == SquadType::SUPPORT;
}

bool Squad::is_bunker_defend_squad() const {
  return type_ == SquadType::BUNKER;
}

bool Squad::is_shuttle_squad() const {
  return type_ == SquadType::SHUTTLE;
}

bool Squad::is_kite_squad() const {
  return type_ == SquadType::KITE;
}

bool Squad::is_rush_squad() const {
  return type_ == SquadType::RUSH;
}

bool Squad::is_ground() const {
  return move_type_ == MoveType::GROUND;
}

bool Squad::is_air() const {
  return move_type_ == MoveType::AIR;
}

bool Squad::has_units(UnitType type, int no) {
  for (int i = 0; i < (int)setup_.size(); i++) {
    if (setup_[i].equals(type)) {
      if (setup_[i].current_count_ >= no) {
        //I have these units
        return true;
      }
    }
  }
  return false;
}

void Squad::handle_message(act::Message* incoming) {
  if (auto mdie = dynamic_cast<msg::squad::MemberDestroyed*>(incoming)) {
    members_.erase(mdie->dead_);
  }
  if (auto madd = dynamic_cast<msg::squad::AddMemberToAny*>(incoming)) {
    auto agent = act::whereis<BaseAgent>(madd->member_id_);
    if (need_unit(agent->unit_type()) && add_member(madd->member_id_)) {
      // ok
    } 
    else if (not madd->continue_.empty()) {
      msg::squad::add_member(agent, madd->continue_);
    }
  }
  else if (auto sdef = dynamic_cast<msg::squad::Defend*>(incoming)) {
    defend(sdef->spot_);
  }
  else if (auto satt = dynamic_cast<msg::squad::Charge*>(incoming)) {
    attack(satt->spot_);
  }
  else if (auto setg = dynamic_cast<msg::squad::SetGoal*>(incoming)) {
    set_goal(setg->goal_);
  }
  else if (auto attg = dynamic_cast<msg::squad::Attack*>(incoming)) {
    attack(attg->goal_);
  }
  else if (auto adds = dynamic_cast<msg::squad::AddSetup*>(incoming)) {
    add_setup(adds->type_, adds->count_);
  }
  else if (auto req = dynamic_cast<msg::squad::Required*>(incoming)) {
    set_required(req->value_);
  }
  else if (dynamic_cast<msg::squad::ForceActive*>(incoming)) {
    force_active();
  }
  else if (auto bup = dynamic_cast<msg::squad::Buildup*>(incoming)) {
    set_buildup(bup->value_);
  }
  else if (auto pri = dynamic_cast<msg::squad::Priority*>(incoming)) {
    set_priority(pri->value_);
  }
  else if (auto apri = dynamic_cast<msg::squad::ActivePriority*>(incoming)) {
    set_active_priority(apri->value_);
  }
  else if (auto assi = dynamic_cast<msg::squad::Assist*>(incoming)) {
    assist(assi->loc_);
  }
  else if (dynamic_cast<msg::squad::Disband*>(incoming)) {
    disband();
  }
  else if (auto mdie = dynamic_cast<msg::squad::MemberDestroyed*>(incoming)) {
    members_.erase(mdie->dead_);
  }
  else {
    // Since BaseAgent is base class for everything, we don't need to pass the
    // message to our base, instead create an error
    unhandled_message(incoming);
  }
}
