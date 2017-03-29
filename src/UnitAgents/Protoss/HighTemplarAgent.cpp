#include "HighTemplarAgent.h"
#include "../../Pathfinding/NavigationAgent.h"
#include "Managers/AgentManager.h"
#include "../../Commander/Commander.h"
#include "Glob.h"
#include "RnpUtil.h"

using namespace BWAPI;

bool HighTemplarAgent::use_abilities() {
  if (rnp::same_type(unit_, UnitTypes::Protoss_High_Templar)) {
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

    if (Broodwar->self()->hasResearched(hallucination)
      && unit_->getEnergy() >= 100) {
      //Check if enemy units are visible
      for (auto& u : Broodwar->enemy()->getUnits()) {
        if (u->exists()) {
          if (unit_->getDistance(u) <= unit_->getType().sightRange()) {
            auto target = find_hallucination_target();
            if (target) {
              if (unit_->useTech(hallucination, target->get_unit())) {
                Broodwar << "Uses Hallucination on " << target->unit_type().getName() << std::endl;
                return true;
              }
            }
          }
        }
      }
    }

    //Morph to Archon	
    if (not unit_->isBeingConstructed()) {
      auto sq = rnp::commander()->get_squad(squad_id_);
      if (sq) {
        if (sq->morphs_to().getID() == UnitTypes::Protoss_Archon.getID() 
          || unit_->getEnergy() < 50)
        {
          if (not any_enemy_units_visible() && not has_cast_transform_) {
            auto target = find_archon_target();
            if (target) {
              if (unit_->useTech(TechTypes::Archon_Warp, target->get_unit())) {
                has_cast_transform_ = true;
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


const BaseAgent* HighTemplarAgent::find_hallucination_target() const {
  //int maxRange = TechTypes::Hallucination.getWeapon().maxRange();

  const BaseAgent* result = nullptr;
  act::interruptible_for_each_actor<BaseAgent>(
      [&result](const BaseAgent* a) {
          if (a->is_of_type(UnitTypes::Protoss_Carrier)
              || a->is_of_type(UnitTypes::Protoss_Scout)
              || a->is_of_type(UnitTypes::Protoss_Archon)
              || a->is_of_type(UnitTypes::Protoss_Reaver)
              || a->get_unit()->isHallucination()) {
            return act::ForEach::Continue;
          }
          result = a;
          return act::ForEach::Break;
      });

  return result;
}

const BaseAgent* HighTemplarAgent::find_archon_target() const {
  auto m_squad = rnp::commander()->get_squad(squad_id_);
  const BaseAgent* result = nullptr;

  if (m_squad) {
    act::interruptible_for_each_in<BaseAgent>(
      m_squad->get_members(),
      [this,&result](const BaseAgent* a) {
        if (a->is_alive()
          && a->get_unit_id() != unit_id_
          && a->is_of_type(UnitTypes::Protoss_High_Templar) 
          && not a->get_unit()->isBeingConstructed()) 
        {
          double dist = a->get_unit()->getPosition().getDistance(unit_->getPosition());
          if (dist <= 64) {
            result = a;
            return act::ForEach::Break;
          }
        }
        return act::ForEach::Continue;
      });
  }

  return result;
}

int HighTemplarAgent::get_friendly_units_within_range(TilePosition tile_pos, int max_range) {
  int f_cnt = 0;
  act::for_each_actor<BaseAgent>(
    [&tile_pos,&max_range,&f_cnt](const BaseAgent* a) {
      if (a->is_unit()
        && not a->is_of_type(UnitTypes::Terran_Medic)) {
        double dist = a->get_unit()->getDistance(Position(tile_pos));
        if (dist <= max_range) {
          f_cnt++;
        }
      }
    });
  return f_cnt;
}
