#include "Upgrader.h"
#include "AgentManager.h"
#include "ResourceManager.h"
#include "Glob.h"

#include <sstream>
#include <iso646.h>

using namespace BWAPI;

Upgrader::Upgrader() {
  debug_ = false;
}

Upgrader::~Upgrader() {
}

void Upgrader::toggle_debug() {
  debug_ = !debug_;
}

std::string Upgrader::format(const std::string& str) const {
  std::string res = str;

  auto& raceName = Broodwar->self()->getRace().getName();
  if (str.find(raceName) == 0) {
    int i = str.find("_");
    res = str.substr(i + 1, str.length());
  }

  return res;
}

void Upgrader::debug_print_info() const {
  if (not debug_) return;

  //Precalc total lines
  int totalLines = 1;
  for (auto& u : TechTypes::allTechTypes()) {
    if (u.getRace().getID() == Broodwar->self()->getRace().getID() && Broodwar->self()->hasResearched(u) && u.mineralPrice() > 0) {
      totalLines++;
    }
  }
  for (auto& u : UpgradeTypes::allUpgradeTypes()) {
    if (u.getRace().getID() == Broodwar->self()->getRace().getID() && u.mineralPrice() > 0) {
      int cLevel = Broodwar->self()->getUpgradeLevel(u);
      if (cLevel > 0) {
        totalLines++;
      }
    }
  }
  if (totalLines == 1) totalLines++;
  Broodwar->drawBoxScreen(3, 25, 162, 26 + totalLines * 16, Colors::Black, true);
  //

  int line = 1;
  for (auto& u : TechTypes::allTechTypes()) {
    if (u.getRace().getID() == Broodwar->self()->getRace().getID() && Broodwar->self()->hasResearched(u) && u.mineralPrice() > 0) {
      Broodwar->drawTextScreen(5, 25 + 16 * line, u.getName().c_str());
      line++;
    }
  }
  for (auto& u : UpgradeTypes::allUpgradeTypes()) {
    if (u.getRace().getID() == Broodwar->self()->getRace().getID() && u.mineralPrice() > 0) {
      int cLevel = Broodwar->self()->getUpgradeLevel(u);
      if (cLevel > 0) {
        std::stringstream ss;
        ss << format(u.getName());
        ss << " \x0F(";
        ss << cLevel;
        ss << "/";
        ss << u.maxRepeats();
        ss << ")";

        Broodwar->drawTextScreen(5, 25 + 16 * line, ss.str().c_str());
        line++;
      }
    }
  }

  Broodwar->drawTextScreen(5, 25, "\x03Techs/upgrades:");
  Broodwar->drawLineScreen(5, 39, 160, 39, Colors::Orange);
  if (line == 1) line++;
  Broodwar->drawLineScreen(5, 25 + line * 16, 160, 25 + line * 16, Colors::Orange);
}


bool Upgrader::check_upgrade(BaseAgent* agent) {
  if (agent->is_alive() && agent->get_unit()->isIdle()) {
    Unit unit = agent->get_unit();

    //Check techs
    for (int i = 0; i < (int)techs_.size(); i++) {
      TechType type = techs_[i];
      if (Broodwar->self()->hasResearched(type)) {
        techs_.erase(techs_.begin() + i);
        return true;
      }
      if (can_research(type, unit)) {
        unit->research(type);
        return true;
      }
    }

    //Check upgrades
    for (int i = 0; i < (int)upgrades_.size(); i++) {
      UpgradeType type = upgrades_[i];
      if (can_upgrade(type, unit)) {
        if (unit->upgrade(type)) {
          upgrades_.erase(upgrades_.begin() + i);
          return true;
        }
      }
    }
  }

  return false;
}

bool Upgrader::can_upgrade(UpgradeType type, Unit unit) {
  //1. Check if unit is idle
  if (not unit->isIdle()) {
    return false;
  }

  //2. Check if unit can do this upgrade
  if (not Broodwar->canUpgrade(type, unit)) {
    return false;
  }

  //3. Check if we have enough resources
  if (not rnp::resources()->hasResources(type)) {
    return false;
  }

  //4. Check if unit is being constructed
  if (unit->isBeingConstructed()) {
    return false;
  }

  //5. Check if some other building is already doing this upgrade
  auto loop_result = act::interruptible_for_each_actor<BaseAgent>(
      [&type](const BaseAgent* a) -> act::ForEach {
          if (a->get_unit()->getUpgrade().getID() == type.getID()) {
            return act::ForEach::Break;
          }
          return act::ForEach::Continue;
      });
  if (loop_result == act::ForEachResult::Interrupted) { return false; }

  //6. Check if we are currently upgrading it
  if (Broodwar->self()->isUpgrading(type)) {
    return false;
  }

  //All clear. Can do the upgrade.
  return true;
}

bool Upgrader::can_research(TechType type, Unit unit) const {
  //Seems Broodwar->canResearch bugs when Lurker Aspect is requested without
  //having an upgraded Lair.
  if (type.getID() == TechTypes::Lurker_Aspect.getID()) {
    if (rnp::agent_manager()->get_finished_units_count(UnitTypes::Zerg_Lair) == 0) return false;
  }

  //1. Check if unit can do this upgrade
  if (not Broodwar->canResearch(type, unit)) {
    return false;
  }

  //2. Check if we have enough resources
  if (not rnp::resources()->hasResources(type)) {
    return false;
  }

  //3. Check if unit is idle
  if (not unit->isIdle()) {
    return false;
  }

  //4. Check if unit is being constructed
  if (unit->isBeingConstructed()) {
    return false;
  }

  //5. Check if some other building is already doing this upgrade
  auto loop_result = act::interruptible_for_each_actor<BaseAgent>(
      [&type](const BaseAgent* a) {
          if (a->get_unit()->getTech().getID() == type.getID()) {
            return act::ForEach::Break;
          }
          return act::ForEach::Continue;
      });
  if (loop_result == act::ForEachResult::Interrupted) { return false; }

  //6. Check if we are currently researching it
  if (Broodwar->self()->isResearching(type)) {
    return false;
  }

  //All clear. Can do the research.
  return true;
}

void Upgrader::add_upgrade(UpgradeType type) {
  upgrades_.push_back(type);
}

void Upgrader::add_tech(TechType type) {
  techs_.push_back(type);
}
