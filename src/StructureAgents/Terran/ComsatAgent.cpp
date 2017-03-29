#include "ComsatAgent.h"
#include "Managers/AgentManager.h"
#include "Influencemap/MapManager.h"
#include "Glob.h"

using namespace BWAPI;

ComsatAgent::ComsatAgent(Unit mUnit)
    : StructureAgent(mUnit), last_sweep_pos_(-1, -1)
{
  unit_id_ = unit_->getID();
  agent_type_ = "ComsatAgent";
}

void ComsatAgent::tick() {
  if (not unit_->isIdle()) return;

  if (Broodwar->getFrameCount() - last_sweep_frame_ > 100 && unit_->getEnergy() >= 50) {
    for (auto& u : Broodwar->enemy()->getUnits()) {
      //Enemy seen
      if (u->exists()) {
        if ((u->isCloaked() || u->isBurrowed())
            && not u->isDetected()
            && u->getType().getID() != UnitTypes::Protoss_Observer.getID()) {
          if (friendlyUnitsWithinRange(u->getPosition()) > 0 && not anyHasSweeped(u->getTilePosition())) {
            Broodwar << "Use Scanner Sweep at ("
                << u->getTilePosition().x << "," << u->getTilePosition().y
                << ") " << u->getType().getName() << " detected" 
                << std::endl;
            bool ok = unit_->useTech(TechTypes::Scanner_Sweep, u->getPosition());
            if (ok) {
              last_sweep_frame_ = Broodwar->getFrameCount();
              last_sweep_pos_ = u->getTilePosition();
              return;
            }
          }
        }
      }
    }

    //Uncomment if you want the Comsat to scan for enemy location.
    /*if (rnp::commander()->isAttacking())
    {
      TilePosition pos = rnp::map_manager()->findAttackPosition();
      if (not rnp::is_valid_position(pos))
      {
        //No attack position found. Sweep a base area
        for (BWTA::BaseLocation* r : BWTA::getBaseLocations())
        {
          if (not anyHasSweeped(r->getTilePosition()) && not Broodwar->isVisible(r->getTilePosition()))
          {
            bool ok = unit->useTech(TechTypes::Scanner_Sweep, r->getPosition());
            if (ok)
            {
              Broodwar << "Use Scanner Sweep to find enemy at (" << r->getTilePosition().x << "," << r->getTilePosition().y << ")" << endl;
              lastSweepFrame = Broodwar->getFrameCount();
              lastSweepPos = r->getTilePosition();
              return;
            }
          }
        }
      }
    }*/
  }
}

int ComsatAgent::friendlyUnitsWithinRange(Position pos) {
  int f_cnt = 0;
  double max_range = 384; //Range of sieged tanks
  act::for_each_actor<BaseAgent>(
    [this,&pos,&max_range,&f_cnt](const BaseAgent* a) {
      if (a->is_unit() && a->unit_type().canAttack()) {
        auto dist = a->get_unit()->getPosition().getDistance(pos);
        if (dist <= max_range) {
          f_cnt++;
        }
      }
    });
  return f_cnt;
}

bool ComsatAgent::anyHasSweeped(TilePosition pos) {
  auto loop_result = act::interruptible_for_each_actor<BaseAgent>(
      [&pos](const BaseAgent* a) {
          auto a_id = a->unit_type().getID();
          if (a_id == UnitTypes::Terran_Comsat_Station.getID()) {
            auto ca = static_cast<const ComsatAgent*>(a);
            if (ca->hasSweeped(pos)) {
              return act::ForEach::Break;
            }
          }
          return act::ForEach::Continue;
      });
  // Early loop interrupted means we've found a swept position
  return loop_result == act::ForEachResult::Interrupted;
}

bool ComsatAgent::hasSweeped(TilePosition pos) const {
  if (Broodwar->getFrameCount() - last_sweep_frame_ > 100) {
    return false;
  }

  return pos.getApproxDistance(last_sweep_pos_) <= 5;
}
