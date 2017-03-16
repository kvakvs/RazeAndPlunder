#include <BWAPI.h>
#include <BWAPI/Client.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "RnpBot.h"

using namespace BWAPI;

void drawStats();
void drawBullets();
void drawVisibilityData();
void showPlayers();
void showForces();
bool show_bullets;
bool show_visibility_data;

void reconnect() {
  while (not BWAPIClient.connect()) {
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
  }
}

int main(int argc, const char* argv[]) {
  std::cout << BOT_PREFIX "Connecting..." << std::endl;
  reconnect();
  while (true) {
    std::cout << "  - waiting to enter match" << std::endl;
    while (not Broodwar->isInGame()) {
      BWAPI::BWAPIClient.update();
      if (!BWAPI::BWAPIClient.isConnected()) {
        std::cout << BOT_PREFIX "Reconnecting..." << std::endl;
        reconnect();
      }
    }

    RnpBot bot;

    std::cout << BOT_PREFIX "starting match!" << std::endl;
    
    // Enable some cheat flags
    Broodwar->enableFlag(Flag::UserInput);
    // Uncomment to enable complete map information
    //Broodwar->enableFlag(Flag::CompleteMapInformation);

    show_bullets = false;
    show_visibility_data = false;

    if (Broodwar->isReplay()) {
      Broodwar << "Players are in this replay:" << std::endl;;
      Playerset players = Broodwar->getPlayers();

      for (auto p : players) {
        if (!p->getUnits().empty() && !p->isNeutral())
          Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
      }
    }
    else {
//      if (Broodwar->enemy())
//        Broodwar << "The match up is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;
//
//      //send each worker to the mineral field that is closest to it
//      Unitset units = Broodwar->self()->getUnits();
//      Unitset minerals = Broodwar->getMinerals();
//      for (auto& u : units) {
//        if (u->getType().isWorker()) {
//          Unit closestMineral = nullptr;
//
//          for (auto& m : minerals) {
//            if (!closestMineral || u->getDistance(m) < u->getDistance(closestMineral))
//              closestMineral = m;
//          }
//          if (closestMineral)
//            u->rightClick(closestMineral);
//        }
//        else if (u->getType().isResourceDepot()) {
//          //if this is a center, tell it to build the appropiate type of worker
//          u->train(Broodwar->self()->getRace().getWorker());
//        }
//      }
    }
    while (Broodwar->isInGame()) {
      for (auto& e : Broodwar->getEvents()) {
        switch (e.getType()) {
        case EventType::MatchFrame:
          bot.onFrame();
          break;

        case EventType::MatchEnd:
          bot.onEnd(e.isWinner());
          break;

        case EventType::SendText:
          bot.onSendText(e.getText());
          break;

        case EventType::ReceiveText:
          bot.onReceiveText(e.getPlayer(), e.getText());
          break;

        case EventType::PlayerLeft:
          bot.onPlayerLeft(e.getPlayer());
          break;

        case EventType::NukeDetect:
          bot.onNukeDetect(e.getPosition());
          break;

        case EventType::UnitCreate:
          bot.onUnitCreate(e.getUnit());
          break;

        case EventType::UnitDestroy:
          bot.onUnitDestroy(e.getUnit());
          break;

        case EventType::UnitMorph:
          bot.onUnitMorph(e.getUnit());
          break;

        case EventType::UnitShow:
          bot.onUnitShow(e.getUnit());
          break;

        case EventType::UnitHide:
          bot.onUnitHide(e.getUnit());
          break;

        case EventType::UnitRenegade:
          bot.onUnitRenegade(e.getUnit());
          break;

        case EventType::SaveGame:
          bot.onSaveGame(e.getText());
          break;

        case EventType::MatchStart: 
          bot.onStart();
          break;

        case EventType::MenuFrame: break;
        
        case EventType::UnitDiscover: 
          bot.onUnitDiscover(e.getUnit());
          break;

        case EventType::UnitEvade: 
          bot.onUnitEvade(e.getUnit());
          break;

        case EventType::UnitComplete: 
          bot.onUnitComplete(e.getUnit());
          break;

        case EventType::None: break;
        default: ;
        }
      }

      //Broodwar->drawTextScreen(300, 0, "FPS: %f", Broodwar->getAverageFPS());

      BWAPI::BWAPIClient.update();
      if (not BWAPI::BWAPIClient.isConnected()) {
        std::cout << BOT_PREFIX "Reconnecting..." << std::endl;
        reconnect();
      }
    }
    std::cout << BOT_PREFIX "Game ended" << std::endl;
  }

  std::cout << BOT_PREFIX "Press ENTER to continue..." << std::endl;
  std::cin.ignore();
  return 0;
}
