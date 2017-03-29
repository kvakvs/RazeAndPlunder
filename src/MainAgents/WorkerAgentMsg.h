#pragma once

namespace msg::worker {

class AssignBuild : public act::Message {
public:
  BWAPI::UnitType type;

  AssignBuild(BWAPI::UnitType t) : type(t) {
  }
};

static void assign_build(const act::ActorId& dst, BWAPI::UnitType t) {
  act::send_message<AssignBuild>(dst, t);
}

class AssignRepair : public act::Message {
public:
  BWAPI::Unit unit_;

  AssignRepair(BWAPI::Unit t) : unit_(t) {
  }

  AssignRepair(const AssignRepair&) = delete;
  AssignRepair& operator =(const AssignRepair&) = delete;
};

static void assign_repair(const act::ActorId& dst, BWAPI::Unit t) {
  act::send_message<AssignRepair>(dst, t);
}

class RightClickRefinery : public act::Message {
public:
  BWAPI::Unit refinery_;
  RightClickRefinery(const RightClickRefinery&) = delete;
  RightClickRefinery& operator =(const RightClickRefinery&) = delete;

  RightClickRefinery(BWAPI::Unit r) : refinery_(r) {
  }
};

static void right_click_refinery(const act::ActorId& dst, BWAPI::Unit r) {
  act::send_message<RightClickRefinery>(dst, r);
}

} // ns msg
