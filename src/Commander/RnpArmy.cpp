#include "Commander/Commander.h"
#include "Commander/RnpArmy.h"
#include "Glob.h"
#include "RnpUtil.h"
#include "Managers/Constructor.h"
#include "Squad.h"

#define MODULE_PREFIX "(army) "
using namespace BWAPI;

namespace rnp {

Army::Army()
: setup_()
{
}

void Army::handle_message(act::Message* m) {
}

void Army::tick() {
}


size_t Army::get_max_size() {
  size_t count = 0;
  for (size_t i = 0; i < setup_.size(); i++) {
    count += setup_[i].wanted_count_;
  }
  return count;
}

void Army::add_setup(UnitType type, int no) {
  rnp::log()->debug(MODULE_PREFIX "add_setup {} x{}", type.toString(), no);

  //First, check if we have the setup already
  for (auto& setup : setup_) {
    if (setup.type_.getID() == type.getID()) {
      //Found, increase the amount
      setup.wanted_count_ += no;
      return;
    }
  }

  //Not found, add as new
  UnitSetup us;
  us.type_ = type;
  us.wanted_count_ = no;
  us.current_count_ = 0;
  setup_.push_back(us);

//  if (not type.isFlyer()) {
//    move_type_ = MoveType::GROUND;
//  }
}

void Army::remove_setup(UnitType type, int no) {
  for (auto& setup : setup_) {
    if (setup.type_.getID() == type.getID()) {
      //Found, reduce the amount
      setup.wanted_count_ = std::max<size_t>(0, setup.wanted_count_ - no);
    }
  }
}

bool Army::need_unit(BWAPI::UnitType type) const {
  //1. Check if prio is set to Inactive squad.
//  if (priority_ >= 1000) {
//    return false;
//  }

  int no_created = 1;
  if (rnp::is_zerg()) {
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
          + no_created <= setup_[i].wanted_count_)
      {
        return true;
      }
    }
  }

  return false;
}

} // rnp
#undef MODULE_PREFIX
