#include "ExplorationManager.h"
#include "BuildingPlacer.h"
#include "RnpUtil.h"
#include "Glob.h"

using namespace BWAPI;

RegionItem::RegionItem(const BWEM::Area* region): location_() {
  location_ = rnp::get_center(*region);
  frame_visited_ = Broodwar->getFrameCount();
}

ExplorationManager::ExplorationManager()
    : bwem_(BWEM::Map::Instance()), enemy_(), explore_(), expansion_site_()
{
  //Add the regions for this map
  for (auto& r : bwem_.Areas()) {
    explore_.insert(new RegionItem(&r));
  }

  site_set_frame_ = 0;

  last_call_frame_ = Broodwar->getFrameCount();

  expansion_site_ = TilePosition(-1, -1);
}

ExplorationManager::~ExplorationManager() {
}

void ExplorationManager::on_frame() {
  //Dont call too often
  int cFrame = Broodwar->getFrameCount();
  if (cFrame - last_call_frame_ < 9) {
    return;
  }
  last_call_frame_ = cFrame;

  for (auto& a : explore_) {
    if (Broodwar->isVisible(a->location_)) {
      a->frame_visited_ = Broodwar->getFrameCount();
    }
  }
}

TilePosition ExplorationManager::search_expansion_site() {
  get_expansion_site();

  if (not rnp::is_valid_position(expansion_site_)) {
    expansion_site_ = rnp::building_placer()->find_expansion_site();
    site_set_frame_ = Broodwar->getFrameCount();
  }

  return expansion_site_;
}

TilePosition ExplorationManager::get_expansion_site() {
  if (expansion_site_.x >= 0) {
    if (Broodwar->getFrameCount() - site_set_frame_ > 500) {
      expansion_site_ = TilePosition(-1, -1);
    }
  }

  return expansion_site_;
}

void ExplorationManager::set_explored(TilePosition pos) {
  for (auto& a : explore_) {
    if (a->location_.x == pos.x && a->location_.y == pos.y) {
      a->frame_visited_ = Broodwar->getFrameCount();
      return;
    }
  }

  Broodwar << "Cannot set to explored" << std::endl;
}

void ExplorationManager::set_expansion_site(TilePosition pos) {
  if (pos.x >= 0) {
    site_set_frame_ = Broodwar->getFrameCount();
    expansion_site_ = pos;
  }
}

TilePosition ExplorationManager::get_next_to_explore(Squad* squad) {
  int longestVisitFrame = Broodwar->getFrameCount();
  TilePosition nextPos = TilePosition(-1, -1);
  for (auto& a : explore_) {
    if (a->frame_visited_ < longestVisitFrame) {
      longestVisitFrame = a->frame_visited_;
      nextPos = a->location_;
    }
  }
  return nextPos;
}

void ExplorationManager::debug_print() {
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

void ExplorationManager::on_unit_spotted(Unit unit) {
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

void ExplorationManager::on_unit_destroyed(Unit unit) {
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

int ExplorationManager::get_spotted_influence_in_region(const BWEM::Area* area) {
  int im = 0;
  for (auto& spotted_object : enemy_) {
    if (rnp::is_inside(*area, spotted_object->getPosition())) {
      im += spotted_object->getType().buildScore();
    }
  }
  return im;
}

TilePosition ExplorationManager::get_closest_spotted_building(TilePosition start) {
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

bool ExplorationManager::can_reach(TilePosition a, TilePosition b) const {
  auto possible_path = bwem_.GetPath(BWAPI::Position(a), BWAPI::Position(b));
  return possible_path.empty() == false;
}

bool ExplorationManager::can_reach(BaseAgent* agent, TilePosition b) const {
  return agent->getUnit()->hasPath(Position(b));
}

bool ExplorationManager::enemy_is_protoss() {
  for (auto& u : Broodwar->getPlayers()) {
    if (u->isEnemy(Broodwar->self())) {
      if (u->getRace().getID() == Races::Protoss.getID()) {
        return true;
      }
    }
  }
  return false;
}

bool ExplorationManager::enemy_is_zerg() {
  for (auto& u : Broodwar->getPlayers()) {
    if (u->isEnemy(Broodwar->self())) {
      if (u->getRace().getID() == Races::Zerg.getID()) {
        return true;
      }
    }
  }
  return false;
}

bool ExplorationManager::enemy_is_terran() {
  for (auto& u : Broodwar->getPlayers()) {
    if (u->isEnemy(Broodwar->self())) {
      if (u->getRace().getID() == Races::Terran.getID()) {
        return true;
      }
    }
  }
  return false;
}

bool ExplorationManager::enemy_is_unknown() {
  if (not enemy_is_terran() && not enemy_is_protoss() && not enemy_is_zerg()) {
    return true;
  }
  return false;
}
