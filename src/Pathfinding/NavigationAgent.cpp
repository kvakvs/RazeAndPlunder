#include "NavigationAgent.h"
#include "PFFunctions.h"
#include "Managers/AgentManager.h"
#include "Influencemap/MapManager.h"
#include "Commander/Commander.h"
#include "Utils/Profiler.h"
#include <math.h>
#include "RnpUtil.h"
#include "Glob.h"

using namespace BWAPI;

NavigationAgent::PFType NavigationAgent::pathfinding_version_
    = NavigationAgent::PFType::HybridBoids;

NavigationAgent::NavigationAgent()
    : bwem_(BWEM::Map::Instance())
{
  check_range_ = 5;
  map_w_ = Broodwar->mapWidth() * 4;
  map_h_ = Broodwar->mapHeight() * 4;
}

NavigationAgent::~NavigationAgent() {
}

bool NavigationAgent::compute_move(const BaseAgent* agent, TilePosition goal) {
  bool cmd = false;
  auto agent_pos = agent->get_unit()->getPosition();

  double r = agent->unit_type().seekRange();
  if (agent->unit_type().sightRange() > r) {
    r = agent->unit_type().sightRange();
  }

  bool enemyInRange = false;
  for (auto& u : Broodwar->enemy()->getUnits()) {
    double dist = agent_pos.getDistance(u->getPosition());
    if (dist <= r) {
      enemyInRange = true;
      break;
    }
  }

  //Retreat to center of the squad if the enemy
  //is overwhelming.
  if (agent->is_under_attack() && (agent->get_unit()->isIdle() || agent->get_unit()->isInterruptible())) {
    int ownI = rnp::map_manager()->get_my_influence_at(agent->get_unit()->getTilePosition());
    int enI = rnp::map_manager()->get_ground_influence_at(agent->get_unit()->getTilePosition());
    if (enI > ownI) {
      //Broodwar << "Retreat from (" << agent->getUnit()->getTilePosition().x << "," << agent->getUnit()->getTilePosition().y << " " << ownI << "<" << enI << endl;
      auto sq = rnp::commander()->get_squad(agent->get_squad_id());
      if (sq && not sq->is_explorer_squad()) {
        TilePosition center = sq->get_center();
        if (center.getDistance(agent->get_unit()->getTilePosition()) >= 4 && not agent->get_unit()->isCloaked()) {
          if (agent->get_unit()->isSieged()) {
            agent->get_unit()->unsiege();
            return true;
          }
          if (agent->get_unit()->isBurrowed()) {
            agent->get_unit()->unburrow();
            return true;
          }
          agent->get_unit()->rightClick(Position(center));
          return true;
        }
      }
    }
  }

  if (enemyInRange) {
    switch (pathfinding_version_) {
      case PFType::Builtin :
        rnp::profiler()->start("NormMove");
        cmd = compute_pathfinding_move(agent, goal);
        rnp::profiler()->end("NormMove");
        break;

      case PFType::HybridBoids :
        rnp::profiler()->start("BoidsMove");
        cmd = compute_boids_move(agent);
        rnp::profiler()->end("BoidsMove");
        break;

      case PFType::HybridPotentialField :
        rnp::profiler()->start("PFmove");
        compute_potential_field_move(agent);
        rnp::profiler()->end("PFmove");
        break;
    }
  }
  else {
    rnp::profiler()->start("NormMove");
    cmd = compute_pathfinding_move(agent, goal);
    rnp::profiler()->end("NormMove");
  }
  return cmd;
}

int NavigationAgent::get_max_unit_size(UnitType type) {
  int size = type.dimensionDown();
  if (type.dimensionLeft() > size) size = type.dimensionLeft();
  if (type.dimensionRight() > size) size = type.dimensionRight();
  if (type.dimensionUp() > size) size = type.dimensionUp();
  return size;
}


