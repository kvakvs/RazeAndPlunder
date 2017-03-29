#include "ScienceVesselAgent.h"
#include "Managers/AgentManager.h"
#include "../../MainAgents/TargetingAgent.h"
#include "Glob.h"

using namespace BWAPI;

bool ScienceVesselAgent::use_abilities() {
  //Shielding
  if (unit_->getEnergy() >= 100 && Broodwar->getFrameCount() - last_shield_frame_ > 100) {
    auto agent = find_important_unit();
    if (agent) {
      if (agent->is_alive() && TargetingAgent::get_no_attackers(agent) > 0) {
        if (unit_->useTech(TechTypes::Defensive_Matrix, agent->get_unit())) {
          Broodwar << "Used Defense Matrix on " << agent->unit_type().getName() << std::endl;
          last_shield_frame_ = Broodwar->getFrameCount();
          return true;
        }
      }
    }
  }

  //Irradiate
  TechType irradiate = TechTypes::Irradiate;
  if (unit_->getEnergy() >= 75 && Broodwar->getFrameCount() - last_irradiate_frame_ > 200 && Broodwar->self()->hasResearched(irradiate)) {
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
      if (unit_->useTech(irradiate, enemyOrganic)) {
        Broodwar << "Irradiate used on " << enemyOrganic->getType().getName() << std::endl;
        last_irradiate_frame_ = Broodwar->getFrameCount();
        return true;
      }
    }
  }

  //EMP shockwave
  TechType emp = TechTypes::EMP_Shockwave;
  if (Broodwar->self()->hasResearched(emp) && unit_->getEnergy() >= emp.energyCost()) {
    int range = emp.getWeapon().maxRange();
    for (auto& a : Broodwar->enemy()->getUnits()) {
      if (is_emp_target(a) && unit_->getDistance(a) <= range) {
        if (unit_->useTech(emp, a->getPosition())) {
          Broodwar << "EMP Shockwave used on " << a->getType().getName() << std::endl;
          return true;
        }
      }
    }
  }

  return false;
}

bool ScienceVesselAgent::is_emp_target(Unit e) {
  if (e->getShields() < 60) return false;
  if (e->getType().getID() == UnitTypes::Protoss_Carrier.getID()) return true;
  if (e->getType().getID() == UnitTypes::Protoss_Arbiter.getID()) return true;
  if (e->getType().getID() == UnitTypes::Protoss_Photon_Cannon.getID()) return true;
  if (e->getType().getID() == UnitTypes::Protoss_Archon.getID()) return true;
  if (e->getType().getID() == UnitTypes::Protoss_Scout.getID()) return true;
  if (e->getType().getID() == UnitTypes::Protoss_Reaver.getID()) return true;
  return false;
}

const BaseAgent* ScienceVesselAgent::find_important_unit() const {
  const BaseAgent* result = nullptr;

  act::interruptible_for_each_actor<BaseAgent>(
    [this,&result](const BaseAgent* a) {
      if (is_important_unit(a)) {
        double dist = unit_->getDistance(a->get_unit());
        if (dist <= 320) {
          result = a;
          return act::ForEach::Break;
        }
      }
      return act::ForEach::Continue;
    });

  return result;
}

bool ScienceVesselAgent::is_important_unit(const BaseAgent* agent) {
//  UnitType type = agent->getUnitType();

  if (agent->is_of_type(UnitTypes::Terran_Siege_Tank_Tank_Mode)) return true;
  if (agent->is_of_type(UnitTypes::Terran_Siege_Tank_Siege_Mode)) return true;
  if (agent->is_of_type(UnitTypes::Terran_Science_Vessel)) return true;
  if (agent->is_of_type(UnitTypes::Terran_Battlecruiser)) return true;

  return false;
}
