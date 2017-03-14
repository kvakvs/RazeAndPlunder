#pragma once

#include <BWAPI.h>
#include "AIloop.h"

#include "bwem.h"
#include "Commander/Commander.h"

#define TOURNAMENT_NAME "SSCAIT 2017"
#define SPONSORS "the Sponsors!"
#define MINIMUM_COMMAND_OPTIMIZATION 1

// Remember not to use "Broodwar" in any global class constructor!

class BotTournamentModule : public BWAPI::TournamentModule {
  virtual bool onAction(int actionType, void* parameter = nullptr);
  virtual void onFirstAdvertisement();
};

DWORD WINAPI AnalyzeThread();

/** This class contains the main game loop and all events that is broadcasted from the Starcraft engine
* using BWAPI. See the BWAPI documentation for more info.
*
*/
class OpprimoBot : public BWAPI::AIModule {
  bool running = false;
  bool profile = false;
  
  // The singleton
  static OpprimoBot* singleton_;

  int speed = 1;
  AIloop loop;

  // Globally visible resources via the singleton
  BWEM::Map& bwem_;
  Commander::Ptr commander_;

public:
  OpprimoBot();

  static OpprimoBot* singleton() {
    bwem_assert(singleton_);
    return singleton_;
  }
  Commander::Ptr get_commander() { return commander_; }

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
