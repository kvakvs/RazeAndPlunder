#include "BuildingPlacer.h"
#include "AgentManager.h"
#include "ExplorationManager.h"
#include "../Pathfinding/Pathfinder.h"
#include "Constructor.h"
#include "../Influencemap/MapManager.h"
#include "Commander/Commander.h"
#include "Utils/Profiler.h"
#include "BWEMUtil.h"
#include "Glob.h"

using namespace BWAPI;

BuildingPlacer::BuildingPlacer() : bwem_(BWEM::Map::Instance()) {
  w_ = Broodwar->mapWidth();
  h_ = Broodwar->mapHeight();
  range_ = 40;

  Unit worker = findWorker(Broodwar->self()->getStartLocation());

  cover_map_ = new TileType*[w_];
  for (int i = 0; i < w_; i++) {
    cover_map_[i] = new TileType[h_];

    //Fill from static map and Region connectability
    for (int j = 0; j < h_; j++) {
      auto ok = TileType::BUILDABLE;
      if (not Broodwar->isBuildable(i, j)) {
        ok = TileType::BLOCKED;
      }

      cover_map_[i][j] = ok;
    }
  }

  //Fill from current agents
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isBuilding()) {
      Corners c = getCorners(a->getUnit());
      fill(c);
    }
  }

  //Fill from minerals
  for (auto& u : Broodwar->getMinerals()) {
    Corners c;
    c.x1 = u->getTilePosition().x - 2;
    c.y1 = u->getTilePosition().y - 2;
    c.x2 = u->getTilePosition().x + 2;
    c.y2 = u->getTilePosition().y + 2;
    fill(c);

    cover_map_[c.x1 + 2][c.y1 + 2] = TileType::MINERAL;
  }

  //Fill from gas
  for (auto& u : Broodwar->getGeysers()) {
    Corners c;
    c.x1 = u->getTilePosition().x - 2;
    c.y1 = u->getTilePosition().y - 2;
    c.x2 = u->getTilePosition().x + 5;
    c.y2 = u->getTilePosition().y + 3;
    fill(c);

    cover_map_[c.x1 + 2][c.y1 + 2] = TileType::GAS;
  }

  // Fill from narrow chokepoints
  for (auto& area : bwem_.Areas()) {
    for (auto choke : area.ChokePoints()) {
      if (rnp::choke_width(choke) <= 4 * 32) {
        TilePosition center = TilePosition(choke->Center());
        Corners c;
        c.x1 = center.x - 1;
        c.x2 = center.x + 1;
        c.y1 = center.y - 1;
        c.y2 = center.y + 1;
        fill(c);
      }
    }
  }

  //Fill from neutral buildings
  for (auto& u : Broodwar->neutral()->getUnits()) {
    if (u->exists() && !u->getType().isResourceContainer() && u->getType().isSpecialBuilding()) {
      Corners c = getCorners(u);
      fill(c);
    }
  }
}

BuildingPlacer::~BuildingPlacer() {
  for (int i = 0; i < w_; i++) {
    delete[] cover_map_[i];
  }
  delete[] cover_map_;
}

Unit BuildingPlacer::findWorker(TilePosition spot) {
  BaseAgent* worker = rnp::agent_manager()->findClosestFreeWorker(spot);
  if (worker != nullptr) {
    return worker->getUnit();
  }
  return nullptr;
}

bool BuildingPlacer::positionFree(TilePosition pos) {
  if (cover_map_[pos.x][pos.y] == TileType::BUILDABLE) {
    return true;
  }
  return false;
}

void BuildingPlacer::blockPosition(TilePosition buildSpot) {
  if (buildSpot.x == -1) {
    //Error check
    return;
  }
  cover_map_[buildSpot.x][buildSpot.y] = TileType::BLOCKED;
}

