#pragma once

#include "Squad.h"
#include "MainAgents/BaseAgent.h"
#include "../Managers/BuildplanEntry.h"

#include "BWEM/bwem.h"
#include <iso646.h>


struct SortSquadList {
  bool operator()(Squad::Ptr& sq1, Squad::Ptr& sq2) const {
    if (sq1->getPriority() != sq2->getPriority()) {
      return sq1->getPriority() < sq2->getPriority();
    }
    return (sq1->isRequired() && not sq2->isRequired());
  }
};

/** The Commander class is the base class for commanders. The Commander classes are responsible for
 * which and when buildings to construct, when to do upgrades/techs, and which squads to build.
 * It is also responsible for finding defensive positions, launch attacks and where to launch an
 * attack.
 *
 * The Commander is implemented as a singleton class. Each class that needs to access Commander can
 * request an instance, and the correct commander (Terran/Protoss/Zerg) will be returned.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Commander {
public:
  using Ptr = std::shared_ptr < Commander > ;

  enum class State {
    DEFEND = 0,
    ATTACK = 1
  };

private:
  int last_call_frame_;
  bool removal_done_ = false;

  // NYI bool chokePointFortified(BWAPI::TilePosition center);
  void sort_squad_list();
  
  BWAPI::TilePosition find_defense_pos(const BWEM::ChokePoint* choke) const;

  void check_no_squad_units();

  void assign_unit(BaseAgent* agent);

protected:
//  static Commander* singleton_;
  int stage_ = 0;

  State current_state_ = State::DEFEND;
  bool debug_bp_ = false;
  bool debug_sq_ = false;

  std::vector<Squad::Ptr> squads_;
  std::vector<BuildplanEntry> buildplan_;
  int workers_num_;
  int workers_per_refinery_;

  Commander();

  // Checks the Commander buildplan, and add buildings,
  // techs and upgrades to the planners. 
  void check_buildplan();

  // Stops the production of workers. 
  void cut_workers_production();

public:
  // Executes basic code for a commander. 
  void on_frame_base();

  virtual ~Commander();

  // Returns the instance of the class. 
//  static Commander* getInstance();

  // Switch on/off buildplan debug info printing to screen. 
  void toggle_buildplan_debug();

  // Switch on/off squads debug info printing to screen. 
  void toggle_squads_debug();

  // Called each update to issue orders. 
  virtual void on_frame() = 0;

  // Returns the number of preferred workers, i.e. the
  // number of workers should be built. 
  int get_preferred_workers_count() const;

  // Returns the preferred number of workers for a refinery. 
  int get_workers_per_refinery() const;

  // Used in debug modes to show goal of squads. 
  virtual void debug_show_goal();

  // Checks if it is time to engage the enemy. This happens when all Required squads
  // are active. 
  bool is_time_to_engage();

  // Updates the goals for all squads. 
  void update_squad_goals();

  // Called each time a unit is created. The unit is then
  // placed in a Squad. 
  void on_unit_created(BaseAgent* agent);

  // Called each time a unit is destroyed. The unit is then
  // removed from its Squad. 
  void on_unit_destroyed(BaseAgent* agent);

  // Checks if the specified unittype needs to be built.
  bool is_unit_needed(BWAPI::UnitType type);

  // Removes a squad. 
  void remove_squad(int id);

  // Adds a new squad. 
  void add_squad(Squad::Ptr sq);

  // Returns the Squad with the specified id, or nullptr if not found. 
  Squad::Ptr getSquad(int id);

  // Returns the position where to launch an attack at. 
  static BWAPI::TilePosition find_attack_position();

  // Checks if workers needs to attack. Happens if base is under attack and no offensive units
  // are available. 
  bool check_workers_attack(BaseAgent* base) const;

  // Checks if we need to assist a building. 
  bool assist_building();

  // Checks if we need to assist a worker that is under attack. 
  bool assist_worker();

  // Checks if there are any removable obstacles nearby, i.e. minerals with 
  // less than 20 resources left. 
  void check_removable_obstacles();

  // Forces an attack, even if some squads are not full. 
  void force_begin_attack();

  // Shows some info on the screen. 
  void print_info();

  // Searches for and returns a good chokepoint position to defend the territory. 
  BWAPI::TilePosition find_chokepoint();

  // Checks if there are any unfinished buildings that does not have an SCV working on them. Terran only. 
  bool check_damaged_buildings();

  // Assigns a worker to finish constructing an interrupted building. Terran only. 
  void assist_unfinished_construction(BaseAgent* agent);

  // Adds a bunker squad when a Terran Bunker has been created. Returns
  // the squadID of the bunker squad. 
  int add_bunker_squad();

  // Removes a bunker squad. Used when the bunker is destroyed
  // with units inside. 
  bool remove_bunker_squad(int unitID);

  // Removes the race from a string, Terran Marine = Marine. 
  static std::string format(const std::string& str);
};
