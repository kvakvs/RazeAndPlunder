#pragma once

#include "MainAgents/BaseAgent.h"


/** Helper class for the NavigationAgent. Contains methods to calculate the potentials generated by own units, enemy units
 * and neutral objects in specific points.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class PFFunctions {

private:

public:

  // Returns the distance between two positions. 
  static float getDistance(BWAPI::WalkPosition w1, BWAPI::WalkPosition w2);

  // Returns the distance between a unit and a position. 
  static float getDistance(BWAPI::WalkPosition wt, BWAPI::Unit unit);

  // Calculates the potential otherOwnUnit generates around unit at distance d. 
  static float calcOwnUnitP(float d, BWAPI::WalkPosition wt, BWAPI::Unit unit, BWAPI::Unit otherOwnUnit);

  // Calculates the terrain potential in a position. 
  static float getTerrainP(BaseAgent* agent, BWAPI::WalkPosition wt);

  // Calculates the trail potential in a position. Trails are used to reduce the local
  // optima problem when using potential fields. 
  static float getTrailP(BaseAgent* agent, BWAPI::WalkPosition wt);

  // Calculates the goal potential for a defending unit. 
  static float getGoalP(BaseAgent* agent, BWAPI::WalkPosition wt);

  // Calculates the potential an offensive attacking unit generates at distance d around an enemy unit. 
  static float calcOffensiveUnitP(float d, BWAPI::Unit attacker, BWAPI::Unit enemy);

  // Calculates the potential a defensive attacking unit generates at distance d around an enemy unit. 
  static float calcDefensiveUnitP(float d, BWAPI::Unit attacker, BWAPI::Unit enemy);

  // Returns true if the own unit can attack the target. 
  static bool canAttack(BWAPI::Unit ownUnit, BWAPI::Unit target);

  // Returns the max range for the unit targeting ground. 
  static int getGroundRange(BWAPI::Unit cUnit);

  // Returns the max range for the unit targeting air. 
  static int getAirRange(BWAPI::Unit cUnit);

};