bool BuildingPlacer::canBuild(UnitType toBuild, TilePosition buildSpot) {
  Corners c = getCorners(toBuild, buildSpot);

  //Step 1: Check BuildingPlacer.
  for (int x = c.x1; x <= c.x2; x++) {
    for (int y = c.y1; y <= c.y2; y++) {
      if (x >= 0 && x < w_ && y >= 0 && y < h_) {
        if (cover_map_[x][y] != TileType::BUILDABLE) {
          //Cant build here.
          return false;
        }
      }
      else {
        //Out of bounds
        return false;
      }
    }
  }

  //Step 2: Check if path is available
  if (not ExplorationManager::canReach(Broodwar->self()->getStartLocation(), buildSpot)) {
    return false;
  }

  //Step 3: Check canBuild
  Unit worker = findWorker(buildSpot);
  if (worker == nullptr) {
    //No worker available
    return false;
  }

  //Step 4: Check any units on tile
  /*if (rnp::agent_manager()->unitsInArea(buildSpot, toBuild.tileWidth(), toBuild.tileHeight(), worker->getID()))
  {
    return false;
  }*/

  //Step 5: If Protoss, check PSI coverage
  if (Constructor::isProtoss()) {
    if (toBuild.requiresPsi()) {
      if (not Broodwar->hasPower(buildSpot, toBuild)) {
        return false;
      }
    }

    //Spread out Pylons
    for (auto& a : rnp::agent_manager()->getAgents()) {
      if (a->isAlive() && a->getUnitType().getID() == UnitTypes::Protoss_Pylon.getID()) {
        if (a->getUnit()->getTilePosition().getDistance(buildSpot) <= 3) {
          return false;
        }
      }
    }
  }

  //Step 6: If Zerg, check creep
  if (Constructor::isZerg()) {
    if (toBuild.requiresCreep()) {
      for (int x = buildSpot.x; x < buildSpot.x + toBuild.tileWidth(); x++) {
        for (int y = buildSpot.y; y < buildSpot.y + toBuild.tileHeight(); y++) {
          if (not Broodwar->hasCreep(TilePosition(x, y))) {
            return false;
          }
        }
      }
    }
  }

  //Step 7: If detector, check if spot is already covered by a detector
  if (toBuild.isDetector()) {
    if (not suitableForDetector(buildSpot)) {
      return false;
    }
  }

  //All passed. It is possible to build here.
  return true;
}

TilePosition BuildingPlacer::findBuildSpot(UnitType toBuild) {
  range_ = 40;
  if (toBuild.getID() == UnitTypes::Protoss_Pylon.getID()) range_ = 80;

  //Refinery
  if (toBuild.isRefinery()) {
    //Use refinery method
    return findRefineryBuildSpot(toBuild, Broodwar->self()->getStartLocation());
  }

  //If we find unpowered buildings, build a Pylon there
  if (BaseAgent::isOfType(toBuild, UnitTypes::Protoss_Pylon)) {
    auto& agents = rnp::agent_manager()->getAgents();
    for (auto& a : agents) {
      if (a->isAlive()) {
        Unit cUnit = a->getUnit();
        if (not cUnit->isPowered()) {
          TilePosition spot = findBuildSpot(toBuild, cUnit->getTilePosition());
          return spot;
        }
      }
    }
  }

  //Build near chokepoints: Bunker, Photon Cannon, Creep Colony
  if (BaseAgent::isOfType(toBuild, UnitTypes::Terran_Bunker) 
    || BaseAgent::isOfType(toBuild, UnitTypes::Protoss_Photon_Cannon) 
    || BaseAgent::isOfType(toBuild, UnitTypes::Zerg_Creep_Colony)) {
    TilePosition cp = rnp::commander()->findChokePoint();
    if (cp.x != -1) {
      TilePosition spot = findBuildSpot(toBuild, cp);
      return spot;
    }
  }

  //Base buildings.
  if (toBuild.isResourceDepot()) {
    TilePosition start = rnp::exploration()->searchExpansionSite();
    if (start.x != -1) {
      //Expansion site found. Build close to it.
      TilePosition spot = findBuildSpot(toBuild, start);
      return spot;
    }
  }

  //General building. Search for spot around bases
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive() && a->getUnitType().isResourceDepot() && !baseUnderConstruction(a)) {
      TilePosition start = a->getUnit()->getTilePosition();
      TilePosition bSpot = findBuildSpot(toBuild, start);
      if (bSpot.x != -1) {
        //Spot found, return it.
        return bSpot;
      }
    }
  }

  return TilePosition(-1, -1);
}

bool BuildingPlacer::isDefenseBuilding(UnitType toBuild) {
  int bId = toBuild.getID();
  if (bId == UnitTypes::Terran_Bunker.getID()) return true;
  if (bId == UnitTypes::Protoss_Photon_Cannon.getID()) return true;
  if (bId == UnitTypes::Zerg_Creep_Colony.getID()) return true;

  return false;
}


bool BuildingPlacer::baseUnderConstruction(BaseAgent* base) {
  if (Constructor::isTerran()) {
    return base->getUnit()->isBeingConstructed();
  }
  if (Constructor::isProtoss()) {
    return base->getUnit()->isBeingConstructed();
  }
  if (Constructor::isZerg()) {
    if (base->isOfType(UnitTypes::Zerg_Hatchery)) {
      return base->getUnit()->isBeingConstructed();
    }
  }
  return false;
}

