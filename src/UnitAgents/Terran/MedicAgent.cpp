#include "MedicAgent.h"
#include "Managers/AgentManager.h"
#include <iso646.h>
#include "Glob.h"

using namespace BWAPI;

bool MedicAgent::use_abilities() {
  //Check heal
  double best_dist = -1;
  Unit to_heal = nullptr;

  act::for_each_actor<BaseAgent>(
    [this,&best_dist,&to_heal](const BaseAgent* a) {
      if (a->is_damaged()) {
        if (is_medic_target(a->get_unit()) 
          && a->get_unit_id() != unit_->getID()) 
        {
          auto c_unit = a->get_unit();
          if (c_unit->exists() && c_unit->getHitPoints() > 0) {
            double dist = unit_->getDistance(c_unit);
            if (best_dist < 0 || dist < best_dist) {
              best_dist = dist;
              to_heal = c_unit;
            }
          }
        }
      }
    });

  if (best_dist >= 0 && to_heal != nullptr) {
    unit_->useTech(TechTypes::Healing, to_heal);
    return true;
  }

  return false;
}

bool MedicAgent::is_medic_target(Unit mUnit) {
  if (not mUnit->getType().isOrganic()) {
    //Can only heal organic units
    return false;
  }

  if (mUnit->getType().isWorker()) {
    //We can heal workers, but no point
    //in following them
    return false;
  }

  if (not mUnit->getType().canAttack()) {
    //Dont follow units that cant attack
    return false;
  }

  if (mUnit->getType().getID() == UnitTypes::Terran_Medic.getID()) {
    //Dont follow other medics
    return false;
  }

  if (mUnit->isLoaded()) {
    //Dont "follow" bunkered units
    return false;
  }

  return true;
}