bool NavigationAgent::compute_boids_move(const BaseAgent* agent) const {
  if (not agent->get_unit()->isIdle() && not agent->get_unit()->isMoving()) return false;

  Unit unit = agent->get_unit();
  if (unit->isSieged() || unit->isBurrowed() || unit->isLoaded()) {
    return false;
  }

  //The difference from current position the agent
  //shall move to.
  double a_diffx = 0;
  double a_diffy = 0;
  auto agent_pos = agent->get_unit()->getPosition();

  //Apply goal
  if (rnp::is_valid_position(agent->get_goal())) {
    Position goal = Position(agent->get_goal());
    double add_x = ((double)goal.x - (double)agent_pos.x) / 100.0;
    double add_y = ((double)goal.y - (double)agent_pos.y) / 100.0;

    a_diffx += add_x;
    a_diffy += add_y;
  }

  //Apply average position for the squad
  double total_dx = 0.0;
  double total_dy = 0.0;
  auto sq = rnp::commander()->get_squad(agent->get_squad_id());
  if (sq != nullptr) {
    int count = 0;
    act::for_each_in<BaseAgent>(
        sq->get_members(),
        [agent,&count,&total_dx,&total_dy](const BaseAgent* a) {
            if (a->is_alive() && a->get_unit_id() != agent->get_unit_id()) {
              auto a_pos = a->get_unit()->getPosition();
              total_dx += static_cast<double>(a_pos.x);
              total_dy += static_cast<double>(a_pos.y);
              count++;
            }
        });

    total_dx /= count - 1.0;
    total_dy /= count - 1.0;

    double addX = (total_dx - (double)agent_pos.x) / 100.0;
    double addY = (total_dy - (double)agent_pos.y) / 100.0;

    a_diffx += addX;
    a_diffy += addY;
  }

  //Apply average heading for the squad
  total_dx = 0;
  total_dy = 0;
  if (sq != nullptr) {
    int no = 0;
    act::for_each_in<BaseAgent>(
        sq->get_members(),
        [agent,&no,&total_dx,&total_dy](const BaseAgent* a) {
            if (a->is_alive() && a->get_unit_id() != agent->get_unit_id()) {
              auto a_angle = a->get_unit()->getAngle();
              total_dx += cos(a_angle);
              total_dy += sin(a_angle);
              no++;
            }
        });

    total_dx = total_dx / (double)(no - 1);
    total_dy = total_dy / (double)(no - 1);

    double addX = (total_dx - cos(agent->get_unit()->getAngle())) / 5.0;
    double addY = (total_dy - sin(agent->get_unit()->getAngle())) / 5.0;

    a_diffx += addX;
    a_diffy += addY;
  }

  //Apply separation from own units. Does not apply for air units
  total_dx = 0;
  total_dy = 0;
  double detection_lim = 10.0;

  if (sq != nullptr && not agent->unit_type().isFlyer()) {
    int cnt = 0;
    act::for_each_in<BaseAgent>(
        sq->get_members(),
        [&agent_pos,agent,&detection_lim,&cnt,&total_dx,&total_dy]
        (const BaseAgent* a) {
            //Set detection limit to be the radius of both units + 2
            detection_lim = (double) (
                get_max_unit_size(agent->unit_type())
                + get_max_unit_size(a->unit_type()) + 2);

            if (a->is_alive() && a->get_unit_id() != agent->get_unit_id()) {
              auto a_pos = a->get_unit()->getPosition();
              double d = agent_pos.getDistance(a_pos);
              if (d <= detection_lim) {
                total_dx -= (a_pos.x - agent_pos.x);
                total_dy -= (a_pos.y - agent_pos.y);
                cnt++;
              }
            }
        });
    if (cnt > 0) {
      double addX = total_dx / 5.0;
      double addY = total_dy / 5.0;

      a_diffx += addX;
      a_diffy += addY;
    }
  }

  //Apply separation from enemy units
  total_dx = 0;
  total_dy = 0;

  //Check if the agent targets ground and/or air
  //Check range of weapons
  bool targetsGround = false;
  auto ground_weapon = agent->unit_type().groundWeapon();
  int groundRange = 0;
  if (ground_weapon.targetsGround()) {
    targetsGround = true;
    if (ground_weapon.maxRange() > groundRange) {
      groundRange = ground_weapon.maxRange();
    }
  }

  auto air_weapon = agent->unit_type().airWeapon();
  if (air_weapon.targetsGround()) {
    targetsGround = true;
    if (air_weapon.maxRange() > groundRange) {
      groundRange = air_weapon.maxRange();
    }
  }

  bool targetsAir = false;
  int airRange = 0;
  if (ground_weapon.targetsAir()) {
    targetsAir = true;
    if (ground_weapon.maxRange() > groundRange) {
      airRange = ground_weapon.maxRange();
    }
  }
  if (air_weapon.targetsAir()) {
    targetsAir = true;
    if (air_weapon.maxRange() > groundRange) {
      airRange = air_weapon.maxRange();
    }
  }
  //Unit cannot attack
  if (not agent->unit_type().canAttack()) {
    groundRange = 6 * 32;
    airRange = 6 * 32;
  }

  //If agent is retreating from an enemy unit or not
  bool retreat = false;

  //Iterate through enemies
  for (auto& u : Broodwar->enemy()->getUnits()) {
    int cnt = 0;
    if (u->exists()) {
      UnitType t = u->getType();
      
      if (t.isFlyer() && targetsAir) {
        detection_lim = (double)airRange - get_max_unit_size(agent->unit_type()) - 2;
      }
      if (not t.isFlyer() && targetsGround) {
        detection_lim = (double)groundRange - get_max_unit_size(agent->unit_type()) - 2;
      }
      if (not agent->unit_type().canAttack()) {
        retreat = true;
        detection_lim = t.sightRange();
      }
      if (unit->getGroundWeaponCooldown() >= 20 || unit->getAirWeaponCooldown() >= 20) {
        retreat = true;
        detection_lim = t.sightRange();
      }
      if (detection_lim < 5) detection_lim = 5; //Minimum separation

      double d = agent_pos.getDistance(u->getPosition());
      if (d <= detection_lim) {
        total_dx -= (u->getPosition().x - agent_pos.x);
        total_dy -= (u->getPosition().y - agent_pos.y);
        cnt++;
      }
    }
    if (cnt > 0) {
      double addX = total_dx;
      double addY = total_dy;

      a_diffx += addX;
      a_diffy += addY;
    }
  }

  //Apply separation from terrain
  total_dx = 0;
  total_dy = 0;
  int cnt = 0;
  WalkPosition unitWT(agent_pos);
  
  detection_lim = (double)(get_max_unit_size(agent->unit_type()) + 16);

  for (int tx = unitWT.x - 2; tx <= unitWT.x + 2; tx++) {
    for (int ty = unitWT.y - 2; ty <= unitWT.y + 2; ty++) {
      if (not Broodwar->isWalkable(tx, ty)) {

        WalkPosition terrainWT(tx, ty);
        WalkPosition uWT(agent_pos);
        double d = terrainWT.getDistance(uWT);

        if (d <= detection_lim) {
          Position tp(terrainWT);
          total_dx -= (tp.x - agent_pos.x);
          total_dy -= (tp.y - agent_pos.y);
          cnt++;
        }
      }
    }
  }
  if (cnt > 0) {
    double addX = total_dx / 10.0;
    double addY = total_dy / 10.0;

    a_diffx += addX;
    a_diffy += addY;
  }

  //Update new position
  int newX = (int)(agent_pos.x + a_diffx);
  int newY = (int)(agent_pos.y + a_diffy);
  Position toMove(newX, newY);

  if (agent_pos.getDistance(toMove) >= 1) {
    if (retreat) {
      return agent->get_unit()->rightClick(toMove);
    }
    else {
      return agent->get_unit()->attack(toMove);
    }
  }

  return false;
}


