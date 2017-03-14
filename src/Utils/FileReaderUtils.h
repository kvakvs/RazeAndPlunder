#pragma once

#include <BWAPI.h>
#include <vector>

using namespace BWAPI;


struct Tokens {
  std::string key;
  std::string value;

  Tokens() {
    key = "";
    value = "";
  }

  Tokens(std::string mKey, std::string mValue) {
    key = mKey;
    value = mValue;
  }
};

/** This class contains some common methods used by classes handling the
 * reading of buildorder/techs/upgrades/squad setup files.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class FileReaderUtils {

private:
  std::vector<Tokens> nameHash;

public:
  FileReaderUtils();

  /** Returns the filename to use in sub folder squads, buildorder or upgrades.
   * The methods checks if for example PvZ is defined, and if not PvX is used. */
  std::string getFilename(std::string subpath);

  /** Returns the path to the folder where the scripfiles are placed. */
  std::string getScriptPath();

  /** Checks if a file in the specified subpath exists, for example PvZ.txt in
   * subfolder buildorder. */
  bool fileExists(std::string subpath, std::string filename);

  /** Returns a unit type from a textline, or Unknown if no type was found. */
  UnitType getUnitType(std::string line);

  /** Returns an upgrade type from a textline, or Unknown if no type was found. */
  UpgradeType getUpgradeType(std::string line);

  /** Returns a tech type from a textline, or Unknown if no type was found. */
  TechType getTechType(std::string line);

  /** Replaces all underscores (_) with whitespaces in a string. */
  void replace(std::string& line);

  /** Splits a line into tokens. Delimiter is the characted to split at, for example = or :. */
  Tokens split(std::string line, std::string delimiter);

  /** Converts a string to an int. */
  int toInt(std::string& str);

  /** Gets the name for the current map. */
  std::string getMapName();

  /** Returns the hash id for a map name. */
  std::string nameToHash(std::string name);

  /** Returns the map name for a hash id. */
  std::string hashToName(std::string hash);

};
