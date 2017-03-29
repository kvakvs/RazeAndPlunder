#include "ExplorationManager.h"
#include "BuildingPlacer.h"
#include "BWEMUtil.h"
#include "Glob.h"

using namespace BWAPI;

RegionItem::RegionItem(const BWEM::Area* region) {
  location = rnp::get_center(*region);
  frameVisited = Broodwar->getFrameCount();
}

ExplorationManager::ExplorationManager(): bwem_(BWEM::Map::Instance()) {
  //Add the regions for this map
  for (auto& r : bwem_.Areas()) {
    explore_.insert(new RegionItem(&r));
  }

  site_set_frame_ = 0;

  last_call_frame_ = Broodwar->getFrameCount();

  expansionSite = TilePosition(-1, -1);
}

ExplorationManager::~ExplorationManager() {
}

void ExplorationManager::computeActions() {
  //Dont call too often
  int cFrame = Broodwar->getFrameCount();
  if (cFrame - last_call_frame_ < 9) {
    return;
  }
  last_call_frame_ = cFrame;

  for (auto& a : explore_) {
    if (Broodwar->isVisible(a->location)) {
      a->frameVisited = Broodwar->getFrameCount();
    }
  }
}

TilePosition ExplorationManager::searchExpansionSite() {
  getExpansionSite();

  if (expansionSite.x == -1) {
    expansionSite = rnp::building_placer()->findExpansionSite();
    site_set_frame_ = Broodwar->getFrameCount();
  }

  return expansionSite;
}

TilePosition ExplorationManager::getExpansionSite() {
  if (expansionSite.x >= 0) {
    if (Broodwar->getFrameCount() - site_set_frame_ > 500) {
      expansionSite = TilePosition(-1, -1);
    }
  }

  return expansionSite;
}

void ExplorationManager::setExplored(TilePosition pos) {
  for (auto& a : explore_) {
    if (a->location.x == pos.x && a->location.y == pos.y) {
      a->frameVisited = Broodwar->getFrameCount();
      return;
    }
  }

  Broodwar << "Cannot set to explored" << std::endl;
}

void ExplorationManager::setExpansionSite(TilePosition pos) {
  if (pos.x >= 0) {
    site_set_frame_ = Broodwar->getFrameCount();
    expansionSite = pos;
  }
}

TilePosition ExplorationManager::getNextToExplore(Squad* squad) {
  int longestVisitFrame = Broodwar->getFrameCount();
  TilePosition nextPos = TilePosition(-1, -1);
  for (auto& a : explore_) {
    if (a->frameVisited < longestVisitFrame) {
      longestVisitFrame = a->frameVisited;
      nextPos = a->location;
    }
  }
  return nextPos;
}

void ExplorationManager::printInfo() {
  //Uncomment this if you want to draw a mark at detected enemy buildings.
  /*for (int i = 0; i < (int)spottedBuildings.size(); i++)
  {
    if (spottedBuildings.at(i)->isActive())
    {
      int x1 = spottedBuildings.at(i)->getTilePosition().x * 32;
      int y1 = spottedBuildings.at(i)->getTilePosition().y * 32;
      int x2 = x1 + 32;
      int y2 = y1 + 32;

      Broodwar->drawBoxMap(x1,y1,x2,y2,Colors::Blue,true);
    }
  }*/

  //Draw a circle around detectors
}

void ExplorationManager::addSpottedUnit(Unit unit) {
  if (unit->getType().isBuilding()) {
    //Check if we already have seen this building
    bool found = false;
    for (auto& a : enemy_) {
      if (a->getUnitID() == unit->getID()) {
        found = true;
        break;
      }
    }

    if (not found) {
      enemy_.insert(new SpottedObject(unit));
    }
  }
}

void ExplorationManager::unitDestroyed(Unit unit) {
  TilePosition uPos = unit->getTilePosition();
  if (unit->getType().isBuilding()) {
    bool removed = false;
    for (auto& a : enemy_) {
      if (a->isAt(unit->getTilePosition())) {
        enemy_.erase(a);
        return;
      }
    }
  }
}

void ExplorationManager::cleanup() {
  bool cont = true;

  while (cont) {
    cont = false;
    for (auto& a : enemy_) {
      if (Broodwar->isVisible(a->getTilePosition())) {
        bool found = false;
        for (auto& u : Broodwar->enemy()->getUnits()) {
          if (u->exists()) {
            if (u->getID() == a->getUnitID()) {
              found = true;
              break;
            }
          }
        }
        if (not found) {
          enemy_.erase(a);
          cont = true;
          break;
        }
      }
    }
  }
}

int ExplorationManager::getSpottedInfluenceInRegion(const BWEM::Area* area) {
  int im = 0;
  for (auto& spotted_object : enemy_) {
    if (rnp::is_inside(*area, spotted_object->getPosition())) {
      im += spotted_object->getType().buildScore();
    }
  }
  return im;
}

TilePosition ExplorationManager::getClosestSpottedBuilding(TilePosition start) {
  cleanup();

  TilePosition pos = TilePosition(-1, -1);
  double bestDist = 100000;

  for (auto& a : enemy_) {
    double cDist = start.getDistance(a->getTilePosition());
    if (cDist < bestDist) {
      bestDist = cDist;
      pos = a->getTilePosition();
    }
  }

  return pos;
}

bool ExplorationManager::canReach(TilePosition a, TilePosition b) {
//  auto ra = bwem_.GetNearestArea(a);
//  auto rb = bwem_.GetNearestArea(b);
  auto& bwem = BWEM::Map::Instance();
  auto possible_path = bwem.GetPath(BWAPI::Position(a), BWAPI::Position(b));
  return possible_path.empty() == false;
}

bool ExplorationManager::canReach(BaseAgent* agent, TilePosition b) {
  return agent->getUnit()->hasPath(Position(b));
}

bool ExplorationManager::enemyIsProtoss() {
  for (auto& u : Broodwar->getPlayers()) {
    if (u->isEnemy(Broodwar->self())) {
      if (u->getRace().getID() == Races::Protoss.getID()) {
        return true;
      }
    }
  }
  return false;
}

bool ExplorationManager::enemyIsZerg() {
  for (auto& u : Broodwar->getPlayers()) {
    if (u->isEnemy(Broodwar->self())) {
      if (u->getRace().getID() == Races::Zerg.getID()) {
        return true;
      }
    }
  }
  return false;
}

bool ExplorationManager::enemyIsTerran() {
  for (auto& u : Broodwar->getPlayers()) {
    if (u->isEnemy(Broodwar->self())) {
      if (u->getRace().getID() == Races::Terran.getID()) {
        return true;
      }
    }
  }
  return false;
}

bool ExplorationManager::enemyIsUnknown() {
  if (not enemyIsTerran() && !enemyIsProtoss() && !enemyIsZerg()) {
    return true;
  }
  return false;
}
