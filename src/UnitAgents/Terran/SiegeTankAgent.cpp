#include "SiegeTankAgent.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

bool SiegeTankAgent::useAbilities() {
  //Siege Mode
  if (Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode)) {
    int range = UnitTypes::Terran_Siege_Tank_Siege_Mode.groundWeapon().maxRange();
    bool goSiege = false;

    //Check if enemy ground units are visible
    for (auto& u : Broodwar->enemy()->getUnits()) {
      if (u->exists() && not u->getType().isFlyer()) {
        if (unit_->getDistance(u) <= range) {
          goSiege = true;
        }
      }
    }

    //If we are defending and are at the defense position, go
    //in siege mode
    auto sq = rnp::commander()->getSquad(squad_id_);
    if (sq) {
      if (not sq->isActive()) {
        double d = unit_->getDistance(Position(goal_));
        if (d <= range * 0.5) goSiege = true;
      }
    }

    if (goSiege && not unit_->isSieged()) {
      unit_->siege();
      return true;
    }
    if (not goSiege && unit_->isSieged()) {
      unit_->unsiege();
      return true;
    }
  }

  return false;
}
