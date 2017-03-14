#pragma once

#include "SpottedObject.h"
#include "../Commander/Squad.h"

#include "bwem.h"

class SpottedObjectSet : public BWAPI::SetContainer<SpottedObject*, std::hash<void*>> {
};

class RegionItem {
public:
  explicit RegionItem(const BWEM::Area* region);

  BWAPI::TilePosition location;
  int frameVisited;
};

class RegionSet : public BWAPI::SetContainer<RegionItem*, std::hash<void*>> {
public:

};

/** The ExplorationManager handles all tasks involving exploration of the game world. It issue orders to a number of units
 * that is used as explorers, keep track of areas recently explored, and keep track of spotted resources or enemy buildings.
 *
 * The ExplorationManager is implemented as a singleton class. Each class that needs to access ExplorationManager can request an instance,
 * and all classes shares the same ExplorationManager instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ExplorationManager {

private:
  BWEM::Map& bwem_;
  SpottedObjectSet enemy;
  RegionSet explore;

  static ExplorationManager* instance;
  int lastCallFrame;

  int siteSetFrame;
  BWAPI::TilePosition expansionSite;

private:
  ExplorationManager();
  void cleanup();

public:
  // Destructor 
  ~ExplorationManager();

  // Returns the instance of the class. 
  static ExplorationManager* getInstance();

  // Called each update to issue orders. 
  void computeActions();

  // Returns the next position to explore for this squad. 
  BWAPI::TilePosition getNextToExplore(Squad* squad);

  // Searches for the next position to expand the base to. 
  BWAPI::TilePosition searchExpansionSite();

  // Returns the next position to expand the base to. 
  BWAPI::TilePosition getExpansionSite();

  // Sets the next position to expand the base to. 
  void setExpansionSite(BWAPI::TilePosition pos);

  // Shows all spotted objects as squares on the SC map. Use for debug purpose. 
  void printInfo();

  // Notifies about an enemy unit that has been spotted. 
  void addSpottedUnit(BWAPI::Unit unit);

  // Notifies that an enemy unit has been destroyed. If the destroyed unit was among
  // the spotted units, it is removed from the list. 
  void unitDestroyed(BWAPI::Unit unit);

  // Returns the closest enemy spotted building from a start position, or TilePosition(-1,-1) if// none was found. 
  BWAPI::TilePosition getClosestSpottedBuilding(BWAPI::TilePosition start);

  // Calculates the influence of spotted enemy buildings within a specified region. 
  int getSpottedInfluenceInRegion(const BWEM::Area* region);

  // Returns true if a ground unit can reach position b from position a.
  // Uses BWTA. 
  static bool canReach(BWAPI::TilePosition a, BWAPI::TilePosition b);

  // Returns true if an agent can reach position b. 
  static bool canReach(BaseAgent* agent, BWAPI::TilePosition b);

  // Sets that a region is explored. The position must be the TilePosition for the center of the
  // region. 
  void setExplored(BWAPI::TilePosition goal);

  // Returns true if an enemy is Protoss. 
  static bool enemyIsProtoss();

  // Returns true if an enemy is Zerg. 
  static bool enemyIsZerg();

  // Returns true if an enemy is Terran. 
  static bool enemyIsTerran();

  // All enemy races are currently unknown. 
  static bool enemyIsUnknown();
};
