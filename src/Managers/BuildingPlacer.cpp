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

CoverMap::CoverMap(size_t xsize, size_t ysize)
    : width_(xsize), height_(ysize), data_()
{
  data_.resize(width_ * height_);

  for (size_t yi = 0; yi < height_; yi++) {
    //Fill from static map and Region connectability
    for (size_t xi = 0; xi < width_; xi++) {
      auto ok = Broodwar->isBuildable(xi, yi, true) 
                  ? TileType::BUILDABLE
                  : TileType::BLOCKED;
      set(xi, yi, ok);
    }
  }
}

bool CoverMap::is_buildable(const BuildingBox& bbox) const {
  if (bbox.x1 < 0 || bbox.x2 >= (int)width_
    || bbox.y1 < 0 || bbox.y2 >= (int)height_) {
    return false;
  }

  for (int x = bbox.x1; x <= bbox.x2; x++) {
    for (int y = bbox.y1; y <= bbox.y2; y++) {
      if (get(x, y) != TileType::BUILDABLE) {
        //Cant build here.
        return false;
      }
    }
  }

  return true;
}

BuildingPlacer::BuildingPlacer() 
  : range_(40)
  , cover_map_(Broodwar->mapWidth(), Broodwar->mapHeight())
  , bwem_(BWEM::Map::Instance())
{
  map_width_ = Broodwar->mapWidth();
  map_height_ = Broodwar->mapHeight();
  RNP_ASSERT(map_width_);
  RNP_ASSERT(map_height_);

//  Unit worker = find_worker(Broodwar->self()->getStartLocation());

  //Fill from current agents
  act::for_each_actor<BaseAgent>(
    [this](const BaseAgent* a) {
      if (a->is_building()) {
        auto c = get_building_box(a->get_unit());
        fill(c);
      }
    });

  //Fill from minerals
  for (auto& u : Broodwar->getMinerals()) {
    BuildingBox c;
    c.x1 = u->getTilePosition().x - 2;
    c.y1 = u->getTilePosition().y - 2;
    c.x2 = u->getTilePosition().x + 2;
    c.y2 = u->getTilePosition().y + 2;
    fill(c);

    cover_map_.set(c.x1 + 2, c.y1 + 2, TileType::MINERAL);
  }

  //Fill from gas
  for (auto& u : Broodwar->getGeysers()) {
    BuildingBox c;
    c.x1 = u->getTilePosition().x - 2;
    c.y1 = u->getTilePosition().y - 2;
    c.x2 = u->getTilePosition().x + 5;
    c.y2 = u->getTilePosition().y + 3;
    fill(c);

    cover_map_.set(c.x1 + 2, c.y1 + 2, TileType::GAS);
  }

  // Fill from narrow chokepoints
  for (auto& area : bwem_.Areas()) {
    for (auto choke : area.ChokePoints()) {
      if (rnp::choke_width(choke) <= 4 * 32) {
        TilePosition center(choke->Center());
        BuildingBox c;
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
      BuildingBox c = get_building_box(u);
      fill(c);
    }
  }
}

BuildingPlacer::~BuildingPlacer() {
}

Unit BuildingPlacer::find_worker(TilePosition spot) {
  auto worker = rnp::agent_manager()->find_closest_free_worker(spot);
  return worker ? worker->get_unit() : nullptr;
}

bool BuildingPlacer::is_position_available(TilePosition pos) const {
  if (cover_map_.get(pos.x, pos.y) == TileType::BUILDABLE) {
    return true;
  }
  return false;
}

void BuildingPlacer::mark_position_blocked(TilePosition buildSpot) {
  if (not rnp::is_valid_position(buildSpot)) {
    //Error check
    return;
  }
  cover_map_.set(buildSpot.x, buildSpot.y, TileType::BLOCKED);
}

bool BuildingPlacer::can_build(UnitType to_build, TilePosition build_spot) const {
  BuildingBox bbox(get_building_box(to_build, build_spot));
  RNP_ASSERT(bbox.x2 >= bbox.x1 && bbox.y2 >= bbox.y1);

  //Step 1: Check BuildingPlacer.
  if (not cover_map_.is_buildable(bbox)) {
    return false;
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
  if (Constructor::is_protoss()) {
    if (to_build.requiresPsi()) {
      if (not Broodwar->hasPower(build_spot, to_build)) {
        return false;
      }
    }

    //Spread out Pylons
    auto loop_result = act::interruptible_for_each_actor<BaseAgent>(
        [&build_spot](const BaseAgent* a) {
            if (a->unit_type().getID() == UnitTypes::Protoss_Pylon.getID()) {
              if (a->get_unit()->getTilePosition().getDistance(build_spot) <= 3) {
                return act::ForEach::Break;
              }
            }
            return act::ForEach::Continue;
        });
    if (loop_result == act::ForEachResult::Interrupted) { return false; }
  }

  //Step 6: If Zerg, check creep
  if (Constructor::is_zerg()) {
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
  if (rnp::same_type(to_build, UnitTypes::Protoss_Pylon)) {
    range_ = 80;
  }

  auto closest_base = Broodwar->self()->getStartLocation();

  //Refinery
  if (to_build.isRefinery()) {
    //Use refinery method
    return find_refinery_build_spot(to_build, closest_base);
  }

  //If we find unpowered buildings, build a Pylon there
  if (rnp::same_type(to_build, UnitTypes::Protoss_Pylon)) {
    TilePosition spot;
    auto loop_result = act::interruptible_for_each_actor<BaseAgent>(
        [this,&spot,&to_build](const BaseAgent* a) {
            auto c_unit = a->get_unit();
            if (not c_unit->isPowered()) {
              spot = find_build_spot(to_build, c_unit->getTilePosition());
              return act::ForEach::Break;
            }
            return act::ForEach::Continue;
        });
    if (loop_result == act::ForEachResult::Interrupted) {
      return spot;
    }
  }

  //Build near chokepoints: Bunker, Photon Cannon, Creep Colony
  if (rnp::same_type(to_build, UnitTypes::Terran_Bunker) 
    || rnp::same_type(to_build, UnitTypes::Protoss_Photon_Cannon) 
    || rnp::same_type(to_build, UnitTypes::Zerg_Creep_Colony)) 
  {
    TilePosition cp = rnp::commander()->find_chokepoint();
    if (rnp::is_valid_position(cp)) {
      // Ensure that chokepoint isn't too far, otherwise take the base center?
      // Cut 20% from the distance until its fairly close
      while (cp.getDistance(closest_base) > 20) {
        cp = TilePosition(closest_base.x + (cp.x - closest_base.x) * 4 / 5,
                          closest_base.y + (cp.y - closest_base.y) * 4 / 5);
      }

      TilePosition spot = find_build_spot(to_build, cp);
      rnp::log()->debug("bp: spot for [def] {0} {1};{2}", 
                        to_build.getName(), spot.x, spot.y);
      return spot;
    }
  }

  //Base buildings.
  if (to_build.isResourceDepot()) {
    TilePosition start = rnp::exploration()->get_expansion_site();
    if (rnp::is_valid_position(start)) {
      //Expansion site found. Build close to it.
      TilePosition spot = find_build_spot(to_build, start);
      rnp::log()->debug("bp: spot for [res] {0} {1};{2}",
                        to_build.getName(), spot.x, spot.y);
      return spot;
    }
  }

  //General building. Search for spot around bases
  TilePosition b_spot;
  auto loop_result = act::interruptible_for_each_actor<BaseAgent>(
      [&to_build,&b_spot,this](const BaseAgent* a) {
          if (a->unit_type().isResourceDepot()
              && not base_under_construction(a))
          {
            auto start = a->get_unit()->getTilePosition();
            b_spot = find_build_spot(to_build, start);
            if (rnp::is_valid_position(b_spot)) {
              //Spot found, return it.
              rnp::log()->debug("bp: spot for [other] {0} {1};{2}",
                                to_build.getName(), b_spot.x, b_spot.y);
              return act::ForEach::Break;
            }
          } // if
          return act::ForEach::Continue;
      });
  if (loop_result == act::ForEachResult::Interrupted) {
    return b_spot;
  }

  rnp::log()->debug("bp: no spot found for {0}", to_build.getName());
  return rnp::make_bad_position();
}

bool BuildingPlacer::is_defense_building(UnitType toBuild) {
  return (rnp::same_type(toBuild, UnitTypes::Terran_Bunker)
    || rnp::same_type(toBuild, UnitTypes::Protoss_Photon_Cannon)
    || rnp::same_type(toBuild, UnitTypes::Zerg_Creep_Colony));
}


bool BuildingPlacer::base_under_construction(const BaseAgent* base) {
  if (Constructor::is_terran()) {
    return base->get_unit()->isBeingConstructed();
  }
  if (Constructor::is_protoss()) {
    return base->get_unit()->isBeingConstructed();
  }
  if (Constructor::is_zerg()) {
    if (base->is_of_type(UnitTypes::Zerg_Hatchery)) {
      return base->get_unit()->isBeingConstructed();
    }
  }
  return false;
}

TilePosition BuildingPlacer::find_spot_at_side(UnitType to_build,
                                               TilePosition start,
                                               TilePosition end) const {
  int dx = (end.x != start.x) ? 1 : 0;
  int dy = (end.y != start.y) ? 1 : 0;
  RNP_ASSERT(dx != 0 || dy != 0);

  auto test_pos = start;

  while (test_pos.x != end.x || test_pos.y != end.y) {
    if (can_build_at(to_build, test_pos)) {
      return test_pos;
    }
    test_pos = TilePosition(test_pos.x + dx, test_pos.y + dy);
  }

  return rnp::make_bad_position();
}

bool BuildingPlacer::can_build_at(UnitType to_build, TilePosition pos) const {
  int maxW = map_width_ - to_build.tileWidth() - 1;
  int maxH = map_height_ - to_build.tileHeight() - 1;

  //Out of bounds check
  if (pos.x >= 0 && pos.x < maxW && pos.y >= 0 && pos.y < maxH) {
    if (can_build(to_build, pos)) {
      return true;
    }
  }
  return false;
}

TilePosition BuildingPlacer::find_build_spot(UnitType to_build, 
                                             TilePosition search_origin) const {
  //Check start pos
  if (can_build_at(to_build, search_origin)) {
    return search_origin;
  }

  //
  //Search outwards making a spiral from center
  // 
  for (size_t search_dist = 1; search_dist <= range_; ++search_dist) {
    //Top
    TilePosition start(search_origin.x - search_dist, search_origin.y - search_dist);
    TilePosition end(search_origin.x + search_dist, search_origin.y - search_dist);
    auto spot = find_spot_at_side(to_build, start, end);
    if (rnp::is_valid_position(spot)) {
      return spot;
    }

    //Bottom
    start = TilePosition(search_origin.x - search_dist, search_origin.y + search_dist);
    end = TilePosition(search_origin.x + search_dist, search_origin.y + search_dist);
    spot = find_spot_at_side(to_build, start, end);
    if (rnp::is_valid_position(spot)) {
      return spot;
    }

    //Left
    start = TilePosition(search_origin.x - search_dist, search_origin.y - search_dist);
    end = TilePosition(search_origin.x - search_dist, search_origin.y + search_dist);
    spot = find_spot_at_side(to_build, start, end);
    if (rnp::is_valid_position(spot)) {
      return spot;
    }

    //Right
    start = TilePosition(search_origin.x + search_dist, search_origin.y - search_dist);
    end = TilePosition(search_origin.x + search_dist, search_origin.y + search_dist);
    spot = find_spot_at_side(to_build, start, end);
    if (rnp::is_valid_position(spot)) {
      return spot;
    }
  }

  return rnp::make_bad_position();
}

void BuildingPlacer::add_constructed_building(Unit unit) {
  if (unit->getType().isAddon()) {
    //Addons are handled by their main buildings
    return;
  }

  BuildingBox c = get_building_box(unit);
  fill(c);
}

void BuildingPlacer::on_building_destroyed(Unit unit) {
  if (unit->getType().isAddon()) {
    //Addons are handled by their main buildings
    return;
  }

  BuildingBox c = get_building_box(unit);
  clear(c);
}

void BuildingPlacer::fill(BuildingBox c) {
  for (int x = c.x1; x <= c.x2; x++) {
    for (int y = c.y1; y <= c.y2; y++) {
      if (x >= 0 && x < (int)map_width_ 
          && y >= 0 && y < (int)map_height_) {
        if (cover_map_.get(x, y) == TileType::BUILDABLE) {
          cover_map_.set(x, y, TileType::BLOCKED);
        }
        if (cover_map_.get(x, y) == TileType::TEMPBLOCKED) {
          cover_map_.set(x, y, TileType::BLOCKED);
        }
      }
    }
  }
}

void BuildingPlacer::fill_temp(UnitType toBuild, TilePosition buildSpot) {
  BuildingBox c = get_building_box(toBuild, buildSpot);

  for (int x = c.x1; x <= c.x2; x++) {
    for (int y = c.y1; y <= c.y2; y++) {
      if (x >= 0 && x < (int)map_width_ 
          && y >= 0 && y < (int)map_height_) {
        if (cover_map_.get(x, y) == TileType::BUILDABLE) {
          cover_map_.set(x, y, TileType::TEMPBLOCKED);
        }
      }
    }
  }
}

void BuildingPlacer::clear(const BuildingBox& c) {
  for (int x = c.x1; x <= c.x2; x++) {
    for (int y = c.y1; y <= c.y2; y++) {
      if (x >= 0 && x < (int)map_width_ 
          && y >= 0 && y < (int)map_height_) {
        if (cover_map_.get(x, y) == TileType::BLOCKED) {
          cover_map_.set(x, y, TileType::BUILDABLE);
        }
        if (cover_map_.get(x, y) == TileType::TEMPBLOCKED) {
          cover_map_.set(x, y, TileType::BUILDABLE);
        }
      }
    }
  }
}

void BuildingPlacer::clear_temp(UnitType to_build, 
                                const TilePosition& build_spot) {
  if (not rnp::is_valid_position(build_spot)) {
    return;
  }

  BuildingBox c = get_building_box(to_build, build_spot);

  for (int x = c.x1; x <= c.x2; x++) {
    for (int y = c.y1; y <= c.y2; y++) {
      if (x >= 0 && x < (int)map_width_ 
          && y >= 0 && y < (int)map_height_) {
        if (cover_map_.get(x, y) == TileType::TEMPBLOCKED) {
          cover_map_.set(x, y, TileType::BUILDABLE);
        }
      }
    }
  }
}

BuildingBox BuildingPlacer::get_building_box(Unit unit) const {
  return get_building_box(unit->getType(), unit->getTilePosition());
}

BuildingBox BuildingPlacer::get_building_box(UnitType type,
                                             const TilePosition& origin) const {
  int x1 = origin.x;
  int y1 = origin.y;
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
  if (rnp::same_type(type, UnitTypes::Terran_Factory) 
    || rnp::same_type(type, UnitTypes::Terran_Starport) 
    || rnp::same_type(type, UnitTypes::Terran_Command_Center) 
    || rnp::same_type(type, UnitTypes::Terran_Science_Facility)) {
    x2 += 2;
  }

  return BuildingBox(x1, y1, x2, y2);
}

TilePosition BuildingPlacer::find_refinery_build_spot(
  UnitType toBuild, const TilePosition& start) const 
{
  TilePosition buildSpot = find_closest_gas_without_refinery(toBuild, start);
  if (rnp::is_valid_position(buildSpot)) {
    auto base = rnp::agent_manager()->get_closest_base(buildSpot);
    if (base == nullptr) {
      Broodwar << "No base found" << std::endl;
      return rnp::make_bad_position();
    }
    double dist = buildSpot.getDistance(base->get_unit()->getTilePosition());
    if (dist >= 14) {
      return rnp::make_bad_position();
    }
  }
  return buildSpot;
}

TilePosition BuildingPlacer::find_closest_gas_without_refinery(
  UnitType toBuild, const TilePosition& start) const 
{
  TilePosition best_spot(-1, -1);
  float best_dist = -1.0f;
  TilePosition home = Broodwar->self()->getStartLocation();
  //Unit worker = find_worker(start);

  for (size_t i = 0; i < map_width_; i++) {
    for (size_t j = 0; j < map_height_; j++) {
      if (cover_map_.get(i, j) == TileType::GAS) {
        TilePosition c_pos(i, j);
        bool ok = true;

        act::for_each_actor<BaseAgent>(
          [&ok,&c_pos](const BaseAgent* a) {
            Unit unit = a->get_unit();
            if (unit->getType().isRefinery()) {
              float dist = rnp::distance(unit->getTilePosition(), c_pos);
              if (dist <= 2) {
                ok = false;
              }
            }
          });

        if (ok) {
          if (rnp::exploration()->can_reach(home, c_pos)) {
            auto agent = rnp::agent_manager()->get_closest_base(c_pos);
            float dist = rnp::distance(agent->get_unit()->getTilePosition(), 
                                       c_pos);
            if (best_dist < 0.0f || dist < best_dist) {
              best_dist = dist;
              best_spot = c_pos;
            }
          }
        } // if ok
      } // if gas
    } // for j
  } // for i

  return best_spot;
}

TilePosition BuildingPlacer::search_refinery_spot() const {
  for (size_t i = 0; i < map_width_; i++) {
    for (size_t j = 0; j < map_height_; j++) {
      if (cover_map_.get(i, j) == TileType::GAS) {
        TilePosition c_pos = TilePosition(i, j);

        bool found = false;
        act::for_each_actor<BaseAgent>(
          [&found,&c_pos](const BaseAgent* a) {
            if (a->unit_type().isRefinery()) {
              float dist = rnp::distance(a->get_unit()->getTilePosition(),
                                         c_pos);
              //TilePosition uPos = a->get_unit()->getTilePosition();
              if (dist <= 2.0f) {
                found = true;
                return;
              }
            }
          });

        if (not found) {
          auto closest_base = rnp::agent_manager()->get_closest_base(c_pos);
          if (closest_base != nullptr) {
            auto bPos = closest_base->get_unit()->getTilePosition();
            auto dist = rnp::distance(bPos, c_pos);
            if (dist < 15.0f) {
              if (rnp::exploration()->can_reach(bPos, c_pos)) {
                return c_pos;
              }
            }
          }
        } // if not found
      } // if gas
    } // for j
  } // for i

  return rnp::make_bad_position();
}

TilePosition BuildingPlacer::find_expansion_site() const {
  //UnitType baseType = Broodwar->self()->getRace().getResourceDepot;
  float best_dist = 1e+12f;
  TilePosition best_pos = rnp::make_bad_position();
  auto start_loc = Broodwar->self()->getStartLocation();

  //Iterate through all base locations
  rnp::for_each_base(
    [&best_dist,&best_pos,&start_loc](const BWEM::Base& base) {
      TilePosition pos(base.Center());

      if (pos.x != Broodwar->self()->getStartLocation().x
        || pos.y != Broodwar->self()->getStartLocation().y) {
        bool taken = false;

        //Check if own buildings are close
        if (rnp::map_manager()->is_under_my_influence(pos)) taken = true;
        //Check if enemy buildings are close
        if (rnp::map_manager()->is_under_enemy_influence(pos)) taken = true;

        //Not taken, calculate ground distance
        if (not taken) {
          if (rnp::exploration()->can_reach(start_loc, pos)) {
            float dist = rnp::pathfinder()->get_dist(start_loc, pos);
            if (dist <= best_dist && dist > 0) {
              best_dist = dist;
              best_pos = pos;
            }
          }
        } // not taken
      } // if pos not start location
    });
   
  //Don't build expansions too far away!
  auto base = rnp::agent_manager()->get_closest_base(best_pos);
  if (base != nullptr) {
    auto d = base->get_unit()->getTilePosition().getDistance(best_pos);
    if (d >= 140) {
      Broodwar << "Expansion site too far away: " << d << std::endl;
      return rnp::make_bad_position();
    }
  }
  else {
    return rnp::make_bad_position();
  }

  return best_pos;
}

Unit BuildingPlacer::find_closest_mineral(TilePosition workerPos) const {
  Unit mineral = nullptr;
  float best_dist = 1e+12f;

  for (auto& area : bwem_.Areas()) {
    for (auto& base : area.Bases()) {
      TilePosition pos(base.Center());
      float c_dist = rnp::distance(pos, workerPos);
      
      if (c_dist < best_dist) {
        //Find closest base
        auto base = rnp::agent_manager()->get_closest_base(pos);
        
        if (base) {
          float dist = rnp::distance(pos, base->get_unit()->getTilePosition());
          if (dist <= 12.0f) {
            //We have a base near this base location
            //Check if we have minerals available
            Unit c_mineral = has_mineral_near(pos);
            if (c_mineral != nullptr) {
              mineral = c_mineral;
              best_dist = c_dist;
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
      float dist = rnp::distance(pos, u->getTilePosition());
      if (dist <= 10.0f) {
        return u;
      }
    }
  }
  return nullptr;
}

bool BuildingPlacer::suitable_for_detector(TilePosition pos) {
  auto result = true;
  act::for_each_actor<BaseAgent>(
    [&pos,&result](const BaseAgent* a) {
      auto type = a->unit_type();
      if (type.isDetector() && type.isBuilding()) {
        auto d_range = type.sightRange() * 1.6f;
        auto dist = rnp::distance(a->get_unit()->getPosition(), Position(pos));
        if (dist <= d_range) {
          result = false;
          return;
        }
      }
    });
  return result;
}

void BuildingPlacer::show_debug() const {
  for (size_t x = 0; x < map_width_; x++) {
    for (size_t y = 0; y < map_height_; y++) {
      if (cover_map_.get(x, y) == TileType::TEMPBLOCKED) {
        Broodwar->drawBoxMap(x * TILEPOSITION_SCALE, 
                             y * TILEPOSITION_SCALE, 
                             x * TILEPOSITION_SCALE + TILEPOSITION_SCALE - 1,
                             y * TILEPOSITION_SCALE + TILEPOSITION_SCALE - 1,
                             Colors::Green, false);
      }
      if (cover_map_.get(x, y) == TileType::BLOCKED) {
        Broodwar->drawBoxMap(x * TILEPOSITION_SCALE, 
                             y * TILEPOSITION_SCALE, 
                             x * TILEPOSITION_SCALE + TILEPOSITION_SCALE - 1,
                             y * TILEPOSITION_SCALE + TILEPOSITION_SCALE - 1,
                             Colors::Brown, false);
      }
    }
  }
}