TilePosition BuildingPlacer::findSpotAtSide(UnitType toBuild, TilePosition start, TilePosition end) {
  int dX = end.x - start.x;
  if (dX != 0) dX = 1;
  int dY = end.y - start.y;
  if (dY != 0) dY = 1;

  TilePosition cPos = start;
  bool done = false;
  while (!done) {
    if (canBuildAt(toBuild, cPos)) return cPos;
    int cX = cPos.x + dX;
    int cY = cPos.y + dY;
    cPos = TilePosition(cX, cY);
    if (cPos.x == end.x && cPos.y == end.y) done = true;
  }

  return TilePosition(-1, -1);
}

bool BuildingPlacer::canBuildAt(UnitType toBuild, TilePosition pos) {
  int maxW = w_ - toBuild.tileWidth() - 1;
  int maxH = h_ - toBuild.tileHeight() - 1;

  //Out of bounds check
  if (pos.x >= 0 && pos.x < maxW && pos.y >= 0 && pos.y < maxH) {
    if (canBuild(toBuild, pos)) {
      return true;
    }
  }
  return false;
}

TilePosition BuildingPlacer::findBuildSpot(UnitType toBuild, TilePosition start) {
  //Check start pos
  if (canBuildAt(toBuild, start)) return start;

  //Search outwards
  bool found = false;
  int cDiff = 1;
  TilePosition spot = TilePosition(-1, -1);
  while (!found) {
    //Top
    TilePosition s = TilePosition(start.x - cDiff, start.y - cDiff);
    TilePosition e = TilePosition(start.x + cDiff, start.y - cDiff);
    spot = findSpotAtSide(toBuild, s, e);
    if (spot.x != -1 && spot.y != -1) {
      found = true;
      break;
    }

    //Bottom
    s = TilePosition(start.x - cDiff, start.y + cDiff);
    e = TilePosition(start.x + cDiff, start.y + cDiff);
    spot = findSpotAtSide(toBuild, s, e);
    if (spot.x != -1 && spot.y != -1) {
      found = true;
      break;
    }

    //Left
    s = TilePosition(start.x - cDiff, start.y - cDiff);
    e = TilePosition(start.x - cDiff, start.y + cDiff);
    spot = findSpotAtSide(toBuild, s, e);
    if (spot.x != -1 && spot.y != -1) {
      found = true;
      break;
    }

    //Right
    s = TilePosition(start.x + cDiff, start.y - cDiff);
    e = TilePosition(start.x + cDiff, start.y + cDiff);
    spot = findSpotAtSide(toBuild, s, e);
    if (spot.x != -1 && spot.y != -1) {
      found = true;
      break;
    }

    cDiff++;
    if (cDiff > range_) found = true;
  }

  return spot;
}

void BuildingPlacer::addConstructedBuilding(Unit unit) {
  if (unit->getType().isAddon()) {
    //Addons are handled by their main buildings
    return;
  }

  Corners c = getCorners(unit);
  fill(c);
}

void BuildingPlacer::buildingDestroyed(Unit unit) {
  if (unit->getType().isAddon()) {
    //Addons are handled by their main buildings
    return;
  }

  Corners c = getCorners(unit);
  clear(c);
}

void BuildingPlacer::fill(Corners c) {
  for (int x = c.x1; x <= c.x2; x++) {
    for (int y = c.y1; y <= c.y2; y++) {
      if (x >= 0 && x < w_ && y >= 0 && y < h_) {
        if (cover_map_[x][y] == TileType::BUILDABLE) {
          cover_map_[x][y] = TileType::BLOCKED;
        }
        if (cover_map_[x][y] == TileType::TEMPBLOCKED) {
          cover_map_[x][y] = TileType::BLOCKED;
        }
      }
    }
  }
}

void BuildingPlacer::fillTemp(UnitType toBuild, TilePosition buildSpot) {
  Corners c = getCorners(toBuild, buildSpot);

  for (int x = c.x1; x <= c.x2; x++) {
    for (int y = c.y1; y <= c.y2; y++) {
      if (x >= 0 && x < w_ && y >= 0 && y < h_) {
        if (cover_map_[x][y] == TileType::BUILDABLE) {
          cover_map_[x][y] = TileType::TEMPBLOCKED;
        }
      }
    }
  }
}

