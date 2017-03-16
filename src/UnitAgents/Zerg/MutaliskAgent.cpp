#include "MutaliskAgent.h"
#include "Managers/AgentManager.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

bool MutaliskAgent::useAbilities() {
  //Check for morph
  if (rnp::agent_manager()->countNoUnits(UnitTypes::Zerg_Greater_Spire) > 0) {
    auto sq = rnp::commander()->getSquad(squad_id_);
    if (sq) {
      if (sq->morphsTo().getID() == UnitTypes::Zerg_Devourer.getID()) {
        if (enemyUnitsVisible()) {
          if (Broodwar->canMake(UnitTypes::Zerg_Devourer, unit_)) {
            if (unit_->morph(UnitTypes::Zerg_Devourer)) {
              return true;
            }
          }
        }
      }

      if (sq->morphsTo().getID() == UnitTypes::Zerg_Guardian.getID()) {
        if (enemyUnitsVisible()) {
          if (Broodwar->canMake(UnitTypes::Zerg_Guardian, unit_)) {
            if (unit_->morph(UnitTypes::Zerg_Guardian)) {
              return true;
            }
          }
        }
      }
    } // if have squad
  } // if have spires

  return false;
}
