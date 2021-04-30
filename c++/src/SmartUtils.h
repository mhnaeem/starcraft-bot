#pragma once

#include <BWAPI.h>
#include "MapTools.h"

namespace SmartUtils
{
	bool              SmartStop(BWAPI::Unit unit);
	bool              SmartAttack(BWAPI::Unit unit, BWAPI::Position pos);
	bool              SmartAttack(BWAPI::Unit unit, BWAPI::Unit enemy);
	bool              SmartMove(BWAPI::Unit unit, BWAPI::Position pos);
	bool              SmartMove(BWAPI::Unit unit, BWAPI::TilePosition pos);
	bool              SmartRightClick(BWAPI::Unit unit, BWAPI::Unit target);
	bool              HasAttackingEnemies(BWAPI::Region region);
	BWAPI::Unitset    SmartDetectEnemy(BWAPI::Unit unit);
	BWAPI::Unit       GetClosestUnitTo(BWAPI::Position p, const BWAPI::Unitset& units);
	BWAPI::Unit       GetClosestUnitTo(BWAPI::Unit unit, const BWAPI::Unitset& units);
};