void BuildingPlacer::clear(Corners c) {
  for (int x = c.x1; x <= c.x2; x++) {
    for (int y = c.y1; y <= c.y2; y++) {
      if (x >= 0 && x < w_ && y >= 0 && y < h_) {
        if (cover_map_[x][y] == TileType::BLOCKED) {
          cover_map_[x][y] = TileType::BUILDABLE;
        }
        if (cover_map_[x][y] == TileType::TEMPBLOCKED) {
          cover_map_[x][y] = TileType::BUILDABLE;
        }
      }
    }
  }
}

void BuildingPlacer::clearTemp(UnitType toBuild, TilePosition buildSpot) {
  if (buildSpot.x == -1) {
    return;
  }

  Corners c = getCorners(toBuild, buildSpot);

  for (int x = c.x1; x <= c.x2; x++) {
    for (int y = c.y1; y <= c.y2; y++) {
      if (x >= 0 && x < w_ && y >= 0 && y < h_) {
        if (cover_map_[x][y] == TileType::TEMPBLOCKED) {
          cover_map_[x][y] = TileType::BUILDABLE;
        }
      }
    }
  }
}

Corners BuildingPlacer::getCorners(Unit unit) const {
  return getCorners(unit->getType(), unit->getTilePosition());
}

Corners BuildingPlacer::getCorners(UnitType type, TilePosition center) const {
  int x1 = center.x;
  int y1 = center.y;
  int x2 = x1 + type.tileWidth() - 1;
  int y2 = y1 + type.tileHeight() - 1;

  int margin = 0;
  if (type.canProduce()) margin = 1;
  if (type.getID() == UnitTypes::Terran_Bunker.getID()) margin = 1;

  x1 -= margin;
  x2 += margin;
  y1 -= margin;
  y2 += margin;

  //Special case: Terran Addon buildings
  //Add 2 extra spaces to the right to make space for the addons.
  if (BaseAgent::isOfType(type, UnitTypes::Terran_Factory) || BaseAgent::isOfType(type, UnitTypes::Terran_Starport) || BaseAgent::isOfType(type, UnitTypes::Terran_Command_Center) || BaseAgent::isOfType(type, UnitTypes::Terran_Science_Facility)) {
    x2 += 2;
  }

  Corners c;
  c.x1 = x1;
  c.y1 = y1;
  c.x2 = x2;
  c.y2 = y2;

  return c;
}

TilePosition BuildingPlacer::findRefineryBuildSpot(UnitType toBuild, TilePosition start) {
  TilePosition buildSpot = findClosestGasWithoutRefinery(toBuild, start);
  if (buildSpot.x >= 0) {
    BaseAgent* base = rnp::agent_manager()->getClosestBase(buildSpot);
    if (base == nullptr) {
      Broodwar << "No base found" << std::endl;
      return TilePosition(-1, -1);
    }
    else {
      double dist = buildSpot.getDistance(base->getUnit()->getTilePosition());
      if (dist >= 14) {
        return TilePosition(-1, -1);
      }
    }

  }
  return buildSpot;
}

TilePosition BuildingPlacer::findClosestGasWithoutRefinery(UnitType toBuild, TilePosition start) {
  TilePosition bestSpot = TilePosition(-1, -1);
  double bestDist = -1;
  TilePosition home = Broodwar->self()->getStartLocation();
  Unit worker = findWorker(start);

  for (int i = 0; i < w_; i++) {
    for (int j = 0; j < h_; j++) {
      if (cover_map_[i][j] == TileType::GAS) {
        TilePosition cPos = TilePosition(i, j);

        bool ok = true;
        auto& agents = rnp::agent_manager()->getAgents();
        for (auto& a : agents) {
          Unit unit = a->getUnit();
          if (unit->getType().isRefinery()) {
            double dist = unit->getTilePosition().getDistance(cPos);
            if (dist <= 2) {
              ok = false;
            }
          }
        }
        if (ok) {
          if (ExplorationManager::canReach(home, cPos)) {
            BaseAgent* agent = rnp::agent_manager()->getClosestBase(cPos);
            double dist = agent->getUnit()->getTilePosition().getDistance(cPos);
            if (bestDist == -1 || dist < bestDist) {
              bestDist = dist;
              bestSpot = cPos;
            }
          }
        }
      }
    }
  }

  return bestSpot;
}

