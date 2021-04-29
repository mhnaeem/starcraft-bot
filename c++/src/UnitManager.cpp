#pragma once

#include "UnitManager.h"
#include "MapTools.h"
#include "InformationManager.h"
#include "SmartUtils.h"

UnitManager::UnitManager()
{
	m_unitOrders = std::map<BWAPI::Unit, UnitOrder>();
}

void UnitManager::onStart()
{
	m_unitOrders.clear();

	setupScouts();
}

void UnitManager::onFrame()
{
	runOrders();
}

void UnitManager::runOrders()
{
	BWAPI::Unitset myUnits = BWAPI::Broodwar->self()->getUnits();
	for (auto unit : myUnits)
	{
		UnitOrder order = m_unitOrders[unit];
		switch (order)
		{
		case SCOUT:
			performScouting(unit);
			break;
		case COLLECT_MINERALS:
			collectMinerals(unit);
			break;
		default:
			break;
		}
	}
}

void UnitManager::idleWorkersCollectMinerals()
{
	std::vector<BWAPI::Unit> workers = InformationManager::Instance().getAllUnitsOfType(BWAPI::Broodwar->self()->getRace().getWorker());
	for (auto worker : workers)
	{
		std::map<BWAPI::Unit, UnitOrder>::iterator it = m_unitOrders.find(worker);
		if (it == m_unitOrders.end()) {
			// if not in list then make them collect minerals
			m_unitOrders[worker] = UnitOrder::COLLECT_MINERALS;
		}
	}
}

void UnitManager::setupScouts()
{
	int scoutCount = 0;
	std::vector<BWAPI::Unit> possibleScouts = InformationManager::Instance().getAllUnitsOfType(BWAPI::Broodwar->self()->getRace().getWorker());
	for (auto unit : possibleScouts)
	{
		if (unit && unit->exists())
		{
			m_unitOrders[unit] = UnitOrder::SCOUT;
			scoutCount++;
		}

		if (scoutCount == 1)
		{
			break;
		}
	}
}

bool UnitManager::performScouting(BWAPI::Unit scout)
{
	if (!scout || !scout->exists() || !scout->isCompleted() || !InformationManager::Instance().getEnemyLocations().empty())
	{
		return false;
	}

	BWAPI::Unitset& enemyUnits = SmartUtils::SmartDetectEnemy(scout);
	if (!enemyUnits.empty())
	{
		BWAPI::Unit enemyBase = nullptr;

		for (auto enemyUnit : enemyUnits)
		{
			if (enemyUnit->getType().isResourceDepot())
			{
				enemyBase = enemyUnit;
				break;
			}
		}

		if (enemyBase)
		{
			InformationManager::Instance().addEnemyPosition(enemyBase->getPosition());
			return false;
		}
	}


	for (auto pos : BWAPI::Broodwar->getStartLocations())
	{
		if (pos && !MapTools::Instance().isExplored(pos))
		{
			bool moved = SmartUtils::SmartMove(scout, pos);
			if (moved)
			{
				return moved;
			}
		}
	}

	return false;
}

bool UnitManager::collectMinerals(BWAPI::Unit worker)
{
	if (!worker || !worker->exists() || !worker->isCompleted() || worker->getType() != BWAPI::Broodwar->self()->getRace().getWorker())
	{
		return false;
	}

	BWAPI::Unit mineralField = InformationManager::Instance().getBases()[0].getMinerals();

	return SmartUtils::SmartRightClick(worker, mineralField);
}