bool NavigationAgent::compute_potential_field_move(const BaseAgent* agent) const {
  if (not agent->get_unit()->isIdle() && not agent->get_unit()->isMoving()) return false;

  Unit unit = agent->get_unit();

  if (unit->isSieged() || unit->isBurrowed() || unit->isLoaded()) {
    return false;
  }

  WalkPosition unitWT = WalkPosition(unit->getPosition());
  int wtX = unitWT.x;
  int wtY = unitWT.y;

  float bestP = get_attacking_unit_p(agent, unitWT);
  //bestP += PFFunctions::getGoalP(Position(unitX,unitY), goal);
  //bestP += PFFunctions::getTrailP(agent, unitX, unitY);
  bestP += PFFunctions::get_terrain_p(agent, unitWT);

  float cP = 0;

  float startP = bestP;
  int bestX = wtX;
  int bestY = wtY;

  for (int cX = wtX - check_range_; cX <= wtX + check_range_; cX++) {
    for (int cY = wtY - check_range_; cY <= wtY + check_range_; cY++) {
      if (cX >= 0 && cY >= 0 && cX <= map_w_ && cY <= map_h_) {
        WalkPosition wt = WalkPosition(cX, cY);
        cP = get_attacking_unit_p(agent, wt);
        //cP += PFFunctions::getGoalP(Position(cX,cY), goal);
        //cP += PFFunctions::getTrailP(agent, cX, cY);
        cP += PFFunctions::get_terrain_p(agent, wt);

        if (cP > bestP) {
          bestP = cP;
          bestX = cX;
          bestY = cY;
        }
      }
    }
  }

  if (bestX != wtX || bestY != wtY) {
    WalkPosition wt = WalkPosition(bestX, bestY);
    Position toMove = Position(wt);

    return agent->get_unit()->attack(toMove);
  }

  return false;
}

