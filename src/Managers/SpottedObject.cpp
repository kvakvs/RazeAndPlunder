#include "SpottedObject.h"

using namespace BWAPI;

SpottedObject::SpottedObject(Unit m_unit)
: type_(m_unit->getType())
, position_(m_unit->getPosition())
, tile_position_(m_unit->getTilePosition())
, unit_id_(m_unit->getID()) {
}
