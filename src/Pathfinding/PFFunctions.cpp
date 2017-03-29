#include "PFFunctions.h"
#include <iso646.h>
#include "RnpUtil.h"

using namespace BWAPI;

float PFFunctions::get_distance(WalkPosition w1, 
                                WalkPosition w2) {
  return static_cast<float>(rnp::distance(w1, w2));
}

float PFFunctions::get_distance(WalkPosition wt, 
                                Unit unit) {
  auto w2 = WalkPosition(unit->getPosition());
  return static_cast<float>(rnp::distance(wt, w2));
}

float PFFunctions::calc_own_unit_p(float d, 
                                   WalkPosition wt, 
                                   Unit unit, 
                                   Unit otherOwnUnit) 
{
  if (unit->getID() == otherOwnUnit->getID()) {
    //Dont count collision with yourself...
    return 0.0f;
  }

  if (otherOwnUnit->getType().isFlyer()) {
    //Cannot collide with flying units.
    return 0.0f;
  }
  if (unit->getType().isFlyer()) {
    bool follow = false;
    auto uid = unit->getType().getID();
    if (uid == UnitTypes::Terran_Science_Vessel.getID() 
        && otherOwnUnit->getType().isMechanical()) follow = true;
    else if (uid == UnitTypes::Terran_SCV.getID() 
        && otherOwnUnit->getType().isMechanical()) follow = true;
    else if (uid == UnitTypes::Protoss_Arbiter.getID()) follow = true;
    else if (uid == UnitTypes::Zerg_Overlord.getID()) follow = true;

    if (follow) {
      //Flying support unit. Make ground units slightly
      //attractive to group up squads.
      float p = static_cast<float>(100.0f - d * 0.2f);
      if (p < 0.0f) p = 0.0f;
      return p;
    }
    //Offensive flying unit. No collisions.
    return 0;
  }

  float p = 0;

  if ((unit->isCloaked() && not otherOwnUnit->isCloaked())
      || (unit->isBurrowed() && not otherOwnUnit->isBurrowed())) {
    //Let cloaked or burrowed units stay away from visible
    //units to avoid getting killed by splash damage.
    if (d <= 4.0f) {
      p = -50.0f;
    }
  }

  if (otherOwnUnit->isIrradiated()) {
    //Other unit under Irradite. Keep distance.
    if (d <= 2.0f) {
      p = -50.0f;
    }
  }
  if (otherOwnUnit->isUnderStorm()) {
    //Other unit under Psionic Storm. Keep distance.
    if (d <= 3.0f) {
      p = -50.0f;
    }
  }

  if (not unit->getType().isBuilding()) {
    if (d <= 2.0f) {
      p = -25.0f;
    }
  }

  if (otherOwnUnit->getType().isBuilding()) {
    UnitType t = otherOwnUnit->getType();
    TilePosition ut = TilePosition(wt);
    TilePosition ot = otherOwnUnit->getTilePosition();
    for (int cx = ot.x; cx < ot.x + t.tileWidth(); cx++) {
      for (int cy = ot.y; cy < ot.y + t.tileHeight(); cy++) {
        if (ut.x == cx && ut.y == cy) p = -50.0f;
      }
    }
  }
  return p;
}

float PFFunctions::get_trail_p(const BaseAgent* agent, 
                               WalkPosition wt) {
  if (agent->get_unit()->isBeingConstructed()) return 0;

  float p = 0;

  //Add current position to trail
  WalkPosition walk_pos(agent->get_unit()->getPosition());
  msg::unit::add_trail_pos(agent->self(), walk_pos);
  //agent->add_trail_position(WalkPosition(agent->get_unit()->getPosition()));

  //Get trail
  std::vector<WalkPosition> trail = agent->get_trail();
  for (int i = 0; i < static_cast<int>(trail.size()); i++) {
    WalkPosition twt = trail[i];
    float d = static_cast<float>(twt.getDistance(wt));
    if (d <= 1.5) p = -10.0;
  }

  return p;
}

float PFFunctions::get_terrain_p(const BaseAgent* agent,
                                 WalkPosition wt) {
  if (agent->unit_type().isFlyer()) return 0.0f;
  if (not Broodwar->isWalkable(wt)) return -1000.0f;

  return 0.0f;
}

float PFFunctions::get_goal_p(const BaseAgent* agent,
                              WalkPosition wt) {
  TilePosition goal = agent->get_goal();
  if (not rnp::is_valid_position(goal)) return 0.0f;

  //Calc max wep range
  int range = 0;
  if (agent->unit_type().isFlyer()) {
    range = agent->unit_type().airWeapon().maxRange();
  }
  else {
    range = agent->unit_type().groundWeapon().maxRange();
  }
  if (range == 0) {
    //Non-attacking unit. Use sight range
    range = agent->unit_type().sightRange();
  }

  //Set good defensive range
  range = static_cast<int>(range * 0.5);
  //if (range < 64) range = 64;
  //Convert range to walktiles
  range = range / 32;

  float p = 0.0f;

  WalkPosition ut = WalkPosition(agent->get_unit()->getPosition());
  double d = ut.getDistance(wt);

  if (d < range) {
    p = static_cast<float>(100.0f - range * 2.0f);
    if (p < 0.0f) {
      p = 0.0f;
    }
  }
  else if (d >= range && d < range + 1) {
    p = 80.0f;
  }
  else {
    float d1 = (float)(d - range);

    p = 80.0f - (float)d1;
    if (p < 0.0f) {
      p = 0.0f;
    }
  }

  return p;
}

