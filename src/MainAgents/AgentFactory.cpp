#include "AgentFactory.h"
#include "WorkerAgent.h"
#include "../StructureAgents/StructureAgent.h"
#include "../UnitAgents/UnitAgent.h"
#include "Commander/Commander.h"
#include "../UnitAgents/TransportAgent.h"

// Terran agents 
#include "../StructureAgents/Terran/CommandCenterAgent.h"
#include "../StructureAgents/Terran/ComsatAgent.h"
#include "../UnitAgents/Terran/SiegeTankAgent.h"
#include "../UnitAgents/Terran/MarineAgent.h"
#include "../UnitAgents/Terran/MedicAgent.h"
#include "../UnitAgents/Terran/VultureAgent.h"
#include "../UnitAgents/Terran/FirebatAgent.h"
#include "../StructureAgents/RefineryAgent.h"
#include "../UnitAgents/Terran/ScienceVesselAgent.h"
#include "../UnitAgents/Terran/BattlecruiserAgent.h"
#include "../UnitAgents/Terran/WraithAgent.h"
#include "../UnitAgents/Terran/GhostAgent.h"

// Protoss agents 
#include "../StructureAgents/Protoss/NexusAgent.h"
#include "../UnitAgents/Protoss/ReaverAgent.h"
#include "../UnitAgents/Protoss/CorsairAgent.h"
#include "../UnitAgents/Protoss/CarrierAgent.h"
#include "../UnitAgents/Protoss/HighTemplarAgent.h"

// Zerg agents 
#include "../StructureAgents/Zerg/HatcheryAgent.h"
#include "../UnitAgents/Zerg/HydraliskAgent.h"
#include "../UnitAgents/Zerg/LurkerAgent.h"
#include "../UnitAgents/Zerg/MutaliskAgent.h"
#include "../UnitAgents/Zerg/QueenAgent.h"
#include "../UnitAgents/Zerg/DefilerAgent.h"
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
    return act::spawn<WorkerAgent>(ActorFlavour::Unit, unit);
  }
  if (unit->getType().isBuilding()) {
    return act::spawn<StructureAgent>(ActorFlavour::Unit, unit);
  }
  return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
}

act::ActorId AgentFactory::create_zerg_agent(Unit unit) {
  UnitType type = unit->getType();

  if (type.isWorker()) {
    return act::spawn<WorkerAgent>(ActorFlavour::Unit, unit);
  }
  if (type.isBuilding()) {
    //Add agents for special buildings here
    if (type.getID() == UnitTypes::Zerg_Hatchery.getID()) {
      return act::spawn<HatcheryAgent>(ActorFlavour::Unit, unit);
    }
    if (type.getID() == UnitTypes::Zerg_Lair.getID()) {
      return act::spawn<HatcheryAgent>(ActorFlavour::Unit, unit);
    }
    if (type.getID() == UnitTypes::Zerg_Hive.getID()) {
      return act::spawn<HatcheryAgent>(ActorFlavour::Unit, unit);
    }
    if (type.getID() == UnitTypes::Zerg_Extractor.getID()) {
      return act::spawn<RefineryAgent>(ActorFlavour::Unit, unit);
    }
    //Default structure agent
    return act::spawn<StructureAgent>(ActorFlavour::Unit, unit);
  }

  if (type.getID() == UnitTypes::Zerg_Overlord.getID()) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Zergling.getID()) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Hydralisk.getID()) {
    return act::spawn<HydraliskAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Mutalisk.getID()) {
    return act::spawn<MutaliskAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Lurker.getID()) {
    return act::spawn<LurkerAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Queen.getID()) {
    return act::spawn<QueenAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Ultralisk.getID()) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Guardian.getID()) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Devourer.getID()) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Defiler.getID()) {
    return act::spawn<DefilerAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Scourge.getID()) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (type.getID() == UnitTypes::Zerg_Infested_Terran.getID()) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  //Default unit agent
  return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
}

act::ActorId AgentFactory::create_terran_agent(Unit unit) {
  if (unit->getType().isWorker()) {
    return act::spawn<WorkerAgent>(ActorFlavour::Unit, unit);
  }
  if (unit->getType().isBuilding()) {
    //Add agents for special buildings here
    if (is_of_type(unit, UnitTypes::Terran_Command_Center)) {
      return act::spawn<CommandCenterAgent>(ActorFlavour::Unit, unit);
    }
    if (is_of_type(unit, UnitTypes::Terran_Comsat_Station)) {
      return act::spawn<ComsatAgent>(ActorFlavour::Unit, unit);
    }
    if (is_of_type(unit, UnitTypes::Terran_Refinery)) {
      return act::spawn<RefineryAgent>(ActorFlavour::Unit, unit);
    }
    if (is_of_type(unit, UnitTypes::Terran_Bunker)) {
      //Make sure we set the squad id to the bunker, so we
      //can remove the squad if the bunker is destroyed.
      auto squad_id = Commander::add_bunker_squad();
      auto a = act::spawn<StructureAgent>(ActorFlavour::Unit, unit);
      msg::unit::set_squad(a, squad_id);
      return a;
    }
    //Default structure agent
    return act::spawn<StructureAgent>(ActorFlavour::Unit, unit);
  }

  if (is_of_type(unit, UnitTypes::Terran_Siege_Tank_Tank_Mode)) {
    return act::spawn<SiegeTankAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Siege_Tank_Siege_Mode)) {
    return act::spawn<SiegeTankAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Marine)) {
    return act::spawn<MarineAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Firebat)) {
    return act::spawn<FirebatAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Medic)) {
    return act::spawn<MedicAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Vulture)) {
    return act::spawn<VultureAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Science_Vessel)) {
    return act::spawn<ScienceVesselAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Battlecruiser)) {
    return act::spawn<BattlecruiserAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Wraith)) {
    return act::spawn<WraithAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Ghost)) {
    return act::spawn<GhostAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Goliath)) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Valkyrie)) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Terran_Dropship)) {
    return act::spawn<TransportAgent>(ActorFlavour::Unit, unit);
  }
  //Default unit agent
  return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
}

act::ActorId AgentFactory::create_protoss_agent(Unit unit) {
  if (unit->getType().isWorker()) {
    return act::spawn<WorkerAgent>(ActorFlavour::Unit, unit);
  }
  if (unit->getType().isBuilding()) {
    //Add agents for special buildings here
    if (is_of_type(unit, UnitTypes::Protoss_Nexus)) {
      return act::spawn<NexusAgent>(ActorFlavour::Unit, unit);
    }
    if (is_of_type(unit, UnitTypes::Protoss_Assimilator)) {
      return act::spawn<RefineryAgent>(ActorFlavour::Unit, unit);
    }
    //Default structure agent
    return act::spawn<StructureAgent>(ActorFlavour::Unit, unit);
  }

  if (is_of_type(unit, UnitTypes::Protoss_Zealot)) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Dragoon)) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Reaver)) {
    return act::spawn<ReaverAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Observer)) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Dark_Templar)) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Scout)) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Shuttle)) {
    return act::spawn<TransportAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Corsair)) {
    return act::spawn<CorsairAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Carrier)) {
    return act::spawn<CarrierAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_High_Templar)) {
    return act::spawn<HighTemplarAgent>(ActorFlavour::Unit, unit);
  }
  if (is_of_type(unit, UnitTypes::Protoss_Arbiter)) {
    return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
  }
  //Default unit agent
  return act::spawn<UnitAgent>(ActorFlavour::Unit, unit);
}

bool AgentFactory::is_of_type(Unit unit, UnitType type) {
  return unit->getType().getID() == type.getID();
}
