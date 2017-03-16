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

void ComsatAgent::computeActions() {
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
  int fCnt = 0;
  double maxRange = 384; //Range of sieged tanks
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isUnit() && a->isAlive() && a->getUnitType().canAttack()) {
      double dist = a->getUnit()->getPosition().getDistance(pos);
      if (dist <= maxRange) {
        fCnt++;
      }
    }
  }
  return fCnt;
}

bool ComsatAgent::anyHasSweeped(TilePosition pos) {
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive() && a->getUnitType().getID() == UnitTypes::Terran_Comsat_Station.getID()) {
      ComsatAgent* ca = (ComsatAgent*)a;
      if (ca->hasSweeped(pos)) {
        return true;
      }
    }
  }
  return false;
}

bool ComsatAgent::hasSweeped(TilePosition pos) {
  if (Broodwar->getFrameCount() - last_sweep_frame_ > 100) {
    return false;
  }

  if (pos.getDistance(last_sweep_pos_) <= 5) {
    return true;
  }

  return false;
}
