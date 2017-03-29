#include "TargetingAgent.h"
#include <iso646.h>

using namespace BWAPI;

bool TargetingAgent::can_attack(UnitType type) {
  if (type.isBuilding()) {
    if (type.canAttack()) return true;
    return false;
  }
  if (type.isAddon()) {
    return false;
  }
  if (type.isWorker()) {
    return false;
  }
  return true;
}

int TargetingAgent::get_no_attackers(const BaseAgent* agent) {
  int cnt = 0;

  for (auto& u : Broodwar->enemy()->getUnits()) {
    if (can_attack(u->getType())) {
      int enemyMSD = 0;
      if (agent->unit_type().isFlyer()) {
        enemyMSD = u->getType().airWeapon().maxRange();
      }
      else {
        enemyMSD = u->getType().groundWeapon().maxRange();
      }

      double d = agent->get_unit()->getPosition().getDistance(u->getPosition());
      if (d <= enemyMSD) {
        cnt++;
      }
    }
  }

  return cnt;
}

bool TargetingAgent::check_target(const BaseAgent* agent) {
  if (not agent->get_unit()->isIdle() && not agent->get_unit()->isMoving()) return false;

  Unit pTarget = find_target(agent);
  if (pTarget != nullptr && pTarget->getPlayer()->isEnemy(Broodwar->self())) {
    bool ok = agent->get_unit()->attack(pTarget, true);
    if (not ok) {
      //Broodwar << "Switch target failed: " << Broodwar->getLastError() << endl;
    }
    return ok;
  }
  return false;
}

bool TargetingAgent::is_highprio_target(UnitType type) {
  if (type.getID() == UnitTypes::Terran_Bunker.getID()) return true;
  if (type.getID() == UnitTypes::Terran_Battlecruiser.getID()) return true;
  if (type.getID() == UnitTypes::Terran_Missile_Turret.getID()) return true;

  if (type.getID() == UnitTypes::Protoss_Carrier.getID()) return true;
  if (type.getID() == UnitTypes::Protoss_Photon_Cannon.getID()) return true;
  if (type.getID() == UnitTypes::Protoss_Archon.getID()) return true;

  if (type.getID() == UnitTypes::Zerg_Sunken_Colony.getID()) return true;
  if (type.getID() == UnitTypes::Zerg_Spore_Colony.getID()) return true;
  if (type.getID() == UnitTypes::Zerg_Ultralisk.getID()) return true;

  return false;
}

Unit TargetingAgent::find_highprio_target(const BaseAgent* agent, int maxDist, bool targetsAir, bool targetsGround) {
  Unit target = nullptr;
  Position cPos = agent->get_unit()->getPosition();
  int bestTargetScore = -10000;

  for (auto& u : Broodwar->enemy()->getUnits()) {
    if (u->exists()) {
      UnitType t = u->getType();
      bool targets = is_highprio_target(t);
      if (t.isFlyer() && not targetsAir) targets = false;
      if (not t.isFlyer() && not targetsGround) targets = false;

      if (targets) {
        double dist = cPos.getDistance(u->getPosition());
        if (dist <= (double)maxDist) {
          if (t.destroyScore() > bestTargetScore) {
            target = u;
            bestTargetScore = t.destroyScore();
          }
        }
      }
    }
  }

  return target;
}

Unit TargetingAgent::find_target(const BaseAgent* agent) {
  //Check if the agent targets ground and/or air
  bool targetsGround = agent->can_target_ground();
  bool targetsAir = agent->can_target_air();

  //Iterate through enemies to select a target
  int bestTargetScore = -10000;
  Unit target = nullptr;
  for (auto& u : Broodwar->enemy()->getUnits()) {
    UnitType t = u->getType();

    bool canAttack = false;
    if (not t.isFlyer() && targetsGround) canAttack = true;
    if ((t.isFlyer() || u->isLifted()) && targetsAir) canAttack = true;
    if (u->isCloaked() && not u->isDetected()) {
      canAttack = false;
      handle_cloaked_unit(u);
    }
    if (u->isBurrowed() && not u->isDetected()) {
      canAttack = false;
      handle_cloaked_unit(u);
    }

    int maxRange = 600;
    if (agent->get_unit()->isSieged() || agent->get_unit()->isBurrowed() || agent->get_unit()->isLoaded()) maxRange = agent->unit_type().groundWeapon().maxRange();

    if (canAttack && agent->get_unit()->getPosition().getDistance(u->getPosition()) <= maxRange) {
      double mod = get_target_modifier(agent->get_unit()->getType(), t);
      int cScore = (int)((double)t.destroyScore() * mod);
      if (u->getHitPoints() < u->getInitialHitPoints()) {
        //Prioritize damaged targets
        cScore++;
      }

      if (cScore > bestTargetScore) {
        bestTargetScore = cScore;
        target = u;
      }
    }
  }

  return target;
}

double TargetingAgent::get_target_modifier(UnitType attacker, UnitType target) {
  //Non-attacking buildings
  if (target.isBuilding() && not target.canAttack() && not target.getID() == UnitTypes::Terran_Bunker.getID()) {
    return 0.05;
  }

  //Terran Goliath prefer air targets
  if (attacker.getID() == UnitTypes::Terran_Goliath.getID()) {
    if (target.isFlyer()) return 2;
  }

  //Siege Tanks prefer to take out enemy defense buildings
  if (attacker.getID() == UnitTypes::Terran_Siege_Tank_Siege_Mode.getID()) {
    if (target.isBuilding() && target.canAttack()) return 1.5;
    if (target.getID() == UnitTypes::Terran_Bunker.getID()) return 1.5;
  }

  //Siege Tanks are nasty and have high prio to be killed.
  if (target.getID() == UnitTypes::Terran_Siege_Tank_Siege_Mode.getID()) {
    return 1.5;
  }

  //Prio to take out detectors when having cloaking units
  if (is_cloaking_unit(attacker) && target.isDetector()) {
    return 2;
  }

  if (attacker.isFlyer() && not target.airWeapon().targetsAir()) {
    //Target cannot attack back. Set to low prio
    return 0.1;
  }

  if (not attacker.isFlyer() && not target.groundWeapon().targetsGround()) {
    //Target cannot attack back. Set to low prio
    return 0.1;
  }

  if (target.isWorker()) {
    //Workers are important but very weak units
    return 3;
  }

  return 1; //Default: No modifier
}

void TargetingAgent::handle_cloaked_unit(Unit unit) {
  //Terran: Cloaked units are handled by ComSat agent

  //Add code for handling cloaked units here.
}

bool TargetingAgent::is_cloaking_unit(UnitType type) {
  if (type.isCloakable()) return true;
  return false;
}
