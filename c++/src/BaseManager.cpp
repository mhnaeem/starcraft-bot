#pragma once

#include "BaseManager.h"
#include "InformationManager.h"
#include <BWAPI.h>
#include <vector>
#include <algorithm>
#include "SmartUtils.h"
#include "BuildManager.h"

BaseManager::BaseManager(BWAPI::Position baseLocation)
{
	m_baseLocation = baseLocation;
	m_regions = std::vector<int>();
	m_chokePoints = std::vector<int>();
	m_units = std::set<int>();
	m_minerals = SmartUtils::GetClosestUnitTo(baseLocation, BWAPI::Broodwar->getMinerals());
	m_gas = SmartUtils::GetClosestUnitTo(baseLocation, BWAPI::Broodwar->getGeysers());

	updateRegions();
	updateChokePoints();
	onFrame();
}

void BaseManager::onFrame()
{
	updateUnits();

	trainWorkers();
}

const std::vector<int> BaseManager::getRegions() const
{
	return m_regions;
}

const std::vector<int> BaseManager::getChokePoints() const
{
	return m_chokePoints;
}

const std::set<int> BaseManager::getUnits() const
{
	return m_units;
}

BWAPI::Unit BaseManager::getMinerals()
{
	return m_minerals;
}

BWAPI::Unit BaseManager::getGas()
{
	return m_gas;
}

BWAPI::Position BaseManager::getLocation()
{
	return m_baseLocation;
}

void BaseManager::updateRegions()
{
	m_regions.clear();
	BWAPI::Region centerRegion = BWAPI::Broodwar->getRegionAt(m_baseLocation.x, m_baseLocation.y);

	if (!centerRegion) { return; }

	std::vector<BWAPI::Region> queue = std::vector<BWAPI::Region>();
	queue.push_back(centerRegion);

	int depth = 0;
	int queueIndex = 0;
	while (true)
	{
		if (queue.empty() || queueIndex == queue.size())
		{
			break;
		}

		BWAPI::Region region = queue[queueIndex];
		if (!region) { continue; }

		m_regions.push_back(region->getID());
		queueIndex++;

		if (depth <= m_depthOfRegions) {
			for (BWAPI::Region expRegion : region->getNeighbors())
			{
				if (!expRegion) { continue; }

				if (std::find(queue.begin(), queue.end(), expRegion) == queue.end())
				{
					queue.push_back(expRegion);
				}
			}
			depth++;
		}
	}
}

void BaseManager::updateChokePoints()
{
	m_chokePoints.clear();

	for (int regionID : m_regions)
	{
		BWAPI::Region region = BWAPI::Broodwar->getRegion(regionID);
		if (!region) { continue; }

		if (region->getDefensePriority() == 2 && region->getNeighbors().size() >= 6)
		{
			m_chokePoints.push_back(regionID);
		}
	}
}

void BaseManager::updateUnits()
{
	m_units.clear();
	
	if (m_regions.empty()) { return; }
	
	for (int regionID : m_regions)
	{
		BWAPI::Region region = BWAPI::Broodwar->getRegion(regionID);

		if (!region) { continue; }

		BWAPI::Unitset units = region->getUnits();
		for (BWAPI::Unit unit : units)
		{
			if (!unit) { continue; }

			int unitID = unit->getID();

			if (m_units.find(unitID) == m_units.end())
			{
				m_units.insert(unitID);
			}
		}
	}
}

void BaseManager::trainWorkers()
{
	if (InformationManager::Instance().usedSupply() % 6 == 0 || InformationManager::Instance().getCountOfType(BWAPI::Broodwar->self()->getRace().getWorker()) <= 5)
	{
		BaseManager::train(BWAPI::Broodwar->self()->getRace().getWorker());
	}
}

bool BaseManager::train(BWAPI::UnitType type)
{
	const std::pair<BWAPI::UnitType, int> whatTrains = type.whatBuilds();

	int unitsReady = 0;
	BWAPI::Unit buildingNeeded = nullptr;

	for (int unitID : m_units)
	{
		BWAPI::Unit unit = BWAPI::Broodwar->getUnit(unitID);

		if (!unit) { continue; }

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

	return SmartUtils::SmartTrain(type, buildingNeeded);
}

bool BaseManager::build(BWAPI::UnitType type)
{
	return BuildManager::Instance().Build(this, type);
}
