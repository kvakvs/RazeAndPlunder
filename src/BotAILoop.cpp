#include "BotAILoop.h"
#include "Managers/BuildingPlacer.h"
#include "Utils/Profiler.h"
#include "Managers/Upgrader.h"
#include "Pathfinding/NavigationAgent.h"
#include "Managers/AgentManager.h"
#include "Influencemap/MapManager.h"
#include "Managers/ExplorationManager.h"
#include "Managers/Constructor.h"
#include "Commander/StrategySelector.h"
#include "Glob.h"

using namespace BWAPI;
using BWAPI::Broodwar;

BotAILoop::BotAILoop() : bwem_(BWEM::Map::Instance()) {
}

void BotAILoop::register_initial_units() {
  for (auto& u : Broodwar->self()->getUnits()) {
    rnp::agent_manager()->add_agent(u);
  }
}

BotAILoop::~BotAILoop() {

}

void BotAILoop::toggle_debug() {
  debug_ = not debug_;
}

void BotAILoop::toggle_unit_debug() {
  debug_units_ = not debug_units_;
}

void BotAILoop::toggle_potential_fields_debug() {
  debug_potential_fields_ = not debug_potential_fields_;
}

void BotAILoop::toggle_building_placement_debug() {
  debug_building_placement_ = not debug_building_placement_;
}

void BotAILoop::toggle_mapmanager_debug() {
  debug_mapmanager_ = not debug_mapmanager_;
}

void BotAILoop::set_debug_sq(int squadID) {
  // Craft the squad actor id
  debug_squad_id_ = act::ActorId(ActorFlavour::Squad, squadID);
}

void BotAILoop::on_frame() {
  rnp::profiler()->start("OnFrame_MapManager");
  rnp::map_manager()->update_influences();
  rnp::profiler()->end("OnFrame_MapManager");

  auto& sched = act::sched();
  sched.hard_sync_now(static_cast<size_t>(Broodwar->getFrameCount()));
  sched.tick();
}

void BotAILoop::on_unit_added(Unit unit) {
  rnp::agent_manager()->add_agent(unit);

  //Remove from buildorder if this is a building
  if (unit->getType().isBuilding()) {
    UnitType ut = unit->getType();
    Constructor::modify([=](Constructor* c) { c->unlock(ut); });
  }
}

void BotAILoop::on_unit_morphed(Unit unit) {
  //rnp::agent_manager()->on_drone_morphed(unit);
  msg::agentmanager::unit_destroyed(unit);
  UnitType ut = unit->getType();
  Constructor::modify([=](Constructor* c) { c->unlock(ut); });
}

void BotAILoop::on_unit_destroyed(Unit unit) {
  if (rnp::is_my_unit(unit)) {
    //Remove bunker squads if the destroyed unit is a bunker
    if (unit->getType().getID() == UnitTypes::Terran_Bunker.getID()) {
      msg::commander::bunker_destroyed(unit->getID());
      //rnp::commander()->remove_bunker_squad(unit->getID());
    }

    msg::agentmanager::unit_destroyed(unit);

    if (unit->getType().isBuilding()) {
      Constructor::modify([unit](Constructor* c) {
          c->on_building_destroyed(unit);
        });
    }
  }

  if (unit->getPlayer()->getID() != Broodwar->self()->getID() 
    && not unit->getPlayer()->isNeutral()) {
    //Update spotted buildings
    ExplorationManager::modify(
      [unit](ExplorationManager* e) { e->on_unit_destroyed(unit); });
  }
}

