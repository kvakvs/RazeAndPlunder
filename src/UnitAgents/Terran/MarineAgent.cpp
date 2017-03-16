#include "MarineAgent.h"
#include "Managers/AgentManager.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

bool MarineAgent::useAbilities() {
  //Load into a Bunker
  if (not unit_->isLoaded()) {
    auto sq = rnp::commander()->getSquad(squad_id_);
    if (sq) {
      if (sq->isBunkerDefend()) {
        auto& agents = rnp::agent_manager()->getAgents();
        for (auto& a : agents) {
          if (a->isAlive() && a->isOfType(UnitTypes::Terran_Bunker) && a->getUnit()->exists()) {
            if (a->getUnit()->getLoadedUnits().size() < 4) {
              unit_->rightClick(a->getUnit());
              sq->setBunkerID(a->getUnitID());
              return true;
            }
          }
        }
      }
    }
  }

  //Use Stim Packs
  if (Broodwar->self()->hasResearched(TechTypes::Stim_Packs) && not unit_->isStimmed() && unit_->getHitPoints() >= 20 && not unit_->isLoaded()) {
    //Check if enemy units are visible
    for (auto& u : Broodwar->enemy()->getUnits()) {
      if (u->exists()) {
        if (unit_->getDistance(u) <= unit_->getType().sightRange()) {
          unit_->useTech(TechTypes::Stim_Packs);
          return true;
        }
      }
    }
  }

  return false;
}
