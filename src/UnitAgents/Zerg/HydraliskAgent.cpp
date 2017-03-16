#include "HydraliskAgent.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

bool HydraliskAgent::useAbilities() {
  if (Broodwar->self()->hasResearched(TechTypes::Lurker_Aspect)) {
    auto sq = rnp::commander()->getSquad(squad_id_);
    if (sq != nullptr
      && sq->morphsTo().getID() == UnitTypes::Zerg_Lurker.getID()
      && not enemyUnitsVisible()
      && Broodwar->canMake(UnitTypes::Zerg_Lurker, unit_)
      && unit_->morph(UnitTypes::Zerg_Lurker)) {
      return true;
    }
  }

  return false;
}
