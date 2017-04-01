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
//
////-----------------------------------------------------------------------------
//// Request to take a new squad member
//class AddMember : public act::Message {
//public:
//  act::ActorId member_id_;
//  // If member adding failed (squad full or something), then the message
//  // will bounce back to return_undeliverable_ asking them to choose another
//  // random squad or try later 
//  act::ActorId return_undeliverable_;
//
//  AddMember(const act::ActorId& member_id,
//            const act::ActorId& return_undeliverable)
//    : member_id_(member_id), return_undeliverable_(return_undeliverable) {
//  }
//};
//
//inline void add_member(const act::ActorId& dst,
//                       const act::ActorId& member_id,
//                       const act::ActorId& return_undeliverable) {
//  act::send_message<AddMember>(dst, member_id, return_undeliverable);
//}


//-----------------------------------------------------------------------------
// Request a squad to take a new member, a list of other squads is also passed,
// so they can be tried too
class AddMemberToAny : public act::Message {
public:
  act::ActorId member_id_;
  // If member adding failed (squad full or something), then the message
  // will be resent to another squad in the continue_ list
  act::ActorId::Vector continue_;

  AddMemberToAny(const act::ActorId& member_id,
                 act::ActorId::Vector&& cont)
    : member_id_(member_id), continue_(std::move(cont)) {
  }
};

inline void add_member(const act::ActorId& dst,
                       const act::ActorId& member_id,
                       act::ActorId::Vector&& squads) {
  act::send_message<AddMemberToAny>(dst, member_id, std::move(squads));
}

template <class Container>
void add_member(const BaseAgent* unit, const Container& squads) {
  auto agent_type = unit->unit_type();
  auto& first = *(squads.begin());
  
  act::ActorId::Vector other_squads;
  other_squads.reserve(squads.size() - 1);
  std::copy_if(squads.begin(), squads.end(),
               std::back_inserter(other_squads),
               [&first](const act::ActorId& aid) { return aid != first; });

  add_member(first, unit->self(), std::move(other_squads));
}

//-----------------------------------------------------------------------------
// Reply failed to add squad member, try again to another squad or later
//class AddMember_Failed : public act::Message {
//public:
//  act::ActorId member_id_;
//
//  explicit AddMember_Failed(const act::ActorId& a): member_id_(a) {
//  }
//};
//
//inline void add_member_failed(const act::ActorId& dst,
//                              const act::ActorId& member_id) {
//  act::send_message<AddMember_Failed>(dst, member_id);
//}

//-----------------------------------------------------------------------------
class AddSetup : public act::Message {
public:
  BWAPI::UnitType type_;
  int count_;

  AddSetup(const BWAPI::UnitType& type, int count)
    : type_(type),
      count_(count) {
  }
};

inline void add_setup(const act::ActorId& dst, BWAPI::UnitType t, int c) {
  act::send_message<AddSetup>(dst, t, c);
}

//-----------------------------------------------------------------------------
class Required : public act::Message {
public:
  bool value_;

  explicit Required(int p) : value_(p) {
  }
};

static void required(const act::ActorId& dst, bool v) {
  act::send_message<Required>(dst, v);
}

//-----------------------------------------------------------------------------
class Buildup : public act::Message {
public:
  bool value_;

  explicit Buildup(int p) : value_(p) {
  }
};

inline void buildup(const act::ActorId& dst, bool v) {
  act::send_message<Buildup>(dst, v);
}

//-----------------------------------------------------------------------------
class Priority : public act::Message {
public:
  int value_;

  explicit Priority(int p) : value_(p) {
  }
};

inline void priority(const act::ActorId& dst, int v) {
  act::send_message<Priority>(dst, v);
}

//-----------------------------------------------------------------------------
class ActivePriority : public act::Message {
public:
  int value_;

  explicit ActivePriority(int p) : value_(p) {
  }
};

inline void active_priority(const act::ActorId& dst, int v) {
  act::send_message<ActivePriority>(dst, v);
}

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

//-----------------------------------------------------------------------------
class ForceActive : public act::Message {
};

static void force_active(const act::ActorId& dst) {
  act::send_message<ForceActive>(dst);
}

//-----------------------------------------------------------------------------
//class UpdateArrivedFrame : public act::Message {
//public:
//  int frame_;
//
//  explicit UpdateArrivedFrame(int t) : frame_(t) {
//  }
//};
//
//inline void update_arrived_frame(const act::ActorId& dst, int c) {
//  act::send_message<UpdateArrivedFrame>(dst, c);
//}

} // ns msg::squad
