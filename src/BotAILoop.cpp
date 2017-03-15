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

BotAILoop::BotAILoop() : bwem_(BWEM::Map::Instance()) {
  debug_unit_ = false;
  debug_pf_ = false;
  debug_bp_ = false;
  debug_sq_ = -1;
  debug_ = true;
}

void BotAILoop::register_initial_units() {
  for (auto& u : Broodwar->self()->getUnits()) {
    rnp::agent_manager()->addAgent(u);
  }
}

BotAILoop::~BotAILoop() {

}

void BotAILoop::toggleDebug() {
  debug_ = !debug_;
}

void BotAILoop::toggleUnitDebug() {
  debug_unit_ = !debug_unit_;
}

void BotAILoop::togglePFDebug() {
  debug_pf_ = !debug_pf_;
}

void BotAILoop::toggleBPDebug() {
  debug_bp_ = !debug_bp_;
}

void BotAILoop::setDebugSQ(int squadID) {
  debug_sq_ = squadID;
}

void BotAILoop::computeActions() {
  rnp::profiler()->start("OnFrame_MapManager");
  rnp::map_manager()->update();
  rnp::profiler()->end("OnFrame_MapManager");
  rnp::profiler()->start("OnFrame_Constructor");
  rnp::constructor()->computeActions();
  rnp::profiler()->end("OnFrame_Constructor");
  rnp::profiler()->start("OnFrame_Commander");
  rnp::commander()->computeActions();
  rnp::profiler()->end("OnFrame_Commander");
  rnp::profiler()->start("OnFrame_ExplorationManager");
  rnp::exploration()->computeActions();
  rnp::profiler()->end("OnFrame_ExplorationManager");
  rnp::profiler()->start("OnFrame_AgentManager");
  rnp::agent_manager()->computeActions();
  rnp::profiler()->end("OnFrame_AgentManager");
}

void BotAILoop::addUnit(Unit unit) {
  rnp::agent_manager()->addAgent(unit);

  //Remove from buildorder if this is a building
  if (unit->getType().isBuilding()) {
    rnp::constructor()->unlock(unit->getType());
  }
}

void BotAILoop::morphUnit(Unit unit) {
  rnp::agent_manager()->morphDrone(unit);
  rnp::constructor()->unlock(unit->getType());
}

void BotAILoop::unitDestroyed(Unit unit) {
  if (unit->getPlayer()->getID() == Broodwar->self()->getID()) {
    //Remove bunker squads if the destroyed unit
    //is a bunker
    if (unit->getType().getID() == UnitTypes::Terran_Bunker.getID()) {
      rnp::commander()->removeBunkerSquad(unit->getID());
    }

    rnp::agent_manager()->removeAgent(unit);
    if (unit->getType().isBuilding()) {
      rnp::constructor()->buildingDestroyed(unit);
    }

    rnp::agent_manager()->cleanup();
  }
  if (unit->getPlayer()->getID() != Broodwar->self()->getID() && !unit->getPlayer()->isNeutral()) {
    //Update spotted buildings
    rnp::exploration()->unitDestroyed(unit);
  }
}

void BotAILoop::show_debug() {
  if (debug_) {
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
    if (NavigationAgent::pathfinding_version_ == 0) {
      st << "Built-in";
    }
    if (NavigationAgent::pathfinding_version_ == 1) {
      st << "Hybrid Boids";
    }
    if (NavigationAgent::pathfinding_version_ == 2) {
      st << "Hybrid PF";
    }

    Broodwar->drawTextScreen(500, 310, st.str().c_str());
    //

    rnp::strategy_selector()->printInfo();

    if (debug_bp_) {
      rnp::building_placer()->debug();
    }
    drawTerrainData();

    rnp::commander()->debug_showGoal();

    auto& agents = rnp::agent_manager()->getAgents();
    for (auto& a : agents) {
      if (a->isBuilding()) a->debug_showGoal();
    }

    //Show goal info for selected units
    auto& selected = Broodwar->getSelectedUnits();
    if (not selected.empty()) {
      for (auto& u : selected) {
        int unitID = (u)->getID();
        BaseAgent* agent = rnp::agent_manager()->getAgent(unitID);
        if (agent != nullptr && agent->isAlive()) {
          agent->debug_showGoal();
        }
      }
    }

    if (debug_bp_) {
      //If we have any unit selected, use that to show PFs.
      if (not selected.empty()) {
        for (auto& u : selected) {
          int unitID = u->getID();
          BaseAgent* agent = rnp::agent_manager()->getAgent(unitID);
          if (agent != nullptr) {
            rnp::navigation()->displayPF(agent);
          }
          break;
        }
      }
    }

    if (debug_unit_) {
      //If we have any unit selected, show unit info.
      if (not selected.empty()) {
        for (auto& u : selected) {
          int unitID = u->getID();
          BaseAgent* agent = rnp::agent_manager()->getAgent(unitID);
          if (agent != nullptr) {
            agent->printInfo();
            break;
          }
        }
      }
    }

    if (debug_sq_ >= 0) {
      auto squad = rnp::commander()->getSquad(debug_sq_);
      if (squad) {
        squad->printInfo();
      }
    }

    rnp::upgrader()->printInfo();
    rnp::commander()->printInfo();
  }
}

void BotAILoop::drawTerrainData() {
  // we will iterate through all the base locations, and draw their outlines.
  for (auto& area : bwem_.Areas()) {
    for (auto& base : area.Bases()) {
      TilePosition p = base.Location();
      Position c = base.Center();

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
          int prg = (int)((double)done / (double)total * w);
          Position p(s.x + prg, s.y + 8);

          Broodwar->drawBoxMap(s.x, s.y, e.x, e.y, Colors::Orange, false);
          Broodwar->drawBoxMap(s.x, s.y, p.x, p.y, Colors::Orange, true);
        }
      } // for static minerals
    } // for bases
  } // for areas

  if (debug_bp_) {
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
  if (Constructor::isZerg()) {
    for (auto& u : Broodwar->getAllUnits()) {
      if (u->getType().getID() == UnitTypes::Zerg_Egg.getID() || u->getType().getID() == UnitTypes::Zerg_Lurker_Egg.getID() || u->getType().getID() == UnitTypes::Zerg_Cocoon.getID()) {
        int total = u->getBuildType().buildTime();
        int done = total - u->getRemainingBuildTime();

        int w = u->getType().tileWidth() * 32;
        int h = u->getType().tileHeight() * 32;

        //Start 
        Position s = Position(u->getPosition().x - w / 2, u->getPosition().y - 4);
        //End
        Position e = Position(s.x + w, s.y + 8);
        //Progress
        int prg = (int)((double)done / (double)total * w);
        Position p = Position(s.x + prg, s.y + 8);

        Broodwar->drawBoxMap(s.x, s.y, e.x, e.y, Colors::Blue, false);
        Broodwar->drawBoxMap(s.x, s.y, p.x, p.y, Colors::Blue, true);
      }
    }
  }
}
