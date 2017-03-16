#include "Config.h"
#include "../Managers/Constructor.h"
#include "../Managers/ExplorationManager.h"

using namespace BWAPI;

Config* Config::instance = nullptr;

Config::Config() {
  version = RNP_VERSION;
  botName = "Raze & Plunder";

  std::stringstream ss;
  ss << "\x1C";
  ss << botName;
  ss << " ";
  ss << version;

  info = ss.str();

  int l = botName.length() + version.length();
  w = 7 + l * 6;
}

Config::~Config() {
  delete instance;
}

Config* Config::getInstance() {
  if (instance == nullptr) {
    instance = new Config();
  }
  return instance;
}

void Config::displayBotName() const {
  if (Broodwar->getFrameCount() >= 10) {
    Broodwar->drawBoxScreen(3, 3, w, 20, Colors::Black, true);
    Broodwar->drawTextScreen(7, 5, info.c_str());
  }
}

std::string Config::getVersion() const {
  return version;
}

std::string Config::getBotName() const {
  return botName;
}
