#pragma once

#include <BWAPI.h>
#include "../Utils/Sets.h"

#include "bwem.h"

struct MRegion {
  const BWEM::Area* region;
  int inf_own_ground;
  int inf_own_air;
  int inf_own_buildings;
  int inf_en_ground;
  int inf_en_air;
  int inf_en_buildings;

  void resetInfluence() {
    inf_own_ground = 0;
    inf_own_air = 0;
    inf_own_buildings = 0;
    inf_en_ground = 0;
    inf_en_air = 0;
    inf_en_buildings = 0;
  }
};

class MRegionSet : public BWAPI::SetContainer<MRegion*, std::hash<void*>> {
public:

};

/** This class creates an influence map where own and enemy influence (buildings, ground strength,
 * air strength) are calculate for each BWTA region.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class MapManager {
  MRegionSet map;
  BaseLocationSet bases;
  int lastCallFrame;
  BWEM::Map& bwem_;
  static MapManager* instance;

private:
  MapManager();

  MRegion* getMapFor(BWAPI::Position p);

  const MRegion* getMapRegion(const BWEM::Area* r);
  const BWEM::ChokePoint* findGuardChokepoint(const MRegion* mr);
  bool isValidChokepoint(const BWEM::ChokePoint* cp);

public:
  // Destructor 
  ~MapManager();

  // Returns the instance of the class. 
  static MapManager* getInstance();

  // Updates the influence map. 
  void update();

  // Returns a good chokepoint to place defensive forces at. 
  const BWEM::ChokePoint* getDefenseLocation();

  // Checks if any region with enemy influence has been found. 
  bool hasEnemyInfluence();

  // Returns a suitable position to attack the enemy at. 
  BWAPI::TilePosition findAttackPosition();

  // Checks if the player has infuence in the specified position. 
  bool hasOwnInfluenceIn(BWAPI::TilePosition pos);

  // Checks if the enemy has influence in the specified position. 
  bool hasEnemyInfluenceIn(BWAPI::TilePosition pos);

  // Returns the player ground unit infuence in the specified position. 
  int getOwnGroundInfluenceIn(BWAPI::TilePosition pos);

  // Returns the enemy ground influence in the specified position. 
  int getEnemyGroundInfluenceIn(BWAPI::TilePosition pos);

  // Prints debug info to screen. 
  void printInfo();
};
