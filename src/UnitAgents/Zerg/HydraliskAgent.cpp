#include "HydraliskAgent.h"
#include "../../Commander/Commander.h"

bool HydraliskAgent::useAbilities() {
  if (Broodwar->self()->hasResearched(TechTypes::Lurker_Aspect)) {
    Squad* sq = Commander::getInstance()->getSquad(squadID);
    if (sq != nullptr
      && sq->morphsTo().getID() == UnitTypes::Zerg_Lurker.getID()
      && !enemyUnitsVisible()
      && Broodwar->canMake(UnitTypes::Zerg_Lurker, unit)
      && unit->morph(UnitTypes::Zerg_Lurker)) {
      return true;
    }
  }

  return false;
}
