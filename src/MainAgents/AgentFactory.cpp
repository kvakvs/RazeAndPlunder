#include "AgentFactory.h"
#include "WorkerAgent.h"
#include "StructureAgents/StructureAgent.h"
#include "UnitAgents/UnitAgent.h"
#include "Commander/Commander.h"
#include "UnitAgents/TransportAgent.h"

// Terran agents 
#include "StructureAgents/Terran/CommandCenterAgent.h"
#include "StructureAgents/Terran/ComsatAgent.h"
#include "UnitAgents/Terran/SiegeTankAgent.h"
#include "UnitAgents/Terran/MarineAgent.h"
#include "UnitAgents/Terran/MedicAgent.h"
#include "UnitAgents/Terran/VultureAgent.h"
#include "UnitAgents/Terran/FirebatAgent.h"
#include "StructureAgents/RefineryAgent.h"
#include "UnitAgents/Terran/ScienceVesselAgent.h"
#include "UnitAgents/Terran/BattlecruiserAgent.h"
#include "UnitAgents/Terran/WraithAgent.h"
#include "UnitAgents/Terran/GhostAgent.h"

// Protoss agents 
#include "StructureAgents/Protoss/NexusAgent.h"
#include "UnitAgents/Protoss/ReaverAgent.h"
#include "UnitAgents/Protoss/CorsairAgent.h"
#include "UnitAgents/Protoss/CarrierAgent.h"
#include "UnitAgents/Protoss/HighTemplarAgent.h"

// Zerg agents 
#include "StructureAgents/Zerg/HatcheryAgent.h"
#include "UnitAgents/Zerg/HydraliskAgent.h"
#include "UnitAgents/Zerg/LurkerAgent.h"
#include "UnitAgents/Zerg/MutaliskAgent.h"
#include "UnitAgents/Zerg/QueenAgent.h"
#include "UnitAgents/Zerg/DefilerAgent.h"
#include "Glob.h"

using namespace BWAPI;

bool AgentFactory::instance_flag_ = false;
AgentFactory* AgentFactory::instance_ = nullptr;

AgentFactory::AgentFactory() {

}

AgentFactory::~AgentFactory() {
  instance_flag_ = false;
  delete instance_;
}

AgentFactory* AgentFactory::get_instance() {
  if (not instance_flag_) {
    instance_ = new AgentFactory();
    instance_flag_ = true;
  }
  return instance_;
}

act::ActorId AgentFactory::create_agent(Unit unit) {
  if (Broodwar->self()->getRace().getID() == Races::Terran.getID()) {
    return create_terran_agent(unit);
  }
  if (Broodwar->self()->getRace().getID() == Races::Protoss.getID()) {
    return create_protoss_agent(unit);
  }
  if (Broodwar->self()->getRace().getID() == Races::Zerg.getID()) {
    return create_zerg_agent(unit);
  }

  //Default agents
  if (unit->getType().isWorker()) {
    return rnp::spawn_unit<WorkerAgent>(unit);
  }
  if (unit->getType().isBuilding()) {
    return rnp::spawn_unit<StructureAgent>(unit);
  }
  return rnp::spawn_unit<UnitAgent>(unit);
}

act::ActorId AgentFactory::create_zerg_agent(Unit unit) const {
  UnitType type = unit->getType();

  if (type.isWorker()) {
    return rnp::spawn_unit<WorkerAgent>(unit);
  }
  if (type.isBuilding()) {
    //Add agents for special buildings here
    if (type.getID() == UnitTypes::Zerg_Hatchery.getID()) {
      return rnp::spawn_unit<HatcheryAgent>(unit);
    }
    if (type.getID() == UnitTypes::Zerg_Lair.getID()) {
      return rnp::spawn_unit<HatcheryAgent>(unit);
    }
    if (type.getID() == UnitTypes::Zerg_Hive.getID()) {
      return rnp::spawn_unit<HatcheryAgent>(unit);
    }
    if (type.getID() == UnitTypes::Zerg_Extractor.getID()) {
      return rnp::spawn_unit<RefineryAgent>(unit);
    }
    //Default structure agent
    return rnp::spawn_unit<StructureAgent>(unit);
  }

  if (type.getID() == UnitTypes::Zerg_Overlord.getID()) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Zergling.getID()) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Hydralisk.getID()) {
    return rnp::spawn_unit<HydraliskAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Mutalisk.getID()) {
    return rnp::spawn_unit<MutaliskAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Lurker.getID()) {
    return rnp::spawn_unit<LurkerAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Queen.getID()) {
    return rnp::spawn_unit<QueenAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Ultralisk.getID()) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Guardian.getID()) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Devourer.getID()) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Defiler.getID()) {
    return rnp::spawn_unit<DefilerAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Scourge.getID()) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (type.getID() == UnitTypes::Zerg_Infested_Terran.getID()) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  //Default unit agent
  return rnp::spawn_unit<UnitAgent>(unit);
}

