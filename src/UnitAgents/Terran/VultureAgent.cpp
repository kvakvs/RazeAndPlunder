#include "VultureAgent.h"

using namespace BWAPI;

bool VultureAgent::use_abilities() {
  //Mines
  if (unit_->getSpiderMineCount() > 0 && Broodwar->getFrameCount() - mine_drop_frame_ >= 100) {
    //Check if enemy units are visible
    for (auto& u : Broodwar->enemy()->getUnits()) {
      if (u->exists()) {
        if (unit_->getDistance(u) <= unit_->getType().sightRange()) {
          if (unit_->useTech(TechTypes::Spider_Mines, unit_->getPosition())) {
            mine_drop_frame_ = Broodwar->getFrameCount();
            Broodwar << "Spider Mine dropped" << std::endl;
            return true;
          }
        }
      }
    }
  }

  return false;
}
