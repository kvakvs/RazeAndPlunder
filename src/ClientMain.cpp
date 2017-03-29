#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <spdlog/spdlog.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "RnpBot.h"
#include "RnpConst.h"
#include "Glob.h"

#ifndef _MSC_VER
#   include <unistd.h>
#endif

using namespace BWAPI;

void ms_sleep(int ms) {
#ifdef _MSC_VER
  std::this_thread::sleep_for(std::chrono::milliseconds{ ms });
#else
  ::usleep(ms * 1000);
#endif
}

void reconnect() {
  while (not BWAPIClient.connect()) {
    ms_sleep(1000);
  }
}


void handle_event(RnpBot* bot, const Event& e) {
  switch (e.getType()) {
  case EventType::MatchFrame:
    bot->onFrame();
    break;

  case EventType::MatchEnd:
    bot->onEnd(e.isWinner());
    break;

  case EventType::SendText:
    bot->onSendText(e.getText());
    break;

  case EventType::ReceiveText:
    bot->onReceiveText(e.getPlayer(), e.getText());
    break;

  case EventType::PlayerLeft:
    bot->onPlayerLeft(e.getPlayer());
    break;

  case EventType::NukeDetect:
    bot->onNukeDetect(e.getPosition());
    break;

  case EventType::UnitCreate:
    bot->onUnitCreate(e.getUnit());
    break;

  case EventType::UnitDestroy:
    bot->onUnitDestroy(e.getUnit());
    break;

  case EventType::UnitMorph:
    bot->onUnitMorph(e.getUnit());
    break;

  case EventType::UnitShow:
    bot->onUnitShow(e.getUnit());
    break;

  case EventType::UnitHide:
    bot->onUnitHide(e.getUnit());
    break;

  case EventType::UnitRenegade:
    bot->onUnitRenegade(e.getUnit());
    break;

  case EventType::SaveGame:
    bot->onSaveGame(e.getText());
    break;

  case EventType::MatchStart:
    bot->onStart();
    break;

  case EventType::MenuFrame: break;

  case EventType::UnitDiscover:
    bot->onUnitDiscover(e.getUnit());
    break;

  case EventType::UnitEvade:
    bot->onUnitEvade(e.getUnit());
    break;

  case EventType::UnitComplete:
    bot->onUnitComplete(e.getUnit());
    break;

  case EventType::None: break;
  default: break;
  }
}

void play() {
  auto log = rnp::log();

  log->trace("  - waiting to enter match");
  while (not Broodwar->isInGame()) {
    BWAPIClient.update();
    if (!BWAPIClient.isConnected()) {
      rnp::log()->info("Reconnecting...");
      reconnect();
    }
  }

  auto bot = std::make_unique<RnpBot>();

  log->info("starting match!");

  if (Broodwar->isReplay()) {
    Broodwar << "Players are in this replay:" << std::endl;;
    auto players = Broodwar->getPlayers();

    for (auto p : players) {
      if (!p->getUnits().empty() && !p->isNeutral())
        Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
    }
  }

  log->flush();

  //
  // Main game loop is here
  //
  while (Broodwar->isInGame() && not bot->is_game_finished()) {
    for (auto& e : Broodwar->getEvents()) {
      handle_event(bot.get(), e);

      if (bot->is_game_finished() || not Broodwar->isInGame()) {
        goto poof_gone;
      }
    }

    BWAPIClient.update();

    if (not BWAPIClient.isConnected()) {
      log->info("Reconnecting...");
      reconnect();
    }
  }
poof_gone: // we love goto, let's use more of that

  log->info("Game ended");
  log->flush();

  while (Broodwar->isInGame()) {
    BWAPIClient.update();
    if (!BWAPIClient.isConnected()) {
      log->info("waiting for the game end...");
      reconnect();
    }
  }
}

int main(int argc, const char* argv[]) {
  rnp::start_logging();
  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern("%L [%H:%M:%S.%e] %v");

  rnp::log()->info("Connecting...");
  reconnect();

  while (true) {
    play();
  }

  rnp::log()->info("Press ENTER to continue...");
  std::cin.ignore();
  return 0;
}
