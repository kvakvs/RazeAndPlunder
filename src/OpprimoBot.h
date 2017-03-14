#pragma once
#include <BWAPI.h>
#include "AIloop.h"

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
private:
  bool running;
  bool profile;
  void gameStopped();

public:
  // Virtual functions for callbacks, leave these as they are.
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual void onSendText(std::string text);
  virtual void onReceiveText(BWAPI::Player player, std::string text);
  virtual void onPlayerLeft(BWAPI::Player player);
  virtual void onNukeDetect(BWAPI::Position target);
  virtual void onUnitDiscover(BWAPI::Unit unit);
  virtual void onUnitEvade(BWAPI::Unit unit);
  virtual void onUnitShow(BWAPI::Unit unit);
  virtual void onUnitHide(BWAPI::Unit unit);
  virtual void onUnitCreate(BWAPI::Unit unit);
  virtual void onUnitDestroy(BWAPI::Unit unit);
  virtual void onUnitMorph(BWAPI::Unit unit);
  virtual void onUnitRenegade(BWAPI::Unit unit);
  virtual void onSaveGame(std::string gameName);
  virtual void onUnitComplete(BWAPI::Unit unit);
  // Everything below this line is safe to modify.

  int speed;
  AIloop loop;
};
