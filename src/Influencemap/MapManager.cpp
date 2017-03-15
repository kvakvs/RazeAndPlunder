#include "MapManager.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include "MainAgents/BaseAgent.h"

#include "bwem.h"
#include "BWEMUtil.h"
#include "Glob.h"

using namespace BWAPI;

MapManager::MapManager() : bwem_(BWEM::Map::Instance()) {
  //Add the regions for this map
  for (auto& r : bwem_.Areas()) {
    MRegion* mr = new MRegion();
    mr->region = &r;
    mr->resetInfluence();
    map_.insert(mr);

    //Add base locations for this map
    for (auto& base : r.Bases()) {
      bases_.insert(new BaseLocationItem(base.Location()));
    }
  }
  last_call_frame_ = 0;
}

const MRegion* MapManager::getMapRegion(const BWEM::Area* area) {
  auto a_center = rnp::get_center(area);
  for (MRegion* mr : map_) {
    auto center = rnp::get_center(mr->region);
    if (center.x == a_center.x && center.y == a_center.y) {
      return mr;
    }
  }
  return nullptr;
}

bool MapManager::isValidChokepoint(const BWEM::ChokePoint* cp) {
  //Use this code to hard-code chokepoints that shall not be used.
  if (Broodwar->mapFileName() == "(4)Andromeda.scx") {
    auto c = Position(cp->Center());
    if (c.x == 2780 && c.y == 3604) return false;
    if (c.x == 2776 && c.y == 448) return false;
    if (c.x == 1292 && c.y == 436) return false;
    if (c.x == 1300 && c.y == 3584) return false;
  }
  if (Broodwar->mapFileName() == "(2)Benzene.scx") {
    auto c = Position(cp->Center());
    if (c.x == 4044 && c.y == 1088) return false;
    if (c.x == 44 && c.y == 1064) return false;
  }
  if (Broodwar->mapFileName() == "(2)Destination.scx") {
    auto c = Position(cp->Center());
    if (c.x == 1309 && c.y == 3851) return false;
    if (c.x == 1730 && c.y == 226) return false;
  }
  if (Broodwar->mapFileName() == "(4)Fortress.scx") {
    auto c = Position(cp->Center());
    if (c.x == 3132 && c.y == 912) return false;
    if (c.x == 764 && c.y == 3312) return false;
  }

  return true;
}

const BWEM::ChokePoint* MapManager::findGuardChokepoint(const MRegion* mr) {
  auto mr_center = rnp::get_center(mr->region);

  for (auto c : mr->region->ChokePoints()) {
    if (isValidChokepoint(c)) {
      auto connected_areas = c->GetAreas();
      auto first_center = rnp::get_center(connected_areas.first);
      
      if (first_center.x == mr_center.x && first_center.y == mr_center.y) {
        const MRegion* adj = getMapRegion(connected_areas.second);
        if (adj->inf_own_buildings == 0) {
          return c;
        }
      }

      auto second_center = rnp::get_center(connected_areas.second);
      if (second_center.x == mr_center.x && second_center.y == mr_center.y) {
        const MRegion* adj = getMapRegion(connected_areas.first);
        if (adj->inf_own_buildings == 0) {
          return c;
        }
      }
    }
  }
  return nullptr;
}

const BWEM::ChokePoint* MapManager::getDefenseLocation() {
  int bestInfluence = 0;
  const BWEM::ChokePoint* best = nullptr;

  for (MRegion* base : map_) {
    if (base->inf_own_buildings > bestInfluence) {
      const BWEM::ChokePoint* cp = findGuardChokepoint(base);
      if (cp != nullptr) {
        if (base->inf_own_buildings > bestInfluence) {
          bestInfluence = base->inf_own_buildings;
          best = cp;
        }
      }
    }
  }

  return best;
}

MRegion* MapManager::getMapFor(Position p) {
  for (auto& mreg : map_) {
    if (rnp::is_inside(*mreg->region, p)) {
      return mreg;
    }
  }
  return nullptr;
}

bool MapManager::hasEnemyInfluence() {
  for (auto& mr : map_) {
    if (mr->inf_en_buildings > 0) {
      return true;
    }
  }
  return false;
}