float PFFunctions::calc_offensive_unit_p(float d, 
                                         Unit attacker,
                                         Unit enemy) {
  //Check if enemy unit exists and is visible.
  if (not enemy->exists()
      || not enemy->isVisible()
      || enemy->isCloaked()) {
    return 0.0f;
  }

  //SCV:s shall not attack
  if (attacker->getType().isWorker()) {
    return 0.0f;
  }

  //Check for flying buildings
  if (enemy->getType().isFlyingBuilding() && enemy->isLifted()) {
    return 0.0f;
  }

  //Check if we can attack the type
  if (enemy->getType().isFlyer() && not attacker->getType().airWeapon().targetsAir()) {
    return 0.0f;
  }
  if (not enemy->getType().isFlyer() && not attacker->getType().groundWeapon().targetsGround()) {
    return 0.0f;
  }

  //Calc max wep range
  int my_msd;
  if (enemy->getType().isFlyer()) {
    my_msd = get_air_range(attacker) - 1;
  }
  else {
    my_msd = get_ground_range(attacker) - 1;
  }

  if (not attacker->getType().canAttack()) {
    //Unit cannot attack, use sightrange instead
    my_msd = 4;//attacker->getType().sightRange() / 8;
  }

  if (attacker->getType().getID() == UnitTypes::Terran_Medic.getID()) {
    my_msd = 6;
  }
  if (attacker->getType().getID() == UnitTypes::Terran_SCV.getID()) {
    my_msd = 8;
  }
  if (attacker->getType().getID() == UnitTypes::Protoss_High_Templar.getID()) {
    my_msd = 6;
  }
  if (attacker->getType().getID() == UnitTypes::Zerg_Overlord.getID()) {
    my_msd = 12;
  }

  //Calc attacker wep range
  int enemy_msd;
  if (attacker->getType().isFlyer()) {
    enemy_msd = get_air_range(enemy);
  }
  else {
    enemy_msd = get_ground_range(enemy);
  }

  float p = 0;

  //Enemy cannot attack back. It is safe to move
  //closer than MSD.
  if (enemy_msd == 0) {
    enemy_msd = (int)(enemy_msd * 0.5f);
  }

  if (can_attack(attacker, enemy)) {
    if (d < my_msd - 1) {
      float fact = 100.0f / my_msd;
      p = d * fact;
      if (p < 0) {
        p = 0;
      }
    }
    else if (d >= my_msd - 1 && d <= my_msd) {
      p = 100.0f;
    }
    else {
      float d1 = d - my_msd;

      p = 80.0f - d1;
      if (p < 0) {
        p = 0;
      }
    }
  }

  return p;
}

float PFFunctions::calc_defensive_unit_p(float d, 
                                         Unit attacker,
                                         Unit enemy) {
  //Check if enemy unit exists and is visible.
  if (not enemy->exists()) {
    return 0.0f;
  }

  //Check for flying buildings
  if (enemy->getType().isFlyingBuilding() && enemy->isLifted()) {
    return 0.0f;
  }

  //Calc attacker wep range
  int enemy_msd;
  if (attacker->getType().isFlyer()) {
    enemy_msd = get_air_range(enemy) + 2;
  }
  else {
    enemy_msd = get_ground_range(enemy) + 2;
  }
  //Cloaked unit: Watch out for detectors.
  if (attacker->isCloaked() && enemy->getType().isDetector()) {
    enemy_msd = (int)(enemy->getType().sightRange() / 8 + 4);
  }

  float p = 0.0f;

  //Defensive mode -> retreat
  p = (-80.0f + d) / 2;
  if (p > 0.0f) {
    p = 0.0f;
  }

  return (float)p;
}

int PFFunctions::get_ground_range(Unit c_unit) {
  int range = 0;
  if (c_unit->getType().groundWeapon().targetsGround()) {
    int gw_r1 = c_unit->getType().groundWeapon().maxRange();
    if (gw_r1 > range) {
      range = gw_r1;
    }
  }
  else if (c_unit->getType().airWeapon().targetsGround()) {
    int gw_r2 = c_unit->getType().airWeapon().maxRange();
    if (gw_r2 > range) {
      range = gw_r2;
    }
  }
  else if (c_unit->getType().getID() == UnitTypes::Terran_Bunker.getID()) {
    range = UnitTypes::Terran_Marine.groundWeapon().maxRange();
  }

  return range / 8;
}

int PFFunctions::get_air_range(Unit c_unit) {
  int range = 0;
  if (c_unit->getType().groundWeapon().targetsAir()) {
    int gw_r1 = c_unit->getType().groundWeapon().maxRange();
    if (gw_r1 > range) {
      range = gw_r1;
    }
  }
  else if (c_unit->getType().airWeapon().targetsAir()) {
    int gw_r2 = c_unit->getType().airWeapon().maxRange();
    if (gw_r2 > range) {
      range = gw_r2;
    }
  }
  else if (c_unit->getType().getID() == UnitTypes::Terran_Bunker.getID()) {
    range = UnitTypes::Terran_Marine.groundWeapon().maxRange();
  }

  return range / 8;
}

bool PFFunctions::can_attack(Unit ownUnit, 
                             Unit target) {
  UnitType o_type = ownUnit->getType();
  UnitType t_type = target->getType();

  if (t_type.isFlyer()) {
    //Own unit is air
    if (o_type.groundWeapon().targetsAir()) {
      return true;
    }
    if (o_type.airWeapon().targetsAir()) {
      return true;
    }
  }
  else {
    //Own unit is ground
    if (o_type.groundWeapon().targetsGround()) {
      return true;
    }
    if (o_type.airWeapon().targetsGround()) {
      return true;
    }
  }

  return false;
}
