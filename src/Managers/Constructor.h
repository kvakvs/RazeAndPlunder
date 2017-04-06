#pragma once

#include <BWAPI.h>
#include <vector>
#include "Actors/Actor.h"
#include "RnpConst.h"
#include "Actors/Algorithm.h"
#include "Glob.h"
#include "MainAgents/AgentFactory.h"

class BuildQueueItem {
public:
  BWAPI::UnitType to_build_;
  int assigned_frame_ = 0;
  int assigned_worker_id_ = 0;

  BuildQueueItem(const BWAPI::UnitType& tb, int assignedf, int workerid)
  : to_build_(tb), assigned_frame_(assignedf), assigned_worker_id_(workerid) {
  }
};

class BuildQueue {
  std::vector<BuildQueueItem> q_items_;
public:
  BuildQueue() {}

  size_t size() const { return q_items_.size(); }

  bool empty() const { return q_items_.empty(); }

  const BuildQueueItem& item(size_t i) const {
    return q_items_[i];
  }

  void remove(size_t i);

  void emplace_back(const BWAPI::UnitType& tb, int assignedf, int workerid);
};

class ConstructionPlan {
  std::vector<BWAPI::UnitType> units_;
public:
  ConstructionPlan(): units_() {}
  
  size_t size() const { return units_.size(); }

  bool empty() const { return units_.empty();  }

  void push_front(const BWAPI::UnitType& ut);

  void push_back(const BWAPI::UnitType& ut) { units_.push_back(ut); }

  const BWAPI::UnitType& item(size_t i) const {
    return units_[i];
  }

  void remove(size_t i);
};

/** The Constructor class is responsible for constructing the buildings added 
 * to the build queue.
 *
 * The Constructor is implemented as a singleton class. Each class that needs 
 * to access Constructor can request an instance, and all classes shares the 
 * same Constructor instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Constructor : public act::Actor {
protected:
  int last_command_center_ = 0;
  int last_call_frame_ = 0;
  BuildQueue queue_;
  ConstructionPlan plan_;

public:
  // Temporarily public until Ctor becomes an actor, called from worker
  void lock(int build_plan_index, int unit_id);

protected:
  bool execute_order(const BWAPI::UnitType& type);

  //bool shallBuildSupplyDepot();

  // Checks if we currently is constructing a building 
  bool is_being_built(BWAPI::UnitType type) const;

  bool has_resources_left();

  size_t minerals_nearby(BWAPI::TilePosition center) const;

public:
  Constructor();
  ~Constructor();

  // Returns the number of entries in the build plan. 
  size_t get_build_plan_length() const {
    return plan_.size();
  }

  // Returns the number of units of the specified type currently being produced. 
  size_t get_in_production_count(BWAPI::UnitType type) const;

  // Checks if we have, or is currently building, or planning to build a structure. 
  bool need_building(BWAPI::UnitType type) const;

  // Notifies that an own unit has been destroyed. 
  void on_building_destroyed(BWAPI::Unit building);
  void tick_manage_buildplan(int frame);

  // When a request to construct a new building is issued, no construction are
  // allowed until the worker has moved to the buildspot and started constructing
  // the building. This is to avoid that the needed resources are not used up by
  // other build orders. During this time the Constructor is locked, and new 
  // construction can only be done when unlock has been called. 
  void unlock(BWAPI::UnitType type);

  // Removes a building from the buildorder. 
  void remove(BWAPI::UnitType type);

  // Called when a worker that is constructing a building is destroyed. 
  void handle_worker_destroyed(BWAPI::UnitType type, int workerID);

  // Sets that a new command center has been built. 
  void command_center_built();

  // Shows some debug info on screen. 
  void debug_print_info() const;

  // Is called when no buildspot has been found for the specified type. Gives each Constructor
  // an opportunity to handle it. 
  void handle_no_buildspot_found(BWAPI::UnitType toBuild);

  // Checks if more supply buildings are needed. 
  bool shall_build_supply();

  // Checks if a supply is under construction. 
  bool supply_being_built();

  // Returns true if next in buildorder is of the specified type. Returns false if
  // buildorder is empty. 
  bool is_next_of_type(BWAPI::UnitType type) const;

  // Returns true if buildorder contains a unit of the specified type. 
  bool contains_type(BWAPI::UnitType type) const;

  // Adds a building to the buildorder queue. 
  void add_building(BWAPI::UnitType type);

  // Adds a building first in the buildorder queue. 
  void add_building_first(BWAPI::UnitType type);

  // Requests to expand the base. 
  void expand(BWAPI::UnitType commandCenterUnit);

  // Checks if next building in the buildqueue is an expansion. 
  bool is_next_building_expand();

  // Adds a refinery to the buildorder list. 
  void add_refinery();

  // Checks if the specified TilePosition is covered by a detector buildings sight radius. 
  static bool is_covered_by_detector(BWAPI::TilePosition pos);

  // Morphs a Zerg drone to a building. 
  bool zerg_drone_morph(BWAPI::UnitType target, BWAPI::UnitType evolved);

  //
  // Actor things
  //

  // Called each update to issue orders. 
  void tick() override;

  // Don't let lambdas capture references, use [=] instead
  static void modify(std::function<void(Constructor*)> f) {
    act::modify_actor<Constructor>(rnp::constructor_id(), f);
  }

  uint32_t ac_flavour() const override {
    return static_cast<uint32_t>(ActorFlavour::Singleton);
  }

  void handle_message(act::Message* incoming) override;
};
