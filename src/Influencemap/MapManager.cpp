#include "MapManager.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include "MainAgents/BaseAgent.h"

#include "BWEM/bwem.h"
#include "RnpUtil.h"
#include "Glob.h"

using namespace BWAPI;

MapManager::MapManager()
    : map_(), bases_(), bwem_(BWEM::Map::Instance())
{
  //Add the regions for this map
  for (auto& r : bwem_.Areas()) {
    MRegion* mr = new MRegion();
    mr->region_ = &r;
    mr->resetInfluence();
    map_.insert(mr);

    //Add base locations for this map
    for (auto& base : r.Bases()) {
      bases_.insert(std::make_unique<BaseLocation>(base.Location()));
    }
  }
  last_call_frame_ = 0;
}

const MRegion* MapManager::get_mregion(const BWEM::Area* area) {
  auto a_center = rnp::get_center(area);
  for (MRegion* mr : map_) {
    auto center = rnp::get_center(mr->region_);
    if (center.x == a_center.x && center.y == a_center.y) {
      return mr;
    }
  }
  return nullptr;
}

bool MapManager::is_valid_chokepoint(const BWEM::ChokePoint* cp) {
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

const BWEM::ChokePoint* MapManager::find_guard_chokepoint(const MRegion* mr) {
  auto mr_center = rnp::get_center(mr->region_);

  for (auto c : mr->region_->ChokePoints()) {
    if (is_valid_chokepoint(c)) {
      auto connected_areas = c->GetAreas();
      auto first_center = rnp::get_center(connected_areas.first);
      
      if (first_center.x == mr_center.x && first_center.y == mr_center.y) {
        auto adj = get_mregion(connected_areas.second);
        if (adj->inf_own_buildings == 0) {
          return c;
        }
      }

      auto second_center = rnp::get_center(connected_areas.second);
      if (second_center.x == mr_center.x && second_center.y == mr_center.y) {
        auto adj = get_mregion(connected_areas.first);
        if (adj->inf_own_buildings == 0) {
          return c;
        }
      }
    }
  }
  return nullptr;
}

const BWEM::ChokePoint* MapManager::get_defense_location() {
  int bestInfluence = 0;
  const BWEM::ChokePoint* best = nullptr;

  for (MRegion* base : map_) {
    if (base->inf_own_buildings > bestInfluence) {
      const BWEM::ChokePoint* cp = find_guard_chokepoint(base);
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

MRegion* MapManager::get_mregion_for(Position p) {
  for (auto& mreg : map_) {
    if (rnp::is_inside(*mreg->region_, p)) {
      return mreg;
    }
  }
  return nullptr;
}

bool MapManager::has_enemy_influence() {
  for (auto& mr : map_) {
    if (mr->inf_en_buildings > 0) {
      return true;
    }
  }
  return false;
}

void MapManager::update_influences() {
  //Dont call too often
  int now_frame = Broodwar->getFrameCount();
  if (now_frame - last_call_frame_ < 10) {
    return;
  }
  last_call_frame_ = now_frame;

  //Reset previous influence scores
  for (MRegion* mr : map_) {
    mr->resetInfluence();
  }

  //Update visited base locations
  for (auto& a : bases_) {
    if (Broodwar->isVisible(a->base_location_)) {
      a->frame_visited_ = Broodwar->getFrameCount();
    }
  }

  //Update own influence
  act::for_each_actor<BaseAgent>(
    [this](const BaseAgent* a) {
      auto pos = a->get_unit()->getPosition();
      if (rnp::is_unknown_position(pos)) { return; }

      auto mr = get_mregion_for(pos);
      if (mr) {
        if (a->unit_type().isBuilding()) {
          mr->inf_own_buildings += a->unit_type().buildScore();
          if (a->unit_type().canAttack()) {
            if (a->can_target_ground()) {
              mr->inf_own_ground += a->unit_type().buildScore();
            }
            if (a->can_target_air()) {
              mr->inf_own_air += a->unit_type().buildScore();
            }
          }
        }
        else if (a->unit_type().isAddon()) {
          //No influence from addons
        }
        else if (a->unit_type().isWorker()) {
          //No influence for workers
        }
        else {
          //Regular units
          if (a->unit_type().canAttack()) {
            if (a->can_target_ground()) {
              mr->inf_own_ground += a->unit_type().buildScore();
            }
            if (a->can_target_air()) {
              mr->inf_own_air += a->unit_type().buildScore();
            }
          }
        }
      }
    });

  //Update enemy buildings influence
  for (MRegion* mr : map_) {
    mr->inf_en_buildings = rnp::exploration()->get_spotted_influence_in_region(mr->region_);
  }

  //Update enemy units influence
  for (auto& u : Broodwar->enemy()->getUnits()) {
    //Enemy seen
    if (u->exists()) {
      MRegion* mr = get_mregion_for(u->getPosition());
      if (mr) {
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

int MapManager::get_my_influence_at(TilePosition pos) {
  for (MRegion* cm : map_) {
    if (rnp::is_inside(*cm->region_, Position(pos))) {
      return cm->inf_own_ground;
    }
  }
  return 0;
}

int MapManager::get_ground_influence_at(TilePosition pos) {
  for (MRegion* cm : map_) {
    if (rnp::is_inside(*cm->region_, Position(pos))) {
      return cm->inf_en_ground;
    }
  }
  return 0;
}

bool MapManager::is_under_my_influence(TilePosition pos) {
  for (MRegion* cm : map_) {
    if (cm->inf_own_buildings > 0 
      && rnp::is_inside(*cm->region_, Position(pos))) {
      return true;
    }
  }
  return false;
}

bool MapManager::is_under_enemy_influence(TilePosition pos) {
  for (MRegion* cm : map_) {
    if (cm->inf_en_buildings > 0 
      && rnp::is_inside(*cm->region_, Position(pos))) {
      return true;
    }
  }
  return false;
}

TilePosition MapManager::find_suitable_attack_position() {
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
    return TilePosition(rnp::get_center(best->region_));
  }
  else {
    //No enemy building found. Move to starting positions.
    int longestVisitFrame = Broodwar->getFrameCount();
    TilePosition base = rnp::make_bad_position();
    for (auto& a : bases_) {
      if (a->frame_visited_ < longestVisitFrame) {
        longestVisitFrame = a->frame_visited_;
        base = a->base_location_;
      }
    }

    return base;
  }
}

MapManager::~MapManager() {
  for (MRegion* mr : map_) {
    delete mr;
  }
}

void MapManager::debug_print_info() {
  for (MRegion* mr : map_) {
    auto mr_center = rnp::get_center(mr->region_);
    /*
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
    */
    //Print location of each chokepoint, and also if it is blocked
    //as defense position.
    for (auto c : mr->region_->ChokePoints()) {
      auto c_center = c->Center();
      int x3 = c_center.x * WALKPOSITION_SCALE;
      int y3 = c_center.y * WALKPOSITION_SCALE;
      Broodwar->drawTextMap(x3, y3, "(%d,%d)", x3, y3);
      if (not is_valid_chokepoint(c)) Broodwar->drawTextMap(x3, y3 + 12, "Blocked");
    }
    Broodwar->drawTextScreen(10, 120, "'%s'", Broodwar->mapFileName().c_str());
  }

  for (auto& a : bwem_.Areas()) {
    for (auto& cp : a.ChokePoints()) {
      for (auto &geom : cp->Geometry()) {
        Broodwar->drawCircleMap(geom.x * WALKPOSITION_SCALE, 
                                geom.y * WALKPOSITION_SCALE, 
                                5, Colors::Green, false);
      }
    }
  }
}
