#pragma once

#include "BuildManager.h"
#include "InformationManager.h"
#include "UnitManager.h"

BuildManager::BuildManager()
{
	m_buildingsInProgress = std::map<BWAPI::UnitType, std::pair<int, int>>();

}

void BuildManager::onStart()
{
	m_buildingsInProgress.clear();
}

void BuildManager::onFrame()
{
	BuildManager::trackBuilds();
}

bool BuildManager::Build(const BaseManager* baseManager, BWAPI::UnitType type)
{
	if (!baseManager) { return false; }

	return BuildManager::Build(baseManager->getLocation(), type);
}

bool BuildManager::Build(BWAPI::Position pos, BWAPI::UnitType type)
{
	if (!type) { return false; }

	if (BuildManager::isBuildInProgress(type)) { return true; }

	if (!InformationManager::Instance().hasEnoughResources(type)) { return false; }

	const BWAPI::UnitType builderType = type.whatBuilds().first;
	if (!builderType) { return false; }

	const int maxBuildRange = 64;
	const bool buildingNearCreep = type.requiresCreep();
	const BWAPI::TilePosition desiredPos = BWAPI::TilePosition(pos);
	const BWAPI::TilePosition buildPos = BWAPI::Broodwar->getBuildLocation(type, desiredPos, maxBuildRange, buildingNearCreep);

	BWAPI::Unit builder = BuildManager::getBuildUnit(buildPos, builderType);
	if (!builder) { return false; }

	const bool build = builder->build(type, buildPos);
	if (build)
	{
		InformationManager::Instance().deductResources(type);
		UnitManager::Instance().setOrder(builder->getID(), UnitOrder::BUILD);
		m_buildingsInProgress[type] = std::pair<int, int>(builder->getID(), InformationManager::Instance().getAllUnitsOfType(type).size());
	}

	BWAPI::Broodwar->printf("%s %s", build ? "Started Building" : "Couldn't Build", type.getName().c_str());
	return build;
}

BWAPI::Unit BuildManager::getBuildUnit(BWAPI::TilePosition buildPos, BWAPI::UnitType builderType)
{
	std::vector<BWAPI::Unit> units = InformationManager::Instance().getAllUnitsOfType(builderType);

	for (auto unit : units)
	{
		if (!unit || !unit->exists() || !unit->isCompleted())
		{
			continue;
		}

		const UnitOrder currentOrder = UnitManager::Instance().getOrder(unit->getID());
		if (currentOrder == UnitOrder::COLLECT_MINERALS)
		{
			return unit;
		}
	}

	return nullptr;
}

bool BuildManager::isBuildInProgress(BWAPI::UnitType type)
{
	if (!type) { return false; }

	std::map<BWAPI::UnitType, std::pair<int, int>>::iterator it = m_buildingsInProgress.find(type);
	if (it == m_buildingsInProgress.end()) {
		return false;
	}

	return true;
}

void BuildManager::trackBuilds()
{
	std::vector<BWAPI::UnitType> toRemove;
	std::map<BWAPI::UnitType, std::pair<int, int>>::iterator it;

	for (it = m_buildingsInProgress.begin(); it != m_buildingsInProgress.end(); it++)
	{
		if (!it->first) { continue; }

		BWAPI::Unit unit = BWAPI::Broodwar->getUnit(it->second.first);

		if (!unit) { continue; }

		if (!unit->exists() || !unit->isCompleted())
		{
			toRemove.push_back(it->first);
			UnitManager::Instance().setOrder(unit->getID(), UnitOrder::COLLECT_MINERALS);
			continue;
		}

		if (InformationManager::Instance().getAllUnitsOfType(it->first).size() <= it->second.second)
		{
			continue;
		}

		toRemove.push_back(it->first);
		UnitManager::Instance().setOrder(unit->getID(), UnitOrder::COLLECT_MINERALS);
	}

	for (auto item : toRemove)
	{
		m_buildingsInProgress.erase(item);
	}
}