bool NavigationAgent::compute_pathfinding_move(const BaseAgent* agent,
                                               TilePosition goal) {
  TilePosition checkpoint = goal;
  if (agent->get_squad_id().is_valid()) {
    auto sq = rnp::commander()->get_squad(agent->get_squad_id());
    if (sq) {
      checkpoint = sq->next_move_position();
      if (agent->is_of_type(UnitTypes::Terran_SCV)) {
        checkpoint = sq->next_follow_move_position();
        act::modify_actor<BaseAgent>(
          agent->self(),
          [=](BaseAgent* a) { a->set_goal(checkpoint); });
      }
    }
  }

  if (rnp::is_valid_position(goal)) {
    move_to_goal(agent, checkpoint, goal);
    return true;
  }
  return false;
}

void NavigationAgent::debug_display_pf(const BaseAgent* agent) const {
  Unit unit = agent->get_unit();
  if (unit->isBeingConstructed()) return;

  //PF
  auto agent_pos = agent->get_unit()->getPosition();
  WalkPosition w = WalkPosition(agent_pos);
  int tileX = w.x;
  int tileY = w.y;
  int range = 10 * 3;

  for (int cTileX = tileX - range; cTileX < tileX + range; cTileX += 3) {
    for (int cTileY = tileY - range; cTileY < tileY + range; cTileY += 3) {
      if (cTileX >= 0 && cTileY >= 0 && cTileX < map_w_ && cTileY < map_h_) {
        WalkPosition wt = WalkPosition(cTileX + 1, cTileY + 1);
        float p = get_attacking_unit_p(agent, wt);
        //cP += PFFunctions::getGoalP(Position(cX,cY), goal);
        //cP += PFFunctions::getTrailP(agent, cX, cY);
        p += PFFunctions::get_terrain_p(agent, wt);

        //print box
        if (p > -950) {
          Position pos = Position(wt);
          Broodwar->drawBoxMap(pos.x - 8, pos.y - 8, pos.x + 8, pos.y + 8, get_color(p), true);
        }
      }
    }
  }
}

Color NavigationAgent::get_color(float p) {
  if (p >= 0) {
    int v = (int)(p * 3);
    int halfV = (int)(p * 0.6);

    if (v > 255) v = 255;
    if (halfV > 255) halfV = 255;

    return Color(halfV, halfV, v);
  }
  else {
    p = -p;
    int v = (int)(p * 1.6);

    int v1 = 255 - v;
    if (v1 <= 0) v1 = 40;
    int halfV1 = (int)(v1 * 0.6);

    return Color(v1, halfV1, halfV1);
  }
}

