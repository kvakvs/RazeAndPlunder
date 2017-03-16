#include "BuildplanEntry.h"

using namespace BWAPI;

BuildplanEntry::BuildplanEntry(UnitType cType, int cSupply)
    : unit_type_(cType), upgrade_type_(), tech_type_()
    , supply_(cSupply)
    , type_(BUILDING) {
}

BuildplanEntry::BuildplanEntry(UpgradeType cType, int cSupply)
    : unit_type_(), upgrade_type_(cType), tech_type_()
    , supply_(cSupply)
    , type_(UPGRADE) {
}

BuildplanEntry::BuildplanEntry(TechType cType, int cSupply)
    : unit_type_(), upgrade_type_(), tech_type_(cType)
    , supply_(cSupply)
    , type_(TECH)
{
}
