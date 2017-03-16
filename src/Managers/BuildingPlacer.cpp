#include "BuildingPlacer.h"
#include "AgentManager.h"
#include "ExplorationManager.h"
#include "Pathfinding/Pathfinder.h"
#include "Constructor.h"
#include "Influencemap/MapManager.h"
#include "Commander/Commander.h"
#include "Utils/Profiler.h"
#include "RnpUtil.h"
#include "Glob.h"

using namespace BWAPI;

BuildingPlacer::BuildingPlacer() : bwem_(BWEM::Map::Instance()) {
  w_ = Broodwar->mapWidth();
  h_ = Broodwar->mapHeight();
  range_ = 40;

  Unit worker = find_worker(Broodwar->self()->getStartLocation());

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
      Corners c = get_corners(a->getUnit());
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
    if (u->exists() && not u->getType().isResourceContainer() && u->getType().isSpecialBuilding()) {
      Corners c = get_corners(u);
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

Unit BuildingPlacer::find_worker(TilePosition spot) {
  BaseAgent* worker = rnp::agent_manager()->findClosestFreeWorker(spot);
  if (worker != nullptr) {
    return worker->getUnit();
  }
  return nullptr;
}

bool BuildingPlacer::is_position_available(TilePosition pos) const {
  if (cover_map_[pos.x][pos.y] == TileType::BUILDABLE) {
    return true;
  }
  return false;
}

void BuildingPlacer::mark_position_blocked(TilePosition buildSpot) const {
  if (not rnp::is_valid_position(buildSpot)) {
    //Error check
    return;
  }
  cover_map_[buildSpot.x][buildSpot.y] = TileType::BLOCKED;
}

bool BuildingPlacer::can_build(UnitType to_build, TilePosition build_spot) const {
  Corners c(get_corners(to_build, build_spot));

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
  // TODO: Check path length, should be not too far?
  if (not rnp::exploration()->can_reach(Broodwar->self()->getStartLocation(), build_spot)) {
    return false;
  }

  //Step 3: Check canBuild
  Unit worker = find_worker(build_spot);
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
    if (to_build.requiresPsi()) {
      if (not Broodwar->hasPower(build_spot, to_build)) {
        return false;
      }
    }

    //Spread out Pylons
    for (auto& a : rnp::agent_manager()->getAgents()) {
      if (a->isAlive() && a->getUnitType().getID() == UnitTypes::Protoss_Pylon.getID()) {
        if (a->getUnit()->getTilePosition().getDistance(build_spot) <= 3) {
          return false;
        }
      }
    }
  }

  //Step 6: If Zerg, check creep
  if (Constructor::isZerg()) {
    if (to_build.requiresCreep()) {
      for (int x = build_spot.x; x < build_spot.x + to_build.tileWidth(); x++) {
        for (int y = build_spot.y; y < build_spot.y + to_build.tileHeight(); y++) {
          if (not Broodwar->hasCreep(TilePosition(x, y))) {
            return false;
          }
        }
      }
    }
  }

  //Step 7: If detector, check if spot is already covered by a detector
  if (to_build.isDetector()) {
    if (not suitable_for_detector(build_spot)) {
      return false;
    }
  }

  //All passed. It is possible to build here.
  return true;
}

TilePosition BuildingPlacer::find_build_spot(UnitType to_build) {
  range_ = 40;
  if (to_build.getID() == UnitTypes::Protoss_Pylon.getID()) range_ = 80;

  //Refinery
  if (to_build.isRefinery()) {
    //Use refinery method
    return find_refinery_build_spot(to_build, Broodwar->self()->getStartLocation());
  }

  //If we find unpowered buildings, build a Pylon there
  if (BaseAgent::isOfType(to_build, UnitTypes::Protoss_Pylon)) {
    auto& agents = rnp::agent_manager()->getAgents();
    for (auto& a : agents) {
      if (a->isAlive()) {
        Unit cUnit = a->getUnit();
        if (not cUnit->isPowered()) {
          TilePosition spot = find_build_spot(to_build, cUnit->getTilePosition());
          return spot;
        }
      }
    }
  }

  //Build near chokepoints: Bunker, Photon Cannon, Creep Colony
  if (BaseAgent::isOfType(to_build, UnitTypes::Terran_Bunker) 
    || BaseAgent::isOfType(to_build, UnitTypes::Protoss_Photon_Cannon) 
    || BaseAgent::isOfType(to_build, UnitTypes::Zerg_Creep_Colony)) {
    TilePosition cp = rnp::commander()->find_chokepoint();
    if (rnp::is_valid_position(cp)) {
      TilePosition spot = find_build_spot(to_build, cp);
      return spot;
    }
  }

  //Base buildings.
  if (to_build.isResourceDepot()) {
    TilePosition start = rnp::exploration()->search_expansion_site();
    if (rnp::is_valid_position(start)) {
      //Expansion site found. Build close to it.
      TilePosition spot = find_build_spot(to_build, start);
      return spot;
    }
  }

  //General building. Search for spot around bases
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive() && a->getUnitType().isResourceDepot() && not base_under_construction(a)) {
      TilePosition start = a->getUnit()->getTilePosition();
      TilePosition bSpot = find_build_spot(to_build, start);
      if (rnp::is_valid_position(bSpot)) {
        //Spot found, return it.
        return bSpot;
      }
    }
  }

  return TilePosition(-1, -1);
}

