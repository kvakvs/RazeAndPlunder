#include "HighTemplarAgent.h"
#include "../../Pathfinding/NavigationAgent.h"
#include "Managers/AgentManager.h"
#include "../../Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

bool HighTemplarAgent::useAbilities() {
  if (isOfType(unit_->getType(), UnitTypes::Protoss_High_Templar)) {
    //Not transformed to Archon. Use spells.

    //PSI storm
    TechType psiStorm = TechTypes::Psionic_Storm;
    if (Broodwar->self()->hasResearched(psiStorm) && unit_->getEnergy() >= 75) {
      int range = TechTypes::Psionic_Storm.getWeapon().maxRange();
      //Check if enemy units are visible
      for (auto& u : Broodwar->enemy()->getUnits()) {
        if (u->exists()
          && (unit_->getDistance(u) <= range && not u->isUnderStorm())
          && unit_->useTech(psiStorm, u->getPosition()))
        {
          Broodwar << "Psionic Storm used on " << u->getType().getName() << std::endl;
          return true;
        }
      }
    }

    //Hallucination
    TechType hallucination = TechTypes::Hallucination;
    if (Broodwar->self()->hasResearched(hallucination) && unit_->getEnergy() >= 100) {
      //Check if enemy units are visible
      for (auto& u : Broodwar->enemy()->getUnits()) {
        if (u->exists()) {
          if (unit_->getDistance(u) <= unit_->getType().sightRange()) {
            BaseAgent* target = findHallucinationTarget();
            if (target != nullptr) {
              if (unit_->useTech(hallucination, target->getUnit())) {
                Broodwar << "Uses Hallucination on " << target->getUnitType().getName() << std::endl;
                return true;
              }
            }
          }
        }
      }
    }

    //Morph to Archon	
    if (not  unit_->isBeingConstructed()) {
      auto sq = rnp::commander()->getSquad(squad_id_);
      if (sq) {
        if (sq->morphsTo().getID() == UnitTypes::Protoss_Archon.getID() 
          || unit_->getEnergy() < 50)
        {
          if (not enemyUnitsVisible() && not hasCastTransform) {
            BaseAgent* target = findArchonTarget();
            if (target) {
              if (unit_->useTech(TechTypes::Archon_Warp, target->getUnit())) {
                hasCastTransform = true;
                return true;
              }
            }
          }
        }
      } // if sq
    } // if not constructed
  }

  return false;
}


BaseAgent* HighTemplarAgent::findHallucinationTarget() {
  int maxRange = TechTypes::Hallucination.getWeapon().maxRange();

  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    bool targetUnit = false;
    if (a->isOfType(UnitTypes::Protoss_Carrier)) targetUnit = true;
    if (a->isOfType(UnitTypes::Protoss_Scout)) targetUnit = true;
    if (a->isOfType(UnitTypes::Protoss_Archon)) targetUnit = true;
    if (a->isOfType(UnitTypes::Protoss_Reaver)) targetUnit = true;

    if (a->isAlive() && targetUnit) {
      if (not a->getUnit()->isHallucination()) {
        return a;
      }
    }
  }

  return nullptr;
}

BaseAgent* HighTemplarAgent::findArchonTarget() {
  auto mSquad = rnp::commander()->getSquad(squad_id_);
  if (mSquad) {
    Agentset agents = mSquad->getMembers();
    for (auto& a : agents) {
      if (a->isAlive() && a->getUnitID() != unit_id_ && a->isOfType(UnitTypes::Protoss_High_Templar) && not a->getUnit()->isBeingConstructed()) {
        double dist = a->getUnit()->getPosition().getDistance(unit_->getPosition());
        if (dist <= 64) {
          return a;
        }
      }
    }
  }

  return nullptr;
}

int HighTemplarAgent::friendlyUnitsWithinRange(TilePosition tilePos, int maxRange) {
  int fCnt = 0;
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isUnit() && not a->isOfType(UnitTypes::Terran_Medic)) {
      double dist = a->getUnit()->getDistance(Position(tilePos));
      if (dist <= maxRange) {
        fCnt++;
      }
    }
  }
  return fCnt;
}
