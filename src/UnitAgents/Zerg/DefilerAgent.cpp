#include "DefilerAgent.h"
#include "Managers/AgentManager.h"
#include "Glob.h"
#include "RnpUtil.h"

using namespace BWAPI;

bool DefilerAgent::use_abilities() {
  //Consume
  if (unit_->getEnergy() >= 125 
      && Broodwar->self()->hasResearched(TechTypes::Consume)) 
  {
    auto loop_result1 = act::interruptible_for_each_actor<BaseAgent>(
      [this](const BaseAgent* a) {
        if (a->is_alive() && a->is_of_type(UnitTypes::Zerg_Zergling)) {
          float dist = rnp::distance(a->get_unit()->getTilePosition(),
                                     unit_->getTilePosition());
          if (dist <= 2.0f) {
            if (unit_->useTech(TechTypes::Consume, a->get_unit())) {
              Broodwar << "Used Consume on " << a->unit_type().getName() << std::endl;
              return act::ForEach::Break;
            }
          }
        }
        return act::ForEach::Continue;
      });

    if (loop_result1 == act::ForEachResult::Interrupted) {
      return true;
    }
  }

  //Dark Swarm
  if (unit_->getEnergy() >= 100 
      && Broodwar->getFrameCount() - dark_swarm_frame_ > 100) 
  {
    auto loop_result2 = act::interruptible_for_each_actor<BaseAgent>(
      [this](const BaseAgent* a) {
        if (a->is_of_type(UnitTypes::Zerg_Mutalisk) && a->is_alive()) {
          if (a->is_under_attack()) {
            //A Mutalisk is in combat. Cover it in Dark Swarm.
            if (unit_->useTech(TechTypes::Dark_Swarm, a->get_unit()->getPosition())) {
              Broodwar << "Use Dark Swarm" << std::endl;
              dark_swarm_frame_ = Broodwar->getFrameCount();
              return act::ForEach::Break;
            }
          }
        }
        return act::ForEach::Continue;
      });

    if (loop_result2 == act::ForEachResult::Interrupted) {
      return true;
    }
  }

  return false;
}