bool BuildingPlacer::is_defense_building(UnitType toBuild) {
  int bId = toBuild.getID();
  if (bId == UnitTypes::Terran_Bunker.getID()) return true;
  if (bId == UnitTypes::Protoss_Photon_Cannon.getID()) return true;
  if (bId == UnitTypes::Zerg_Creep_Colony.getID()) return true;

  return false;
}


bool BuildingPlacer::base_under_construction(BaseAgent* base) {
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

TilePosition BuildingPlacer::find_spot_at_side(UnitType to_build, TilePosition start, TilePosition end) {
  int dX = end.x - start.x;
  if (dX != 0) dX = 1;
  RNP_ASSERT(dX == 0 || dX == 1);

  int dY = end.y - start.y;
  if (dY != 0) dY = 1;
  RNP_ASSERT(dY == 0 || dY == 1);

  TilePosition test_pos(start);
  bool done = false;

  while (not done) {
    if (can_build_at(to_build, test_pos)) return test_pos;
    int cX = test_pos.x + dX;
    int cY = test_pos.y + dY;
    test_pos = TilePosition(cX, cY);
    if (test_pos.x == end.x && test_pos.y == end.y) done = true;
  }

  return TilePosition {-1, -1};
}

bool BuildingPlacer::can_build_at(UnitType to_build, TilePosition pos) const {
  int maxW = w_ - to_build.tileWidth() - 1;
  int maxH = h_ - to_build.tileHeight() - 1;

  //Out of bounds check
  if (pos.x >= 0 && pos.x < maxW && pos.y >= 0 && pos.y < maxH) {
    if (can_build(to_build, pos)) {
      return true;
    }
  }
  return false;
}

TilePosition BuildingPlacer::find_build_spot(UnitType to_build, TilePosition start) {
  //Check start pos
  if (can_build_at(to_build, start)) {
    return start;
  }

  //Search outwards
  int search_dist = 1;
  TilePosition spot;
  
  while (true) {
    //Top
    TilePosition s(start.x - search_dist, start.y - search_dist);
    TilePosition e(start.x + search_dist, start.y - search_dist);
    spot = find_spot_at_side(to_build, s, e);
    if (rnp::is_valid_position(spot)) {
      return spot;
    }

    //Bottom
    s = TilePosition(start.x - search_dist, start.y + search_dist);
    e = TilePosition(start.x + search_dist, start.y + search_dist);
    spot = find_spot_at_side(to_build, s, e);
    if (rnp::is_valid_position(spot)) {
      return spot;
    }

    //Left
    s = TilePosition(start.x - search_dist, start.y - search_dist);
    e = TilePosition(start.x - search_dist, start.y + search_dist);
    spot = find_spot_at_side(to_build, s, e);
    if (rnp::is_valid_position(spot)) {
      return spot;
    }

    //Right
    s = TilePosition(start.x + search_dist, start.y - search_dist);
    e = TilePosition(start.x + search_dist, start.y + search_dist);
    spot = find_spot_at_side(to_build, s, e);
    if (rnp::is_valid_position(spot)) {
      return spot;
    }

    search_dist++;
    if (search_dist > range_) {
      break;
    }
  }

  return TilePosition {-1, -1};
}

void BuildingPlacer::add_constructed_building(Unit unit) const {
  if (unit->getType().isAddon()) {
    //Addons are handled by their main buildings
    return;
  }

  Corners c = get_corners(unit);
  fill(c);
}

void BuildingPlacer::on_building_destroyed(Unit unit) const {
  if (unit->getType().isAddon()) {
    //Addons are handled by their main buildings
    return;
  }

  Corners c = get_corners(unit);
  clear(c);
}

void BuildingPlacer::fill(Corners c) const {
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

void BuildingPlacer::fill_temp(UnitType toBuild, TilePosition buildSpot) const {
  Corners c = get_corners(toBuild, buildSpot);

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

void BuildingPlacer::clear(Corners c) const {
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

void BuildingPlacer::clear_temp(UnitType toBuild, TilePosition buildSpot) const {
  if (not rnp::is_valid_position(buildSpot)) {
    return;
  }

  Corners c = get_corners(toBuild, buildSpot);

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

Corners BuildingPlacer::get_corners(Unit unit) const {
  return get_corners(unit->getType(), unit->getTilePosition());
}

Corners BuildingPlacer::get_corners(UnitType type, TilePosition center) const {
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

TilePosition BuildingPlacer::find_refinery_build_spot(UnitType toBuild, TilePosition start) const {
  TilePosition buildSpot = find_closest_gas_without_refinery(toBuild, start);
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

TilePosition BuildingPlacer::find_closest_gas_without_refinery(UnitType toBuild, TilePosition start) const {
  TilePosition bestSpot = TilePosition(-1, -1);
  double bestDist = -1;
  TilePosition home = Broodwar->self()->getStartLocation();
  Unit worker = find_worker(start);

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
          if (rnp::exploration()->can_reach(home, cPos)) {
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

TilePosition BuildingPlacer::search_refinery_spot() const {
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
              if (rnp::exploration()->can_reach(bPos, cPos)) {
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

TilePosition BuildingPlacer::find_expansion_site() const {
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
          if (rnp::exploration()->can_reach(Broodwar->self()->getStartLocation(), pos)) {
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

Unit BuildingPlacer::find_closest_mineral(TilePosition workerPos) const {
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
            Unit cMineral = has_mineral_near(pos);
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

Unit BuildingPlacer::has_mineral_near(TilePosition pos) {
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

bool BuildingPlacer::suitable_for_detector(TilePosition pos) {
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
