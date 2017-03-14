#pragma once

#include "../MainAgents/BaseAgent.h"


#define DISABLE_UNIT_AI 0

/** The UnitAgent is the base agent class for all agents handling mobile units. If a unit is created and no
 * specific agent for that type is found, the unit is assigned to a UnitAgents. UnitAgents can attack and
 * assist building under enemy fire, but cannot use any special abilities. To use abilities such as Terran Vultures
 * dropping spider mines, an agent implementation for that unit type must be created.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class UnitAgent : public BaseAgent {

protected:

public:
  explicit UnitAgent(BWAPI::Unit mUnit);
  UnitAgent();
  virtual ~UnitAgent();

  // Called each update to issue orders. 
  void computeActions() override;

  // Issues an attack order. True if an order is requested, false otherwise. 
  bool attack();

  // Issues a move order. True if an order is requested, false otherwise. 
  bool move();

  /** Issues an order to use special abilities. True if an order is requested, false otherwise. 
   * Must be implemented in specific subclasses for each unit type.
   */
  virtual bool useAbilities() {
    return false;
  }

  // Used to print info about this agent to the screen. 
  void printInfo() override;
};
