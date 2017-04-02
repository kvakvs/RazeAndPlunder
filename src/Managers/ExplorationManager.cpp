#include "ExplorationManager.h"
#include "BuildingPlacer.h"
#include "RnpUtil.h"
#include "Glob.h"
#include "PathAB.h"
#include "Influencemap/MapManager.h"
#include "RnpRandom.h"
#include "RnpConst.h"
#include "Utils/Profiler.h"

using namespace BWAPI;

RegionItem::RegionItem(const BWEM::Area* region): location_() {
  location_ = rnp::get_center(*region);
  rnp::log()->trace("Explore area registered with center at {0};{1}",
                    location_.x, location_.y);
  visited();
}

RegionItem::RegionItem(const BWAPI::TilePosition& pos) : location_(pos) {
  visited();
}

ExplorationManager::ExplorationManager()
    : bwem_(BWEM::Map::Instance()), enemy_buildings_(), explore_(), expansion_site_()
{
  //Add the regions for this map
  for (auto& r : bwem_.Areas()) {
    explore_.insert(std::make_unique<RegionItem>(&r));
  }

  site_set_frame_ = 0;

  last_call_frame_ = Broodwar->getFrameCount();

  expansion_site_ = rnp::make_bad_position();
}

ExplorationManager::~ExplorationManager() {
}

void ExplorationManager::tick() {
  ProfilerAuto p_auto(*rnp::profiler(), "OnFrame_ExplorationManager");
  cleanup();

  // This updates like once/500 frames
  search_expansion_site();

  //Dont call too often
  int c_frame = Broodwar->getFrameCount();
  if (c_frame - last_call_frame_ < 9) {
    return;
  }
  last_call_frame_ = c_frame;

  for (auto& region_item : explore_) {
    if (Broodwar->isVisible(region_item->location())) {
      region_item->visited();
    }
  }
}

void ExplorationManager::handle_message(act::Message* incoming) {
  unhandled_message(incoming);
}

act::ActorId::Set 
ExplorationManager::actors_in_range(const BWAPI::TilePosition& center, 
                                    int radius) const {
  act::ActorId::Set found_ids;
  float fradius = static_cast<float>(radius);
  // TODO: optimize me
  act::for_each_actor<BaseAgent>(
    [&center,fradius,&found_ids](const BaseAgent* a) {
      if (rnp::distance(a->get_unit()->getTilePosition(), center) < fradius) {
        found_ids.insert(a->self());
      }
    });
  return found_ids;
}

TilePosition ExplorationManager::get_random_unexplored_base() const {
  std::vector<TilePosition> candidates;
  candidates.reserve(16);

  rnp::for_each_base(
    [this,&candidates](const BWEM::Base& base) {
      auto base_pos = TilePosition(base.Center());
      //if (rnp::map_manager()->is_under_enemy_influence(base_pos)) {}
      candidates.push_back(base_pos);
    });

  if (candidates.empty()) {
    return rnp::make_bad_position();
  }
  
  return rnp::Rand::choice(candidates);
}

void ExplorationManager::search_expansion_site() {
  if (rnp::is_valid_position(expansion_site_)) {
    if (Broodwar->getFrameCount() - site_set_frame_ > 500) {
      expansion_site_ = rnp::make_bad_position();
    }
  }

  if (not rnp::is_valid_position(expansion_site_)) {
    expansion_site_ = rnp::building_placer()->find_expansion_site();
    site_set_frame_ = Broodwar->getFrameCount();
  }
}

TilePosition ExplorationManager::get_expansion_site() const {
  return expansion_site_;
}

void ExplorationManager::set_explored(TilePosition pos) {
  for (auto& region_item : explore_) {
    if (region_item->location() == pos) {
      region_item->visited();
      return;
    }
  }

  explore_.insert(std::make_unique<RegionItem>(pos));
  Broodwar << "Explored " << pos << std::endl;
}

void ExplorationManager::set_expansion_site(TilePosition pos) {
  if (pos.x >= 0) {
    site_set_frame_ = Broodwar->getFrameCount();
    expansion_site_ = pos;
  }
}

