#include "HydraliskAgent.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

bool HydraliskAgent::use_abilities() {
  if (Broodwar->self()->hasResearched(TechTypes::Lurker_Aspect)) {
    auto sq = rnp::commander()->get_squad(squad_id_);
    if (sq != nullptr
      && sq->morphs_to().getID() == UnitTypes::Zerg_Lurker.getID()
      && not any_enemy_units_visible()
      && Broodwar->canMake(UnitTypes::Zerg_Lurker, unit_)
      && unit_->morph(UnitTypes::Zerg_Lurker)) {
      return true;
    }
  }

  return false;
}