void MapManager::update() {
  //Dont call too often
  int cFrame = Broodwar->getFrameCount();
  if (cFrame - last_call_frame_ < 10) {
    return;
  }
  last_call_frame_ = cFrame;

  //Reset previous influence scores
  for (MRegion* mr : map_) {
    mr->resetInfluence();
  }

  //Update visited base locations
  for (auto& a : bases_) {
    if (Broodwar->isVisible(a->baseLocation)) {
      a->frameVisited = Broodwar->getFrameCount();
    }
  }

  //Update own influence
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive()) {
      MRegion* mr = getMapFor(a->getUnit()->getPosition());
      if (mr != nullptr) {
        if (a->getUnitType().isBuilding()) {
          mr->inf_own_buildings += a->getUnitType().buildScore();
          if (a->getUnitType().canAttack()) {
            if (a->getUnitType().groundWeapon().targetsGround() || a->getUnitType().airWeapon().targetsGround()) {
              mr->inf_own_ground += a->getUnitType().buildScore();
            }
            if (a->getUnitType().groundWeapon().targetsAir() || a->getUnitType().airWeapon().targetsAir()) {
              mr->inf_own_air += a->getUnitType().buildScore();
            }
          }
        }
        else if (a->getUnitType().isAddon()) {
          //No influence from addons
        }
        else if (a->getUnitType().isWorker()) {
          //No influence for workers
        }
        else {
          //Regular units
          if (a->getUnitType().canAttack()) {
            if (a->getUnitType().groundWeapon().targetsGround() || a->getUnitType().airWeapon().targetsGround()) {
              mr->inf_own_ground += a->getUnitType().buildScore();
            }
            if (a->getUnitType().groundWeapon().targetsAir() || a->getUnitType().airWeapon().targetsAir()) {
              mr->inf_own_air += a->getUnitType().buildScore();
            }
          }
        }
      }
    }
  }

  //Update enemy buildings influence
  for (MRegion* mr : map_) {
    mr->inf_en_buildings = rnp::exploration()->getSpottedInfluenceInRegion(mr->region);
  }

  //Update enemy units influence
  for (auto& u : Broodwar->enemy()->getUnits()) {
    //Enemy seen
    if (u->exists()) {
      MRegion* mr = getMapFor(u->getPosition());
      if (mr != nullptr) {
        UnitType type = u->getType();
        if (not type.isWorker() && type.canAttack()) {
          if (type.groundWeapon().targetsGround() || type.airWeapon().targetsGround()) {
            mr->inf_en_ground += type.buildScore();
          }
          if (type.groundWeapon().targetsAir() || type.airWeapon().targetsAir()) {
            mr->inf_en_air += type.buildScore();
          }
        }
      }
    }
  }
}

int MapManager::getOwnGroundInfluenceIn(TilePosition pos) {
  for (MRegion* cm : map_) {
    if (rnp::is_inside(*cm->region, Position(pos))) {
      return cm->inf_own_ground;
    }
  }
  return 0;
}

int MapManager::getEnemyGroundInfluenceIn(TilePosition pos) {
  for (MRegion* cm : map_) {
    if (rnp::is_inside(*cm->region, Position(pos))) {
      return cm->inf_en_ground;
    }
  }
  return 0;
}

bool MapManager::hasOwnInfluenceIn(TilePosition pos) {
  for (MRegion* cm : map_) {
    if (cm->inf_own_buildings > 0 
      && rnp::is_inside(*cm->region, Position(pos))) {
      return true;
    }
  }
  return false;
}

bool MapManager::hasEnemyInfluenceIn(TilePosition pos) {
  for (MRegion* cm : map_) {
    if (cm->inf_en_buildings > 0 
      && rnp::is_inside(*cm->region, Position(pos))) {
      return true;
    }
  }
  return false;
}

TilePosition MapManager::findAttackPosition() {
  MRegion* best = nullptr;
  for (MRegion* cm : map_) {
    if (cm->inf_en_buildings > 0) {
      if (best == nullptr) {
        best = cm;
      }
      else {
        //Launch an attack at the enemy controlled region with the
        //lowest influence.
        if (cm->inf_en_buildings < best->inf_en_buildings) {
          best = cm;
        }
      }
    }
  }

  if (best != nullptr) {
    return TilePosition(rnp::get_center(best->region));
  }
  else {
    //No enemy building found. Move to starting positions.
    int longestVisitFrame = Broodwar->getFrameCount();
    TilePosition base = TilePosition(-1, -1);
    for (auto& a : bases_) {
      if (a->frameVisited < longestVisitFrame) {
        longestVisitFrame = a->frameVisited;
        base = a->baseLocation;
      }
    }

    return base;
  }
}

MapManager::~MapManager() {
  for (MRegion* mr : map_) {
    delete mr;
  }
  for (auto& a : bases_) {
    delete a;
  }
}

void MapManager::printInfo() {
  for (MRegion* mr : map_) {
    auto mr_center = rnp::get_center(mr->region);
    int x1 = mr_center.x;
    int y1 = mr_center.y;
    int x2 = x1 + 110;
    int y2 = y1 + 90;

    Broodwar->drawBoxMap(x1, y1, x2, y2, Colors::Brown, true);
    Broodwar->drawTextMap(x1 + 5, y1, "Buildings own: %d", mr->inf_own_buildings);
    Broodwar->drawTextMap(x1 + 5, y1 + 15, "Ground own: %d", mr->inf_own_ground);
    Broodwar->drawTextMap(x1 + 5, y1 + 30, "Air own: %d", mr->inf_own_air);
    Broodwar->drawTextMap(x1 + 5, y1 + 45, "Buildings en: %d", mr->inf_en_buildings);
    Broodwar->drawTextMap(x1 + 5, y1 + 60, "Ground en: %d", mr->inf_en_ground);
    Broodwar->drawTextMap(x1 + 5, y1 + 75, "Air en: %d", mr->inf_en_air);

    //Print location of each chokepoint, and also if it is blocked
    //as defense position.
    for (auto c : mr->region->ChokePoints()) {
      auto c_center = c->Center();
      x1 = c_center.x;
      y1 = c_center.y;
      Broodwar->drawTextMap(x1, y1, "(%d,%d)", x1, y1);
      if (not isValidChokepoint(c)) Broodwar->drawTextMap(x1, y1 + 12, "Blocked");
    }
    Broodwar->drawTextScreen(10, 120, "'%s'", Broodwar->mapFileName().c_str());
  }
}