bool NavigationAgent::move_to_goal(const BaseAgent* agent,
                                   TilePosition checkpoint,
                                   TilePosition goal) {
  if (not rnp::is_valid_position(checkpoint)
    || not rnp::is_valid_position(goal)) 
  {
    return false;
  }
  Unit unit = agent->get_unit();

  if (unit->isStartingAttack() || unit->isAttacking()) {
    return false;
  }

  Position to_reach = Position(checkpoint.x * 32 + 16, checkpoint.y * 32 + 16);
  double dist_to_reach = rnp::distance(to_reach, unit->getPosition());

  int engage_dist = (int)(unit->getType().groundWeapon() * 0.5);
  if (engage_dist < 2 * 32) engage_dist = 2 * 32;

  //Explorer units shall have
  //less engage dist to avoid getting
  //stuck at waypoints.
  auto sq = rnp::commander()->get_squad(agent->get_squad_id());
  if (sq) {
    if (sq->is_explorer_squad()) {
      engage_dist = 32;
    }
    else {
      //Add additional distance to avoid clogging
      //choke points.
      engage_dist += 4 * 32;
    }
  }

  if (dist_to_reach <= engage_dist) {
    //Dont stop close to chokepoints
    TilePosition tp = unit->getTilePosition();
    auto cp = rnp::get_nearest_chokepoint(bwem_, tp);
    float d = rnp::distance(tp, TilePosition(cp->Center()));
    if (d > 4.0f) {
      if (unit->isMoving()) unit->stop();
      return true;
    }
  }

  auto squad_id = agent->get_squad_id();
  if (squad_id.is_valid()) {
    auto sq = rnp::commander()->get_squad(squad_id);
    if (sq) {
      if (sq->is_attacking()) {
        //Squad is attacking. Don't stop
        //at checkpoints.
        to_reach = Position(goal.x * 32 + 16, goal.y * 32 + 16);
      }
    }
  }
  //Move
  if (not unit->isMoving()) return unit->attack(to_reach);
  else return true;
}

float NavigationAgent::get_attacking_unit_p(const BaseAgent* agent,
                                            WalkPosition wp) const {
  float p = 0;

  //Enemy Units
  for (auto& u : Broodwar->enemy()->getUnits()) {
    //Enemy seen

    UnitType t = u->getType();
    bool retreat = false;
    if (not agent->unit_type().canAttack() 
        && agent->unit_type().isFlyer()) retreat = true;

    if (not agent->unit_type().canAttack()
        && not agent->unit_type().isFlyer()) retreat = true;

    if (agent->get_unit()->getGroundWeaponCooldown() >= 20 
        || agent->get_unit()->getAirWeaponCooldown() >= 20) retreat = true;

    float dist = PFFunctions::get_distance(wp, u);

    if (not retreat) {
      p += PFFunctions::calc_offensive_unit_p(dist, agent->get_unit(), u);
    }
    else {
      p += PFFunctions::calc_defensive_unit_p(dist, agent->get_unit(), u);
    }
  }

  //Own Units
  act::for_each_actor<BaseAgent>(
    [&p,&wp,agent](const BaseAgent* a) {
      if (a->is_alive()) {
        auto dist = PFFunctions::get_distance(wp, a->get_unit());
        p += PFFunctions::calc_own_unit_p(dist, wp, agent->get_unit(), a->get_unit());
      }
    });

  //Neutral Units
  for (auto& u : Broodwar->getNeutralUnits()) {
    if (u->getType().getID() == UnitTypes::Terran_Vulture_Spider_Mine.getID()) {
      WalkPosition w2 = WalkPosition(u->getPosition());
      float dist = PFFunctions::get_distance(wp, u);
      if (dist <= 2) {
        p -= 50.0;
      }
    }
  }

  return p;
}

float NavigationAgent::get_defending_unit_p(const BaseAgent* agent,
                                            WalkPosition wp) const {
  float p = 0;

  p += PFFunctions::get_goal_p(agent, wp);
  p += PFFunctions::get_terrain_p(agent, wp);

  //Own Units
  act::for_each_actor<BaseAgent>(
    [&p,&wp,agent](const BaseAgent* a) {
      auto dist = PFFunctions::get_distance(wp, a->get_unit());
      auto ptmp = PFFunctions::calc_own_unit_p(dist, wp, agent->get_unit(), a->get_unit());
      p += ptmp;
    });

  return p;
}
