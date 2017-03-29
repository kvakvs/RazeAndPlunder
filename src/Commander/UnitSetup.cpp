#include "UnitSetup.h"
#include "BWAPI/UnitType.h"

using namespace BWAPI;

UnitSetup::UnitSetup() {

}

bool UnitSetup::equals(UnitType m_type) const {
  return equals(type_, m_type);
}

bool UnitSetup::equals(UnitType t1, UnitType t2) {
  UnitType used1 = shrink(t1);
  UnitType used2 = shrink(t2);
  return used1.getID() == used2.getID();
}

UnitType UnitSetup::shrink(UnitType t) {
  if (t.getID() == UnitTypes::Terran_Siege_Tank_Siege_Mode) 
    return UnitTypes::Terran_Siege_Tank_Tank_Mode;
  if (t.getID() == UnitTypes::Zerg_Lurker) 
    return UnitTypes::Zerg_Hydralisk;
  if (t.getID() == UnitTypes::Zerg_Guardian) 
    return UnitTypes::Zerg_Mutalisk;
  if (t.getID() == UnitTypes::Zerg_Devourer) 
    return UnitTypes::Zerg_Mutalisk;
  if (t.getID() == UnitTypes::Protoss_High_Templar) 
    return UnitTypes::Protoss_Archon;
  return t;
}
