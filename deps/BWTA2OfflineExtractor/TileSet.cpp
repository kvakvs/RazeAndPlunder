#include "..\BWTA\Source\MapData.h"
#include "..\BWTA\Source\TileType.h"
#include "TileSet.h"


TileType* TileSet::getTileType(TileID tileID)
{
  if ( BWTA::MapData::TileSet )
    return  (BWTA::MapData::TileSet + ((tileID >> 4 ) & 0x7FF));
  return NULL;
}