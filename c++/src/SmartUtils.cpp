#pragma once

#include <BWAPI.h>
#include "MapTools.h"
#include "SmartUtils.h"
#include "InformationManager.h"

bool SmartUtils::SmartStop(BWAPI::Unit unit)
{
	if (!unit || !unit->exists() || !unit->isCompleted() || !unit->canStop())
	{
		return false;
	}

	if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Stop)
	{
		return true;
	}

	return unit->stop();
}

bool SmartUtils::SmartAttack(BWAPI::Unit unit, BWAPI::Position pos)
{
	if (!unit || !unit->exists() || !unit->isCompleted() || !unit->canAttack() || !pos || !MapTools::Instance().isValidPosition(pos))
	{
		return false;
	}

	if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Attack_Unit && unit->getLastCommand().getTargetPosition() == pos)
	{
		return true;
	}

	return unit->attack(pos);
}

bool SmartUtils::SmartAttack(BWAPI::Unit unit, BWAPI::Unit enemy)
{
	if (!unit || !unit->exists() || !unit->isCompleted() || !unit->canAttack() || !enemy || !enemy->exists())
	{
		return false;
	}

	if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Attack_Unit && unit->getLastCommand().getTarget() == enemy)
	{
		return true;
	}

	return unit->attack(enemy);
}

bool SmartUtils::SmartMove(BWAPI::Unit unit, BWAPI::Position pos)
{
	if (!unit || !unit->exists() || !unit->isCompleted() || !unit->canMove() || !pos || !MapTools::Instance().isValidPosition(pos) || !MapTools::Instance().isWalkable(BWAPI::TilePosition(pos)))
	{
		return false;
	}

	if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Move && unit->getLastCommand().getTargetPosition() == pos)
	{
		return true;
	}

	return unit->move(pos);
}

bool SmartUtils::SmartMove(BWAPI::Unit unit, BWAPI::TilePosition pos)
{
	if (!pos) { return false; }
	return SmartMove(unit, BWAPI::Position(pos));
}

BWAPI::Unit SmartUtils::GetClosestUnitTo(BWAPI::Position p, const BWAPI::Unitset& units)
{
	BWAPI::Unit closestUnit = nullptr;

	if (!p) { return closestUnit; }
	for (auto& u : units)
	{
		if (!closestUnit || u->getDistance(p) < closestUnit->getDistance(p))
		{
			closestUnit = u;
		}
	}

	return closestUnit;
}

BWAPI::Unit SmartUtils::GetClosestUnitTo(BWAPI::Unit unit, const BWAPI::Unitset& units)
{
	if (!unit)
	{
		return nullptr;
	}

	return GetClosestUnitTo(unit->getPosition(), units);
}

BWAPI::Unitset SmartUtils::SmartDetectEnemy(BWAPI::Unit unit)
{
	if (unit)
	{
		return unit->getUnitsInRadius(1024, BWAPI::Filter::IsEnemy && BWAPI::Filter::Exists && BWAPI::Filter::IsCompleted);
	}
}

bool SmartUtils::SmartRightClick(BWAPI::Unit unit, BWAPI::Unit target)
{
	// if there's no valid unit, ignore the command
	if (!unit || !target) { return false; }

	// Don't issue a 2nd command to the unit on the same frame
	if (unit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount()) { return false; }

	// If we are issuing the same type of command with the same arguments, we can ignore it
	// Issuing multiple identical commands on successive frames can lead to bugs
	if (unit->getLastCommand().getTarget() == target) { return true; }

	// If there's nothing left to stop us, right click!
	return unit->rightClick(target);
}

bool SmartUtils::HasAttackingEnemies(BWAPI::Region region)
{
	for (auto unit : region->getUnits())
	{
		if (unit && unit->exists() && unit->canAttack())
		{
			return true;
		}
	}

	return false;
}

bool SmartUtils::SmartTrain(BWAPI::UnitType type)
{
	if (!type) { return false; }

	if (!InformationManager::Instance().hasEnoughResources(type)) { return false; }

	const std::pair<BWAPI::UnitType, int> whatTrains = type.whatBuilds();

	int unitsReady = 0;
	BWAPI::Unit buildingNeeded = nullptr;

	for (BWAPI::Unit unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (!unit) { continue; }

		if (!unit->exists() || !unit->isCompleted() || unit->isTraining()) { continue; }

		if (unit->getType() == whatTrains.first)
		{
			unitsReady++;
			buildingNeeded = unit;
		}

		if (unitsReady == whatTrains.second)
		{
			break;
		}
	}

	if (!buildingNeeded || unitsReady != whatTrains.second) { return false; }

	bool train = buildingNeeded->train(type);
	if (train)
	{
		InformationManager::Instance().deductResources(type);
	}
	BWAPI::Broodwar->printf("%s %s", train ? "Started Training" : "Couldn't Train", type.getName().c_str());

	return train;
}

bool SmartUtils::SmartTrain(BWAPI::UnitType type, BWAPI::Unit target)
{
	if (!type || !target) { return false; }

	if (!target->exists() || !target->isCompleted()) { return false; }

	if (target->isTraining()) { return true; }

	if (!target->canTrain(type) || !InformationManager::Instance().hasEnoughResources(type)) { return false; }

	bool train = target->train(type);
	if (train)
	{
		InformationManager::Instance().deductResources(type);
	}
	BWAPI::Broodwar->printf("%s %s", train ? "Started Training" : "Couldn't Train", type.getName().c_str());

	return train;
}
