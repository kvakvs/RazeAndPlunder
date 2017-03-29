#include "BattlecruiserAgent.h"
#include "../../MainAgents/TargetingAgent.h"

using namespace BWAPI;

bool BattlecruiserAgent::use_abilities() {
  //To prevent order spamming
  last_order_frame_ = Broodwar->getFrameCount();

  if (Broodwar->getFrameCount() - lastUseFrame >= 80 && (unit_->isIdle() || unit_->isMoving())) {
    TechType gun = TechTypes::Yamato_Gun;
    if (Broodwar->self()->hasResearched(gun)) {
      if (unit_->getEnergy() >= gun.energyCost()) {
        int range = gun.getWeapon().maxRange();

        Unit target = TargetingAgent::find_highprio_target(this, range, true, true);
        if (target != nullptr) {
          Broodwar << "Yamato Gun used on " << target->getType().getName() << std::endl;
          unit_->useTech(gun, target);
          lastUseFrame = Broodwar->getFrameCount();
          return true;
        }
      }
    }
  }

  return false;
}
