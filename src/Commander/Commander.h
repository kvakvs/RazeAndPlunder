#pragma once

#include "Squad.h"
#include "MainAgents/BaseAgent.h"
#include "../Managers/BuildplanEntry.h"

#include "bwem.h"
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
  void sortSquadList();
  BWAPI::TilePosition findDefensePos(const BWEM::ChokePoint* choke) const;

  void checkNoSquadUnits();
  void assignUnit(BaseAgent* agent);

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
  void checkBuildplan();

  // Stops the production of workers. 
  void cutWorkers();

public:
  // Executes basic code for a commander. 
  void computeActionsBase();

  virtual ~Commander();

  // Returns the instance of the class. 
//  static Commander* getInstance();

  // Switch on/off buildplan debug info printing to screen. 
  void toggleBuildplanDebug();

  // Switch on/off squads debug info printing to screen. 
  void toggleSquadsDebug();

  // Called each update to issue orders. 
  virtual void computeActions() {
  }

  // Returns the number of preferred workers, i.e. the
  // number of workers should be built. 
  int getNoWorkers() const;

  // Returns the preferred number of workers for a refinery. 
  int getWorkersPerRefinery() const;

  // Used in debug modes to show goal of squads. 
  virtual void debug_showGoal();

  // Checks if it is time to engage the enemy. This happens when all Required squads
  // are active. 
  bool shallEngage();

  // Updates the goals for all squads. 
  void updateGoals();

  // Called each time a unit is created. The unit is then
  // placed in a Squad. 
  void unitCreated(BaseAgent* agent);

  // Called each time a unit is destroyed. The unit is then
  // removed from its Squad. 
  void unitDestroyed(BaseAgent* agent);

  /* Checks if the specified unittype needs to be built. */
  bool needUnit(BWAPI::UnitType type);

  // Removes a squad. 
  void removeSquad(int id);

  // Adds a new squad. 
  void addSquad(Squad::Ptr sq);

  // Returns the Squad with the specified id, or nullptr if not found. 
  Squad::Ptr getSquad(int id);

  // Returns the position where to launch an attack at. 
  BWAPI::TilePosition findAttackPosition() const;

  // Checks if workers needs to attack. Happens if base is under attack and no offensive units
  // are available. 
  bool checkWorkersAttack(BaseAgent* base) const;

  // Checks if we need to assist a building. 
  bool assistBuilding();

  // Checks if we need to assist a worker that is under attack. 
  bool assistWorker();

  // Checks if there are any removable obstacles nearby, i.e. minerals with less than 20 resources
  // left. 
  void checkRemovableObstacles();

  // Forces an attack, even if some squads are not full. 
  void forceAttack();

  // Shows some info on the screen. 
  void printInfo();

  // Searches for and returns a good chokepoint position to defend the territory. 
  BWAPI::TilePosition findChokePoint();

  // Checks if there are any unfinished buildings that does not have an SCV working on them. Terran only. 
  bool checkDamagedBuildings();

  // Assigns a worker to finish constructing an interrupted building. Terran only. 
  void finishBuild(BaseAgent* agent);

  // Adds a bunker squad when a Terran Bunker has been created. Returns
  // the squadID of the bunker squad. 
  int addBunkerSquad();

  // Removes a bunker squad. Used when the bunker is destroyed
  // with units inside. 
  bool removeBunkerSquad(int unitID);

  // Removes the race from a string, Terran Marine = Marine. 
  static std::string format(std::string str);
};
