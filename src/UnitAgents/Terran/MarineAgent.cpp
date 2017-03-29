#include "MarineAgent.h"
#include "Managers/AgentManager.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

bool MarineAgent::use_abilities() {
  //Load into a Bunker
  if (not unit_->isLoaded()) {
    auto sq = rnp::commander()->get_squad(squad_id_);
    if (sq) {
      if (sq->is_bunker_defend_squad()) {
        auto result = false;
        act::for_each_actor<BaseAgent>(
          [this,&sq,&result](const BaseAgent* a) {
            if (a->is_of_type(UnitTypes::Terran_Bunker)
                && a->get_unit()->exists()) {
              if (a->get_unit()->getLoadedUnits().size() < 4) {
                unit_->rightClick(a->get_unit());
                act::modify_actor<Squad>(sq->self(),
                                         [a](Squad* sq) {
                                           sq->set_bunker_id(a->get_unit_id());
                                         });
                result = true;
                return;
              }
            }
          });
        if (result) { return true; }
      }
    }
  }

  //Use Stim Packs
  if (Broodwar->self()->hasResearched(TechTypes::Stim_Packs) 
      && not unit_->isStimmed() 
      && unit_->getHitPoints() >= 20 
      && not unit_->isLoaded())
  {
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