TilePosition ExplorationManager::get_next_to_explore(Squad* squad) const {
  RNP_ASSERT(explore_.empty() == false);

  int longest_visit_frame = Broodwar->getFrameCount();
  TilePosition loc = rnp::make_bad_position();
  for (auto& region_item : explore_) {
    if (region_item->frame_visited() < longest_visit_frame) {
      longest_visit_frame = region_item->frame_visited();
      loc = region_item->location();
    }
  }

  if (not rnp::is_valid_map_position(loc)) {
    // TODO: It is time to attack but we know no enemy location
    loc = get_random_unexplored_base();
//    assert(loc.y < 129);
//    act::modify_actor<ExplorationManager>(
//      self(),
//      [loc](ExplorationManager* e) {
//        e->m_random_explore_.update(loc);
//      });
  }

  return loc;
}

void ExplorationManager::debug_print() const {
  //Uncomment this if you want to draw a mark at detected enemy buildings.
  /*for (int i = 0; i < (int)spottedBuildings.size(); i++)
  {
    if (spottedBuildings[i]->isActive())
    {
      int x1 = spottedBuildings[i]->getTilePosition().x * 32;
      int y1 = spottedBuildings[i]->getTilePosition().y * 32;
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
//    bool found = false;
//    for (auto& a : enemy_buildings_) {
//      if (a->get_unit_id() == unit->getID()) {
//        found = true;
//        break;
//      }
//    }
    bool found = std::any_of(enemy_buildings_.begin(),
                             enemy_buildings_.end(),
                             [unit](auto& b) {
                               return b->get_unit_id() == unit->getID();
                             });

    if (not found) {
      enemy_buildings_.insert(new SpottedObject(unit));
    }
  }
}

void ExplorationManager::on_unit_destroyed(Unit unit) {
  if (unit->getType().isBuilding()) {
    for (auto& a : enemy_buildings_) {
      if (a->is_at(unit->getTilePosition())) {
        enemy_buildings_.erase(a);
        return;
      }
    }
  }
}

void ExplorationManager::cleanup() {
  bool cont = true;

  while (cont) {
    cont = false;
    for (auto& a : enemy_buildings_) {
      if (Broodwar->isVisible(a->get_tile_position())) {
        bool found = false;
        for (auto& u : Broodwar->enemy()->getUnits()) {
          if (u->exists()) {
            if (u->getID() == a->get_unit_id()) {
              found = true;
              break;
            }
          }
        }
        if (not found) {
          enemy_buildings_.erase(a);
          cont = true;
          break;
        }
      }
    }
  }
}

int ExplorationManager::get_spotted_influence_in_region(const BWEM::Area* area) const {
  int im = 0;
  for (auto& spotted_object : enemy_buildings_) {
    if (rnp::is_inside(*area, spotted_object->get_position())) {
      im += spotted_object->get_type().buildScore();
    }
  }
  return im;
}

TilePosition ExplorationManager::get_closest_spotted_building(TilePosition start) const {
  auto pos = rnp::make_bad_position();
  auto best_dist = 100000.0;

  for (auto& a : enemy_buildings_) {
    auto c_dist = start.getDistance(a->get_tile_position());
    if (c_dist < best_dist) {
      best_dist = c_dist;
      pos = a->get_tile_position();
    }
  }

  return pos;
}

TilePosition ExplorationManager::get_random_spotted_building() const {
  if (enemy_buildings_.empty()) {
    rnp::log()->debug("No enemy buildings in cache");
    return rnp::make_bad_position();
  }

  std::vector<TilePosition> candidates;
  candidates.reserve(enemy_buildings_.size());

  for (auto& a : enemy_buildings_) {
    if (a->get_type().isBuilding()) {
      candidates.push_back(a->get_tile_position());
    }
  }

  return rnp::Rand::choice(candidates);
}

bool ExplorationManager::can_reach(TilePosition a, TilePosition b) {
  rnp::PathAB pathab(a, b);
  return pathab.is_good();
}

bool ExplorationManager::can_reach(BaseAgent* agent, TilePosition b) const {
  //return agent->get_unit()->hasPath(Position(b));
  return can_reach(agent->get_unit()->getTilePosition(), b);
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
