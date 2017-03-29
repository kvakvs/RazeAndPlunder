#include "MutaliskAgent.h"
#include "Managers/AgentManager.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

bool MutaliskAgent::use_abilities() {
  //Check for morph
  if (rnp::agent_manager()->get_units_of_type_count(UnitTypes::Zerg_Greater_Spire) > 0) {
    auto sq = rnp::commander()->get_squad(squad_id_);
    if (sq) {
      if (sq->morphs_to().getID() == UnitTypes::Zerg_Devourer.getID()) {
        if (any_enemy_units_visible()) {
          if (Broodwar->canMake(UnitTypes::Zerg_Devourer, unit_)) {
            if (unit_->morph(UnitTypes::Zerg_Devourer)) {
              return true;
            }
          }
        }
      }

      if (sq->morphs_to().getID() == UnitTypes::Zerg_Guardian.getID()) {
        if (any_enemy_units_visible()) {
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
