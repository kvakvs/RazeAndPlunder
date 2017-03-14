#pragma once

#include <BWAPI.h>

using namespace BWAPI;


struct CTokens {
  std::string key;
  std::string value;
};

/** This class handles some global settings for the bot.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Config {

private:
  std::string botName;
  std::string version;
  std::string info;
  int w;

  static Config* instance;

  Config();

public:
  ~Config();

  /** Returns class instance. */
  static Config* getInstance();

  /** Returns the name of the bot as specified in the config file. */
  std::string getBotName() const;

  /** Returns the current bot version. */
  std::string getVersion() const;

  /** Displays bot name in the game window. */
  void displayBotName() const;
};