act::ActorId AgentFactory::create_terran_agent(Unit unit) const {
  if (unit->getType().isWorker()) {
    return rnp::spawn_unit<WorkerAgent>(unit);
  }
  if (unit->getType().isBuilding()) {
    //Add agents for special buildings here
    if (is_of_type(unit, UnitTypes::Terran_Command_Center)) {
      return rnp::spawn_unit<CommandCenterAgent>(unit);
    }
    if (is_of_type(unit, UnitTypes::Terran_Comsat_Station)) {
      return rnp::spawn_unit<ComsatAgent>(unit);
    }
    if (is_of_type(unit, UnitTypes::Terran_Refinery)) {
      return rnp::spawn_unit<RefineryAgent>(unit);
    }
    if (is_of_type(unit, UnitTypes::Terran_Bunker)) {
      //Make sure we set the squad id to the bunker, so we
      //can remove the squad if the bunker is destroyed.
//      auto squad_id = Commander::add_bunker_squad();
      auto a = rnp::spawn_unit<StructureAgent>(unit);
//      msg::unit::set_squad(a, squad_id);
      rnp::log()->warn("spawned bunker, TODO add marines");
      return a;
    }
    //Default structure agent
    return rnp::spawn_unit<StructureAgent>(unit);
  }

  if (is_of_type(unit, UnitTypes::Terran_Siege_Tank_Tank_Mode)) {
    return rnp::spawn_unit<SiegeTankAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Siege_Tank_Siege_Mode)) {
    return rnp::spawn_unit<SiegeTankAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Marine)) {
    return rnp::spawn_unit<MarineAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Firebat)) {
    return rnp::spawn_unit<FirebatAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Medic)) {
    return rnp::spawn_unit<MedicAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Vulture)) {
    return rnp::spawn_unit<VultureAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Science_Vessel)) {
    return rnp::spawn_unit<ScienceVesselAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Battlecruiser)) {
    return rnp::spawn_unit<BattlecruiserAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Wraith)) {
    return rnp::spawn_unit<WraithAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Ghost)) {
    return rnp::spawn_unit<GhostAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Goliath)) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Valkyrie)) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Dropship)) {
    return rnp::spawn_unit<TransportAgent>(unit);
  }
  //Default unit agent
  return rnp::spawn_unit<UnitAgent>(unit);
}

act::ActorId AgentFactory::create_protoss_agent(Unit unit) const {
  if (unit->getType().isWorker()) {
    return rnp::spawn_unit<WorkerAgent>(unit);
  }
  if (unit->getType().isBuilding()) {
    //Add agents for special buildings here
    if (is_of_type(unit, UnitTypes::Protoss_Nexus)) {
      return rnp::spawn_unit<NexusAgent>(unit);
    }
    if (is_of_type(unit, UnitTypes::Protoss_Assimilator)) {
      return rnp::spawn_unit<RefineryAgent>(unit);
    }
    //Default structure agent
    return rnp::spawn_unit<StructureAgent>(unit);
  }

  if (is_of_type(unit, UnitTypes::Protoss_Zealot)) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Dragoon)) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Reaver)) {
    return rnp::spawn_unit<ReaverAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Observer)) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Dark_Templar)) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Scout)) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Shuttle)) {
    return rnp::spawn_unit<TransportAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Corsair)) {
    return rnp::spawn_unit<CorsairAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Carrier)) {
    return rnp::spawn_unit<CarrierAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_High_Templar)) {
    return rnp::spawn_unit<HighTemplarAgent>(unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Arbiter)) {
    return rnp::spawn_unit<UnitAgent>(unit);
  }
  //Default unit agent
  return rnp::spawn_unit<UnitAgent>(unit);
}

bool AgentFactory::is_of_type(Unit unit, UnitType type) {
  return unit->getType().getID() == type.getID();
}
