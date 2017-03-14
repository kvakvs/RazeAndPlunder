#include "Config.h"
#include "../Managers/Constructor.h"
#include "../Managers/ExplorationManager.h"

Config* Config::instance = nullptr;

Config::Config()
{
	version = "15.4";
	botName = "OpprimoBot";

	stringstream ss;
	ss << "\x1C";
	ss << botName;
	ss << " ";
	ss << version;

	info = ss.str();

	int l = botName.length()+version.length();
	w = 7+l*6;
}

Config::~Config()
{
	delete instance;
}

Config* Config::getInstance()
{
	if (instance == nullptr)
	{
		instance = new Config();
	}
	return instance;
}

void Config::displayBotName()
{
	if (Broodwar->getFrameCount() >= 10)
	{
		Broodwar->drawBoxScreen(3,3,w,20,Colors::Black,true);
		Broodwar->drawTextScreen(7,5, info.c_str());
	}
}

string Config::getVersion()
{
	return version;
}

string Config::getBotName()
{
	return botName;
}
