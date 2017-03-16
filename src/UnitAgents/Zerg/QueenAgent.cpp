#include "QueenAgent.h"

using namespace BWAPI;

bool QueenAgent::useAbilities() {
  //Spawn Broodlings
  if (unit_->getEnergy() >= 150) {
    //Count enemy units and find an enemy organic unit
    int cntEnemy = 0;
    Unit enemyOrganic = nullptr;
    for (auto& u : Broodwar->enemy()->getUnits()) {
      if (u->exists()) {
        if (unit_->getDistance(u) <= 6 * 32 && u->getIrradiateTimer() == 0) {
          cntEnemy++;
          if (u->getType().isOrganic()) {
            enemyOrganic = u;
          }
        }
      }
    }

    if (cntEnemy >= 5 && enemyOrganic != nullptr) {
      if (unit_->useTech(TechTypes::Spawn_Broodlings, enemyOrganic)) {
        Broodwar << "Used Spawn Broodlings on " << enemyOrganic->getType().getName() << std::endl;
        return true;
      }
    }
  }

  return false;
}