void BotAILoop::show_debug() {
  if (!debug_) return;

  //Show timer
  std::stringstream ss;
  ss << "\x0FTime: ";
  ss << Broodwar->elapsedTime() / 60;
  ss << ":";
  int sec = Broodwar->elapsedTime() % 60;
  if (sec < 10) ss << "0";
  ss << sec;

  Broodwar->drawTextScreen(110, 5, ss.str().c_str());
  //

  //Show pathfinder version
  std::stringstream st;
  st << "\x0FPathfinder: ";
  switch (NavigationAgent::pathfinding_version_) {
  case NavigationAgent::PFType::Builtin :
    st << "Built-in";
    break;
  case NavigationAgent::PFType::HybridBoids :
    st << "Hybrid Boids";
    break;
  case NavigationAgent::PFType::HybridPotentialField :
    st << "Hybrid PF";
    break;
  }

  Broodwar->drawTextScreen(500, 310, st.str().c_str());

  rnp::strategy_selector()->print_info();

  if (debug_building_placement_) {
    rnp::building_placer()->show_debug();
  }
  draw_terrain_data();

  rnp::commander()->debug_show_goal();

  act::for_each_actor<BaseAgent>(
    [](const BaseAgent* a) {
      if (a->is_building()) a->debug_show_goal();
    });

  //Show goal info for selected units
  auto& selected = Broodwar->getSelectedUnits();
  if (not selected.empty()) {
    for (auto& u : selected) {
      int unitID = (u)->getID();
      auto agent = rnp::agent_manager()->get_agent(unitID);
      if (agent && agent->is_alive()) {
        agent->debug_show_goal();
      }
    }
  }

  if (debug_building_placement_) {
    //If we have any unit selected, use that to show PFs.
    if (not selected.empty()) {
      for (auto& u : selected) {
        int unitID = u->getID();
        auto agent = rnp::agent_manager()->get_agent(unitID);
        if (agent) {
          rnp::navigation()->debug_display_pf(agent);
        }
        break;
      }
    }
  }

  if (debug_units_) {
    //If we have any unit selected, show unit info.
    if (not selected.empty()) {
      for (auto& u : selected) {
        int unit_id = u->getID();
        auto agent = rnp::agent_manager()->get_agent(unit_id);
        if (agent) {
          auto pos = agent->get_unit()->getPosition();
          Broodwar->drawTextMap(pos.x, pos.y, "id:%d", unit_id);
          agent->debug_print_info();
          break;
        }
      }
    }
  }

  if (act::whereis<Squad>(debug_squad_id_)) {
    auto squad = rnp::commander()->get_squad(debug_squad_id_);
    if (squad) {
      squad->debug_print_info();
    }
  }

  if (debug_mapmanager_) {
    rnp::map_manager()->debug_print_info();
  }

  rnp::upgrader()->debug_print_info();
  rnp::commander()->debug_print_info();
}

void BotAILoop::draw_terrain_data() {
  // we will iterate through all the base locations, and draw their outlines.
  rnp::for_each_base(
    [](const BWEM::Base& base) {
//      TilePosition p(base.Location());
//      Position c(base.Center());

      //Draw a progress bar at each resource
      for (auto& u : Broodwar->getStaticMinerals()) {
        if (u->getResources() > 0) {

          int total = u->getInitialResources();
          int done = u->getResources();

          int w = 60;
          int h = 64;

          //Start 
          Position s(u->getPosition().x - w / 2 + 2, u->getPosition().y - 4);
          //End
          Position e(s.x + w, s.y + 8);
          //Progress
          int prg = static_cast<int>(
            static_cast<float>(done) / static_cast<float>(total) * w
            );
          Position p(s.x + prg, s.y + 8);

          Broodwar->drawBoxMap(s.x, s.y, e.x, e.y, Colors::Orange, false);
          Broodwar->drawBoxMap(s.x, s.y, p.x, p.y, Colors::Orange, true);
        }
      } // for static minerals
    });

  if (debug_building_placement_) {
    //we will iterate through all the regions and draw the polygon outline of it in white.
//    for (BWEM::Area* r : BWTA::getRegions()) {
//      auto p = BWTA::PolygonImpl(r->getPolygon());
//      for (int j = 0; j < (int)p.size(); j++) {
//        Position point1 = p[j];
//        Position point2 = p[(j + 1) % p.size()];
//        Broodwar->drawLineMap(point1.x, point1.y, point2.x, point2.y, Colors::Orange);
//      }
//    }

    //we will visualize the chokepoints with yellow lines
//    for (BWEM::Area* r : BWTA::getRegions()) {
//      for (BWEM::ChokePoint* c : r->getChokepoints()) {
//        Position point1 = c->getSides().first;
//        Position point2 = c->getSides().second;
//        Broodwar->drawLineMap(point1.x, point1.y, point2.x, point2.y, Colors::Yellow);
//      }
//    }
  }

  //locate zerg eggs and draw progress bars
  if (Constructor::is_zerg()) {
    for (auto& u : Broodwar->getAllUnits()) {
      if (u->getType().getID() == UnitTypes::Zerg_Egg.getID() 
          || u->getType().getID() == UnitTypes::Zerg_Lurker_Egg.getID()
          || u->getType().getID() == UnitTypes::Zerg_Cocoon.getID()) {
        int total = u->getBuildType().buildTime();
        int done = total - u->getRemainingBuildTime();

        int w = u->getType().tileWidth() * 32;
        int h = u->getType().tileHeight() * 32;

        //Start 
        Position s = Position(u->getPosition().x - w / 2, u->getPosition().y - 4);
        //End
        Position e = Position(s.x + w, s.y + 8);

        //Progress
        int prg = static_cast<int>(
          static_cast<float>(done) / static_cast<float>(total) * w
          );
        Position p = Position(s.x + prg, s.y + 8);

        Broodwar->drawBoxMap(s.x, s.y, e.x, e.y, Colors::Blue, false);
        Broodwar->drawBoxMap(s.x, s.y, p.x, p.y, Colors::Blue, true);
      }
    }
  }
}
