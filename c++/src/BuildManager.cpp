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

bool BuildManager::Build(BaseManager* baseManager, BWAPI::UnitType type)
{
	if (!baseManager) { return false; }

	return BuildManager::Build(baseManager->getLocation(), type);
}

bool BuildManager::Build(BWAPI::UnitType type)
{
	std::vector<BaseManager> bases = InformationManager::Instance().getBases();
	BWAPI::Position pos = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	if (!bases.empty())
	{
		pos = bases[0].getLocation();
	}

	return BuildManager::Build(pos, type);
}

bool BuildManager::Build(BWAPI::Position pos, BWAPI::UnitType type)
{
	if (!type) { return false; }

	if (!type.isBuilding()) { return false; }

	const BWAPI::UnitType builderType = type.whatBuilds().first;
	if (!builderType) { return false; }

	BWAPI::Unit builder = UnitManager::Instance().getBuildUnit(builderType);
	if (!builder) { return false; }

	return BuildManager::Build(pos, builder, type);
}

bool BuildManager::Build(BWAPI::Position pos, BWAPI::Unit builder, BWAPI::UnitType type)
{
	if (!type || !pos || !builder || !builder->exists() || !builder->isCompleted()) { return false; }

	if (!type.isBuilding()) { return false; }

	if (BuildManager::isBuildInProgress(type)) { return true; }

	if (!InformationManager::Instance().hasEnoughResources(type)) { return false; }

	const int maxBuildRange = 100;
	const bool buildingNearCreep = type.requiresCreep();
	const BWAPI::TilePosition desiredPos = BWAPI::TilePosition(pos);
	const BWAPI::TilePosition buildPos = BWAPI::Broodwar->getBuildLocation(type, desiredPos, maxBuildRange, buildingNearCreep);

	BWAPI::UnitCommand command = builder->getLastCommand();
	if (command.getType() == BWAPI::UnitCommandTypes::Build && command.getUnitType() == type) { return true; }

	const bool build = builder->build(type, buildPos);
	if (build)
	{
		InformationManager::Instance().deductResources(type);
		UnitManager::Instance().setOrder(builder->getID(), UnitOrder::BUILD);
		m_buildingsInProgress[type] = std::pair<int, int>(builder->getID(), InformationManager::Instance().getCountOfType(type));
	}

	BWAPI::Broodwar->printf("%s %s", build ? "Started Building" : "Couldn't Build", type.getName().c_str());
	return build;
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

		if (!unit->exists() || !unit->isCompleted() || unit->getLastCommand().getType() != BWAPI::UnitCommandTypes::Build)
		{
			toRemove.push_back(it->first);
			UnitManager::Instance().setOrder(unit->getID(), UnitOrder::COLLECT_MINERALS);
			continue;
		}

		if (it->first == BWAPI::UnitTypes::Protoss_Photon_Cannon)
		{
			std::cout << "num of canon: " << InformationManager::Instance().getCountOfType(it->first);
		}

		if (InformationManager::Instance().getCountOfType(it->first) <= it->second.second)
		{
			continue;
		}

		toRemove.push_back(it->first);

		UnitOrder order = UnitOrder::COLLECT_MINERALS;
		if (UnitManager::Instance().isCamper(unit->getID()))
		{
			order = UnitOrder::CAMP;
		}
		UnitManager::Instance().setOrder(unit->getID(), order);
	}

	for (BWAPI::UnitType item : toRemove)
	{
		m_buildingsInProgress.erase(item);
	}
}

std::set<BWAPI::UnitType> BuildManager::BuildingsNeeded(BWAPI::UnitType building)
{
	std::set<BWAPI::UnitType> set = std::set<BWAPI::UnitType>();

	std::function<void(BWAPI::UnitType)> fillUp = [&](BWAPI::UnitType type)
	{
		if (!type || type == BWAPI::Broodwar->self()->getRace().getResourceDepot() || type == BWAPI::Broodwar->self()->getRace().getWorker())
		{
			return;
		}

		std::map<BWAPI::UnitType, int> required = type.requiredUnits();

		for (std::map<BWAPI::UnitType, int>::const_iterator i = required.begin(); i != required.end(); i++)
		{
			const int count = InformationManager::Instance().getCountOfType(i->first);

			if (count >= i->second) { continue; }

			if (set.find(i->first) != set.end()) { continue; }

			set.insert(i->first);
			fillUp(i->first);
		}
	};
	set.insert(building);
	fillUp(building);
	return set;
}
