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
	m_regions = std::vector<BWAPI::Region>();
	m_chokePoints = std::vector<BWAPI::Region>();
	m_units = std::set<BWAPI::Unit>();
	m_minerals = SmartUtils::GetClosestUnitTo(baseLocation, BWAPI::Broodwar->getMinerals());
	m_gas = SmartUtils::GetClosestUnitTo(baseLocation, BWAPI::Broodwar->getGeysers());

	onFrame();
}

void BaseManager::onFrame()
{
	updateRegions();
	updateChokePoints();
	updateUnits();

	trainWorkers();
}

const std::vector<BWAPI::Region>& BaseManager::getRegions() const
{
	return m_regions;
}

const std::vector<BWAPI::Region>& BaseManager::getChokePoints() const
{
	return m_chokePoints;
}

const std::set<BWAPI::Unit>& BaseManager::getUnits() const
{
	return m_units;
}

const BWAPI::Unit& BaseManager::getMinerals() const
{
	return m_minerals;
}

const BWAPI::Unit& BaseManager::getGas() const
{
	return m_gas;
}

const BWAPI::Position& BaseManager::getLocation() const
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

		BWAPI::Region& region = queue[queueIndex];
		m_regions.push_back(region);
		queueIndex++;

		if (depth <= m_depthOfRegions) {
			for (auto expRegion : region->getNeighbors())
			{
				if (expRegion && std::find(queue.begin(), queue.end(), expRegion) == queue.end())
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

	for (auto region : m_regions)
	{
		if (region && region->getDefensePriority() == 2)
		{
			m_chokePoints.push_back(region);
		}
	}
}

void BaseManager::updateUnits()
{
	m_units.clear();
	
	if (m_regions.empty()) { return; }
	
	for (auto region : m_regions)
	{
		if (region)
		{
			BWAPI::Unitset& units = region->getUnits();
			for (auto unit : units)
			{
				if (m_units.find(unit) == m_units.end())
				{
					m_units.insert(unit);
				}
			}
		}
	}
}

void BaseManager::trainWorkers()
{
	if (InformationManager::Instance().usedSupply() % 6 == 0 || InformationManager::Instance().getAllUnitsOfType(BWAPI::Broodwar->self()->getRace().getWorker()).size() <= 5)
	{
		BaseManager::train(BWAPI::Broodwar->self()->getRace().getWorker());
	}
}

bool BaseManager::train(BWAPI::UnitType type)
{
	const std::pair<BWAPI::UnitType, int> whatTrains = type.whatBuilds();

	int unitsReady = 0;
	BWAPI::Unit buildingNeeded = nullptr;

	for (auto unit : m_units)
	{
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
