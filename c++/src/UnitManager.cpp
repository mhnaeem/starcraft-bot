#pragma once

#include "UnitManager.h"
#include "MapTools.h"
#include "InformationManager.h"
#include "SmartUtils.h"
#include <algorithm>
#include <cmath>

UnitManager::UnitManager()
{
	m_unitOrders = std::map<int, UnitOrder>();
}

void UnitManager::onStart()
{
	m_unitOrders.clear();
	m_scoutConfusionAngle = 30.0;

	setupScouts();
}

void UnitManager::onFrame()
{
	idleWorkersCollectMinerals();
	runOrders();
}

void UnitManager::runOrders()
{
	BWAPI::Unitset myUnits = BWAPI::Broodwar->self()->getUnits();
	for (auto unit : myUnits)
	{
		UnitOrder order = m_unitOrders[unit->getID()];
		switch (order)
		{
		case UnitOrder::SCOUT:
			performScouting(unit);
			break;
		case UnitOrder::COLLECT_MINERALS:
			collectMinerals(unit);
			break;
		case UnitOrder::SCOUT_CONFUSION_MICRO:
			performScoutConfusionMicro(unit);
			return;
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
		std::map<int, UnitOrder>::iterator it = m_unitOrders.find(worker->getID());
		if (it == m_unitOrders.end()) {
			// if not in list then make them collect minerals
			m_unitOrders[worker->getID()] = UnitOrder::COLLECT_MINERALS;
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
			m_unitOrders[unit->getID()] = UnitOrder::SCOUT;
			scoutCount++;
		}

		if (scoutCount == 1)
		{
			break;
		}
	}
}

void UnitManager::performScoutConfusionMicro(BWAPI::Unit scout)
{

	const std::vector<BWAPI::Position> enemyCenterLocations = InformationManager::Instance().getEnemyLocations();
	if (enemyCenterLocations.empty())
	{
		return;
	}

	auto enemyWorkerInRadius = [&](BWAPI::Unit scout)
	{
		for (auto& unit : BWAPI::Broodwar->enemy()->getUnits())
		{
			if (unit && unit->exists() && unit->isCompleted() && unit->getType().isWorker() && (unit->getDistance(scout) <= 64))
			{
				return true;
			}
		}
		return false;
	};

	auto const getNewPos = [&](double angle, int radius) {
		double radians = angle * 3.14 / 180;
		int newX = enemyCenterLocations[0].x + (std::cos(radians) * radius);
		int newY = enemyCenterLocations[0].y + (std::sin(radians) * radius);

		return BWAPI::Position(newX, newY);
	};

	if (scout && scout->exists())
	{
		const BWAPI::Unit enemyBase = scout->getUnitsInRadius(1024, BWAPI::Filter::Exists && BWAPI::Filter::IsEnemy && BWAPI::Filter::IsResourceDepot).getClosestUnit();

		if (scout->isUnderAttack() || enemyWorkerInRadius(scout))
		{
			BWAPI::Position newPos = getNewPos(m_scoutConfusionAngle, 200);
			for (size_t i = 0; i < 20; i++)
			{
				if (scout->getLastCommand().getTargetPosition() == newPos && scout->getDistance(newPos) >= 60)
				{
					break;
				}

				m_scoutConfusionAngle += 45.0;
				newPos = getNewPos(m_scoutConfusionAngle, 200);

				if (!SmartUtils::HasAttackingEnemies(BWAPI::Broodwar->getRegionAt(newPos.x, newPos.y)))
				{
					SmartUtils::SmartMove(scout, newPos);
					break;
				}
			}
		}
		else if (scout->getLastCommand().getType() != BWAPI::UnitCommandTypes::Attack_Unit && enemyBase && scout->hasPath(enemyBase))
		{
			SmartUtils::SmartAttack(scout, enemyBase);
		}
		else if (scout->hasPath(enemyCenterLocations[0]))
		{
			SmartUtils::SmartMove(scout, enemyCenterLocations[0]);
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
			m_unitOrders[scout->getID()] = UnitOrder::SCOUT_CONFUSION_MICRO;
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
