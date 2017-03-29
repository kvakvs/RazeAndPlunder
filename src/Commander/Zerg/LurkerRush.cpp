#if 0

#include "LurkerRush.h"
#include "Managers/BuildplanEntry.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include "Commander/RushSquad.h"
#include "Commander/ExplorationSquad.h"
#include "Glob.h"

using namespace BWAPI;

LurkerRush::LurkerRush()
    : main_sq_(), l1_sq_(), sc1_sq_(), sc2_sq_()
{
  plan(UnitTypes::Zerg_Spawning_Pool, 5);
  plan(UnitTypes::Zerg_Extractor, 5);
  plan(UnitTypes::Zerg_Hydralisk_Den, 8);
  plan(TechTypes::Lurker_Aspect, 13);

  main_sq_ = std::make_shared<Squad>(1, Squad::SquadType::OFFENSIVE, "MainSquad", 10);
  main_sq_->set_required(true);
  main_sq_->set_buildup(true);
  squads_.push_back(main_sq_);

  l1_sq_ = std::make_shared<RushSquad>(2, "LurkerSquad", 8);
  l1_sq_->add_setup(UnitTypes::Zerg_Hydralisk, 4);
  l1_sq_->set_buildup(false);
  l1_sq_->set_required(false);
  l1_sq_->set_active_priority(11);
  l1_sq_->set_morph_to(UnitTypes::Zerg_Lurker);
  squads_.push_back(l1_sq_);

  sc1_sq_ = std::make_shared<ExplorationSquad>(4, "ScoutingSquad", 8);
  sc1_sq_->add_setup(UnitTypes::Zerg_Overlord, 1);
  sc1_sq_->set_required(false);
  sc1_sq_->set_buildup(false);
  sc1_sq_->set_active_priority(10);
  squads_.push_back(sc1_sq_);

  sc2_sq_ = std::make_shared<RushSquad>(5, "RushSquad", 7);
  sc2_sq_->add_setup(UnitTypes::Zerg_Zergling, 2);
  sc2_sq_->set_required(false);
  sc2_sq_->set_buildup(false);
  sc2_sq_->set_active_priority(1000);

  workers_num_ = 8;
  workers_per_refinery_ = 3;
}

LurkerRush::~LurkerRush() {
}

void LurkerRush::on_frame() {
  on_frame_base();

  workers_num_ = rnp::agent_manager()->get_bases_count() * 6 + rnp::agent_manager()->get_units_of_type_count(UnitTypes::Zerg_Extractor) * 3;

  int cSupply = Broodwar->self()->supplyUsed() / 2;

  if (stage_ == 0 && rnp::agent_manager()->get_finished_units_count(UnitTypes::Zerg_Lair) > 0) {
    //Check if we have spotted any enemy buildings. If not, send
    //out two Zerglings to rush base locations. Only needed for
    //2+ player maps.
    //This is needed to find out where the enemy is before we
    //send out the Lurkers.
    //auto start_loc = Broodwar->self()->getStartLocation();
    TilePosition tp = rnp::exploration()->get_random_spotted_building();
    if (not rnp::is_valid_position(tp)
      && Broodwar->getStartLocations().size() > 2) 
    {
      squads_.push_back(sc2_sq_);
    }
    stage_++;
  }
  if (stage_ == 1 && rnp::agent_manager()->get_finished_units_count(UnitTypes::Zerg_Lurker) > 0) {
    plan(UnitTypes::Zerg_Spire, cSupply);
    plan(UnitTypes::Zerg_Hatchery, cSupply);

    main_sq_->add_setup(UnitTypes::Zerg_Mutalisk, 20);
    main_sq_->set_buildup(false);

    stage_++;
  }
  if (stage_ == 2 && rnp::agent_manager()->get_bases_count() > 1) {
    plan(UnitTypes::Zerg_Creep_Colony, cSupply);
    plan(UnitTypes::Zerg_Creep_Colony, cSupply);

    stage_++;
  }
  if (stage_ == 3 && rnp::agent_manager()->get_finished_units_count(UnitTypes::Zerg_Sunken_Colony) > 0) {
    plan(UnitTypes::Zerg_Hatchery, cSupply);

    stage_++;
  }
}

#endif // 0
