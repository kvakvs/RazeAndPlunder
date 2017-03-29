#pragma once
#include "Glob.h"

namespace msg::commander {

// Runs some code on mutable Commander*, at some appropriate time soon
inline void modify_commander(std::function<void(Commander *)> fn) {
  act::modify_actor<Commander>(rnp::commander_id(), fn);
}

class UnitCreated: public act::Message {
public:
  act::ActorId new_;
  explicit UnitCreated(const act::ActorId& n): new_(n) {}
};
inline void unit_created(const act::ActorId& d) {
  act::send_message<UnitCreated>(rnp::commander_id(), d);
}

class UnitDestroyed : public act::Message {
public:
  act::ActorId dead_;
  explicit UnitDestroyed(const act::ActorId& d) : dead_(d) {}
};
inline void unit_destroyed(const act::ActorId& d) {
  act::send_message<UnitDestroyed>(rnp::commander_id(), d);
}

class BunkerDestroyed : public act::Message {
public:
  int unit_id_;
  explicit BunkerDestroyed(int n) : unit_id_(n) {}
};
inline void bunker_destroyed(int b) {
  act::send_message<BunkerDestroyed>(rnp::commander_id(), b);
}

class SquadCreated : public act::Message {
public:
  act::ActorId squad_;
  explicit SquadCreated(const act::ActorId& s) : squad_(s) {}
};
inline void squad_created(const act::ActorId& d) {
  act::send_message<SquadCreated>(rnp::commander_id(), d);
}

class RemoveSquad : public act::Message {
public:
  act::ActorId squad_;
  explicit RemoveSquad(const act::ActorId& s) : squad_(s) {}
};
inline void remove_squad(const act::ActorId& d) {
  act::send_message<RemoveSquad>(rnp::commander_id(), d);
}

class UpdateGoals : public act::Message {};
inline void update_goals() {
  act::send_message<UpdateGoals>(rnp::commander_id());
}

} // ns msg
  