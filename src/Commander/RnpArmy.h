#pragma once

#include "Commander/UnitSetup.h"
#include "Actors/Actor.h"
#include "RnpConst.h"
#include "Actors/Algorithm.h"
#include "CommanderMsg.h"

namespace rnp {

//
// Army is a high level overview of what kind of army we are building
// but without predefined grouping by squads
//
class Army: public act::Actor {
  std::vector<UnitSetup> setup_; // TODO: maybe change to map?

public:
  Army();
  
  uint32_t ac_flavour() const override { 
    return static_cast<uint32_t>(ActorFlavour::Singleton); 
  }

  void handle_message(act::Message* m) override;
  void tick() override;

  // Returns the maximum size of the full army 
  size_t get_max_size();

  // Adds a setup for certain unit in the army. Setup is a type and amount 
  // of units that shall be created and maintained 
  void add_setup(BWAPI::UnitType type, int no);

  // Removes a setup for some unit type
  void remove_setup(BWAPI::UnitType type, int no);
  bool need_unit(BWAPI::UnitType type) const;
};

} // ns rnp

namespace msg::army {

inline void add_setup(BWAPI::UnitType ut, size_t count) {
  act::modify_actor<rnp::Army>(
    rnp::army_id(),
    [=](rnp::Army* a) { a->add_setup(ut, count); });
}

} // msg::army
