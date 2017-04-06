#pragma once

namespace msg::squad {

//-----------------------------------------------------------------------------
class MemberDestroyed : public act::Message {
public:
  act::ActorId dead_;

  explicit MemberDestroyed(const act::ActorId& dead)
    : dead_(dead) {
  }
};
inline void member_destroyed(const act::ActorId& dst,
                       const act::ActorId& member_id) {
  act::send_message<MemberDestroyed>(dst, member_id);
}

                                                   //-----------------------------------------------------------------------------
//template <class Container>
//void add_member(const BaseAgent* unit, const Container& squads) {
//  auto agent_type = unit->unit_type();
//  auto& first = *(squads.begin());
//  
//  act::ActorId::Vector other_squads;
//  other_squads.reserve(squads.size() - 1);
//  std::copy_if(squads.begin(), squads.end(),
//               std::back_inserter(other_squads),
//               [&first](const act::ActorId& aid) { return aid != first; });
//
//  add_member(first, unit->self(), std::move(other_squads));
//}

//-----------------------------------------------------------------------------
class Defend : public act::Message {
public:
  BWAPI::TilePosition spot_;

  explicit Defend(const BWAPI::TilePosition& s) : spot_(s) {
  }
};

inline void defend(const act::ActorId& dst, const BWAPI::TilePosition& spot) {
  act::send_message<Defend>(dst, spot);
}

//-----------------------------------------------------------------------------
class Assist : public act::Message {
public:
  BWAPI::TilePosition loc_;

  explicit Assist(const BWAPI::TilePosition& l): loc_(l) {
  }
};

inline void assist(const act::ActorId& dst, const BWAPI::TilePosition& l) {
  act::send_message<Assist>(dst, l);
}

//-----------------------------------------------------------------------------
class Disband : public act::Message {
};

inline void disband(const act::ActorId& dst) {
  act::send_message<Disband>(dst);
}

} // ns msg::squad
