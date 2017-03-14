#pragma once

#include <BWAPI.h>
#include "AIloop.h"

#include "bwem.h"

#define TOURNAMENT_NAME "SSCAIT 2017"
#define SPONSORS "the Sponsors!"
#define MINIMUM_COMMAND_OPTIMIZATION 1

// Remember not to use "Broodwar" in any global class constructor!

class BotTournamentModule : public BWAPI::TournamentModule {
  virtual bool onAction(int actionType, void* parameter = nullptr);
  virtual void onFirstAdvertisement();
};

class Commander;
class Profiler;

/** This class contains the main game loop and all events that is broadcasted from the Starcraft engine
* using BWAPI. See the BWAPI documentation for more info.
*
*/
class RnpBot : public BWAPI::AIModule {
  bool running_ = false;
  bool profile_ = false;
  
  // The singleton
  static RnpBot* singleton_;

  int speed_ = 1;
  AIloop ai_loop_;

public:
  // Globally visible resources via the singleton
  BWEM::Map& bwem_;
  std::shared_ptr<Commander> commander_;
  std::unique_ptr<Profiler> profiler_;

public:
  RnpBot();

  static RnpBot* singleton() {
    bwem_assert(singleton_);
    return singleton_;
  }

  // Virtual functions for callbacks, leave these as they are.
  void onStart() override;
  void onEnd(bool isWinner) override;
  void onFrame() override;
  void onSendText(std::string text) override;
  void onReceiveText(BWAPI::Player player, std::string text) override;
  void onPlayerLeft(BWAPI::Player player) override;
  void onNukeDetect(BWAPI::Position target) override;
  void onUnitDiscover(BWAPI::Unit unit) override;
  void onUnitEvade(BWAPI::Unit unit) override;
  void onUnitShow(BWAPI::Unit unit) override;
  void onUnitHide(BWAPI::Unit unit) override;
  void onUnitCreate(BWAPI::Unit unit) override;
  void onUnitDestroy(BWAPI::Unit unit) override;
  void onUnitMorph(BWAPI::Unit unit) override;
  void onUnitRenegade(BWAPI::Unit unit) override;
  void onSaveGame(std::string gameName) override;
  void onUnitComplete(BWAPI::Unit unit) override;

private:
  void gameStopped();
};
