#pragma once
#include "Actors/Algorithm.h"

namespace msg::unit {

//-----------------------------------------------------------------------------
class AttackBWUnit : public act::Message {
public:
  BWAPI::Unit target_;

  explicit AttackBWUnit(BWAPI::Unit target)
    : target_(target) {
  }
};

inline void attack_unit(const act::ActorId& dst, BWAPI::Unit tgt) {
  act::send_message<AttackBWUnit>(dst, tgt);
}

//-----------------------------------------------------------------------------
class Attack : public act::Message {
public:
  act::ActorId target_;
  explicit Attack(const act::ActorId& a): target_(a) {}
};

inline void attack(const act::ActorId& dst, const act::ActorId& tgt) {
  act::send_message<Attack>(dst, tgt);
}

//-----------------------------------------------------------------------------
class Destroyed : public act::Message {
};

//-----------------------------------------------------------------------------
// You belong to the squad now, son
class JoinedSquad : public act::Message {
public:
  act::ActorId squad_;
  BWAPI::TilePosition goal_;
  JoinedSquad(const act::ActorId& s, const BWAPI::TilePosition& g)
    : squad_(s), goal_(g) {}
};

inline void joined_squad(const act::ActorId& dst,
                         const act::ActorId& squad, BWAPI::TilePosition goal) {
  act::send_message<JoinedSquad>(dst, squad, goal);
}

//-----------------------------------------------------------------------------
// Used on bunkers to tell them their squad
class SetSquad : public act::Message {
public:
  act::ActorId squad_;
  explicit SetSquad(const act::ActorId& s): squad_(s) {}
};

inline void set_squad(const act::ActorId& dst,
                      const act::ActorId& squad) {
  act::send_message<SetSquad>(dst, squad);
}

//-----------------------------------------------------------------------------
class LeftSquad : public act::Message {
public:
  act::ActorId squad_;
  explicit LeftSquad(const act::ActorId& a) : squad_(a) {}
};

inline void left_squad(const act::ActorId& dst, const act::ActorId& squad) {
  act::send_message<LeftSquad>(dst, squad);
}

//-----------------------------------------------------------------------------
class SetGoal : public act::Message {
public:
  BWAPI::TilePosition goal_;
  explicit SetGoal(const BWAPI::TilePosition& t) : goal_(t) {}
};

inline void set_goal(const act::ActorId& dst, const BWAPI::TilePosition& g) {
  act::send_message<SetGoal>(dst, g);
}

//-----------------------------------------------------------------------------
class AddTrailPos : public act::Message {
public:
  BWAPI::WalkPosition pos_;
  explicit AddTrailPos(const BWAPI::WalkPosition& p) : pos_(p) {}
};

inline void add_trail_pos(const act::ActorId& dst, 
                          const BWAPI::WalkPosition& p) {
  act::send_message<AddTrailPos>(dst, p);
}

} // msg::agent