TilePosition BuildingPlacer::searchRefinerySpot() const {
  for (int i = 0; i < w_; i++) {
    for (int j = 0; j < h_; j++) {
      if (cover_map_[i][j] == TileType::GAS) {
        TilePosition cPos = TilePosition(i, j);

        bool found = false;
        auto& agents = rnp::agent_manager()->getAgents();
        for (auto& a : agents) {
          if (a->getUnitType().isRefinery()) {
            double dist = a->getUnit()->getTilePosition().getDistance(cPos);
            TilePosition uPos = a->getUnit()->getTilePosition();
            if (dist <= 2) {
              found = true;
              break;
            }
          }
        }

        if (not found) {
          BaseAgent* agent = rnp::agent_manager()->getClosestBase(cPos);
          if (agent != nullptr) {
            TilePosition bPos = agent->getUnit()->getTilePosition();
            double dist = bPos.getDistance(cPos);

            if (dist < 15) {
              if (ExplorationManager::canReach(bPos, cPos)) {
                return cPos;
              }
            }
          }
        }
      }
    }
  }

  return TilePosition(-1, -1);
}

TilePosition BuildingPlacer::findExpansionSite() const {
  UnitType baseType = Broodwar->self()->getRace().getCenter();
  double bestDist = 100000;
  TilePosition bestPos = TilePosition(-1, -1);

  //Iterate through all base locations
  for (auto& area : bwem_.Areas()) {
    for (auto& base : area.Bases()) {
      TilePosition pos(base.Center());

      if (pos.x != Broodwar->self()->getStartLocation().x 
        || pos.y != Broodwar->self()->getStartLocation().y) {
        bool taken = false;

        //Check if own buildings are close
        if (rnp::map_manager()->hasOwnInfluenceIn(pos)) taken = true;
        //Check if enemy buildings are close
        if (rnp::map_manager()->hasEnemyInfluenceIn(pos)) taken = true;

        //Not taken, calculate ground distance
        if (not taken) {
          if (ExplorationManager::canReach(Broodwar->self()->getStartLocation(), pos)) {
            double dist = rnp::pathfinder()->getDistance(Broodwar->self()->getStartLocation(), pos);
            if (dist <= bestDist && dist > 0) {
              bestDist = dist;
              bestPos = pos;
            }
          }
        } // not taken
      } // if pos not start location
    } // for bases
  } // for areas

  //Don't build expansions too far away!
  BaseAgent* base = rnp::agent_manager()->getClosestBase(bestPos);
  if (base != nullptr) {
    double d = base->getUnit()->getTilePosition().getDistance(bestPos);
    if (d >= 140) {
      Broodwar << "Expansion site too far away: " << d << std::endl;
      return TilePosition(-1, -1);
    }
  }
  else {
    return TilePosition(-1, -1);
  }

  return bestPos;
}

Unit BuildingPlacer::findClosestMineral(TilePosition workerPos) {
  Unit mineral = nullptr;
  double bestDist = 10000;

  for (auto& area : bwem_.Areas()) {
    for (auto& base : area.Bases()) {
      TilePosition pos(base.Center());
      double cDist = pos.getDistance(workerPos);
      
      if (cDist < bestDist) {
        //Find closest base
        BaseAgent* base = rnp::agent_manager()->getClosestBase(pos);
        if (base != nullptr) {
          double dist = pos.getDistance(base->getUnit()->getTilePosition());
          if (dist <= 12) {
            //We have a base near this base location
            //Check if we have minerals available
            Unit cMineral = hasMineralNear(pos);
            if (cMineral != nullptr) {
              mineral = cMineral;
              bestDist = cDist;
            }
          }
        }
      } // if cdist better than bestdist
    } // for bases
  } // for areas

  //We have no base with minerals, do nothing
  return mineral;
}

Unit BuildingPlacer::hasMineralNear(TilePosition pos) {
  for (auto& u : Broodwar->getMinerals()) {
    if (u->exists() && u->getResources() > 0) {
      double dist = pos.getDistance(u->getTilePosition());
      if (dist <= 10) {
        return u;
      }
    }
  }
  return nullptr;
}

bool BuildingPlacer::suitableForDetector(TilePosition pos) {
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    UnitType type = a->getUnitType();
    if (a->isAlive() && type.isDetector() && type.isBuilding()) {
      double dRange = type.sightRange() * 1.6;
      double dist = a->getUnit()->getPosition().getDistance(Position(pos));
      if (dist <= dRange) {
        return false;
      }
    }
  }
  return true;
}

void BuildingPlacer::debug() const {
  for (int x = 0; x < w_; x++) {
    for (int y = 0; y < h_; y++) {
      if (cover_map_[x][y] == TileType::TEMPBLOCKED) {
        Broodwar->drawBoxMap(x * 32, y * 32, x * 32 + 31, y * 32 + 31, Colors::Green, false);
      }
      if (cover_map_[x][y] == TileType::BLOCKED) {
        Broodwar->drawBoxMap(x * 32, y * 32, x * 32 + 31, y * 32 + 31, Colors::Brown, false);
      }
    }
  }
}
