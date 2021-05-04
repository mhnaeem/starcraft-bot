#pragma once

#include "UnitManager.h"
#include "MapTools.h"
#include "InformationManager.h"
#include "SmartUtils.h"
#include "BuildManager.h"
#include <algorithm>
#include <cmath>

UnitManager::UnitManager()
{
	m_unitOrders = std::map<int, UnitOrder>();
	m_campers = std::set<int>();
}

void UnitManager::onStart()
{
	m_unitOrders.clear();
	m_campers.clear();
	m_scoutConfusionAngle = 30.0;
	setupScouts();
}

void UnitManager::onFrame()
{
	assignTasksToIdleUnits();
	runOrders();

	for (auto m : m_campers)
	{
		BWAPI::Unit u = BWAPI::Broodwar->getUnit(m);
		if (!u) { continue; }
		std::string x = "";

		UnitOrder order = m_unitOrders[u->getID()];
		switch (order)
		{
		case UnitOrder::SCOUT:
			x = "scout ";
			break;
		case UnitOrder::COLLECT_MINERALS:
			x = "mineral ";
			break;
		case UnitOrder::SCOUT_CONFUSION_MICRO:
			x = "scout mirco ";
			break;
		case UnitOrder::COLLECT_GAS:
			x = "gass ";
			break;
		case UnitOrder::CAMP:
			x = "camp  ";
			break;
		case UnitOrder::RALLY:
			x = "rally ";
			break;
		default:
			break;
		}

		std::cout << u->getID() << " " << u->getLastCommand().getType().getName() << " " << u->getLastCommand().getUnitType().getName() << " " << x << "\n";
	}
}

void UnitManager::runOrders()
{
	BWAPI::Unitset myUnits = BWAPI::Broodwar->self()->getUnits();
	for (BWAPI::Unit unit : myUnits)
	{
		if (!unit) { continue; }

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
			break;
		case UnitOrder::COLLECT_GAS:
			collectGas(unit);
			break;
		case UnitOrder::CAMP:
			camp(unit);
			break;
		case UnitOrder::RALLY:
			rally(unit);
			break;
		default:
			break;
		}
	}
}

void UnitManager::assignTasksToIdleUnits()
{
	BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
	BWAPI::Unitset units = BWAPI::Broodwar->self()->getUnits();

	for (BWAPI::Unit unit : units)
	{
		if (!unit || !unit->exists() || !unit->isCompleted()) { continue; }

		int unitID = unit->getID();
		BWAPI::UnitType unitType = unit->getType();
		UnitOrder command = m_unitOrders[unitID];
		std::map<int, UnitOrder>::iterator it = m_unitOrders.find(unitID);

		// if already in list then skip
		if (it != m_unitOrders.end()) { continue; }

		if (unitType != workerType && !unitType.isBuilding())
		{
			m_unitOrders[unitID] = UnitOrder::RALLY;
		}

		if (unit->isIdle())
		{
			if (command == UnitOrder::BUILD || command == UnitOrder::COLLECT_GAS)
			{
				m_unitOrders[unitID] = UnitOrder::COLLECT_MINERALS;
			}
		}
	}

	if (UnitManager::unitsWithOrder(UnitOrder::COLLECT_GAS) < 2 && InformationManager::Instance().getCountOfType(BWAPI::Broodwar->self()->getRace().getRefinery()) > 0)
	{
		int gasCollectors = 0;
		for (size_t i = 0; i < units.size(); i++)
		{
			if (gasCollectors >= 2)
			{
				break;
			}

			BWAPI::Unit possibleGasCollector = UnitManager::getBuildUnit(workerType);

			if (!possibleGasCollector || !possibleGasCollector->exists() || !possibleGasCollector->isCompleted())
			{
				continue;
			}

			if (m_unitOrders[possibleGasCollector->getID()] == UnitOrder::COLLECT_MINERALS)
			{
				m_unitOrders[possibleGasCollector->getID()] = UnitOrder::COLLECT_GAS;
				gasCollectors++;
			}
		}
	}
}

void UnitManager::setupScouts()
{
	int scoutCount = 0;
	std::vector<int> possibleScouts = InformationManager::Instance().getAllUnitsOfType(BWAPI::Broodwar->self()->getRace().getWorker());
	for (int unitID : possibleScouts)
	{
		BWAPI::Unit unit = BWAPI::Broodwar->getUnit(unitID);
		if (unit && unit->exists() && unit->isCompleted())
		{
			m_unitOrders[unitID] = UnitOrder::SCOUT;
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

	std::vector<BaseManager> enemyCenterLocations = InformationManager::Instance().getEnemyBases();
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
		int newX = enemyCenterLocations[0].getLocation().x + (std::cos(radians) * radius);
		int newY = enemyCenterLocations[0].getLocation().y + (std::sin(radians) * radius);

		return BWAPI::Position(newX, newY);
	};

	if (!scout || !scout->exists() || !scout->isCompleted())
	{
		return;
	}

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
	else if (scout->hasPath(enemyCenterLocations[0].getLocation()))
	{
		SmartUtils::SmartMove(scout, enemyCenterLocations[0].getLocation());
	}
}

bool UnitManager::performScouting(BWAPI::Unit scout)
{
	if (!scout || !scout->exists() || !scout->isCompleted() || !InformationManager::Instance().getEnemyBases().empty())
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
			InformationManager::Instance().addEnemyBase(enemyBase->getPosition());
			m_unitOrders[scout->getID()] = UnitOrder::SCOUT_CONFUSION_MICRO;
			UnitManager::sendCamper();
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

	std::vector<BaseManager> bases = InformationManager::Instance().getBases();
	if (bases.empty()) { return false; }

	BWAPI::Unit mineralField = bases[0].getMinerals();

	return SmartUtils::SmartRightClick(worker, mineralField);
}

bool UnitManager::collectGas(BWAPI::Unit worker)
{
	if (!worker || !worker->exists() || !worker->isCompleted() || !worker->getType().isWorker())
	{
		return false;
	}

	auto units = InformationManager::Instance().getAllUnitsOfType(BWAPI::Broodwar->self()->getRace().getRefinery());
	if (units.empty()) { return false; }

	BWAPI::Unit refinery = BWAPI::Broodwar->getUnit(units[0]);

	if (!refinery || !refinery->exists() || !refinery->isCompleted()) { return false; }
	return SmartUtils::SmartRightClick(worker, refinery);
}

void UnitManager::setOrder(int unitID, UnitOrder order)
{
	m_unitOrders[unitID] = order;
}

UnitOrder UnitManager::getOrder(int unitID)
{
	return m_unitOrders[unitID];
}

void UnitManager::attack()
{
	auto enemy = InformationManager::Instance().getEnemyBases();
	if (enemy.empty())
	{
		return;
	}

	auto enemyWorkerInRadius = [&](BWAPI::Unit u)
	{
		for (auto& unit : BWAPI::Broodwar->enemy()->getUnits())
		{
			if (unit && unit->exists() && unit->isCompleted() && (unit->getDistance(u) <= 200))
			{
				return true;
			}
		}
		return false;
	};

	auto enemyLocation = enemy[0].getLocation();

	for (BWAPI::Unit unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (!unit || !unit->isCompleted() || !unit->exists()) { continue; }

		if (unit->getType().isWorker() || unit->getType().isBuilding()) { continue; }

		BWAPI::Unitset attackers = SmartUtils::SmartDetectEnemy(unit);
		if (unit->isUnderAttack() || !attackers.empty())
		{
			SmartUtils::SmartAttack(unit, attackers.getClosestUnit());
		}
		else
		{
			SmartUtils::SmartMove(unit, enemyLocation);
		}
	}
}

void UnitManager::camp(BWAPI::Unit unit)
{
	if (!unit || !unit->exists() || !unit->isCompleted()) { return; }

	if (m_centerPosition == BWAPI::Positions::Invalid)
	{
		std::vector<BaseManager> enemyBases = InformationManager::Instance().getEnemyBases();
		if (enemyBases.empty()) { return; }

		std::vector<int> enemyChokepoints = enemyBases[0].getChokePoints();
		if (enemyChokepoints.empty()) { return; }

		BWAPI::Region enemyChokepoint = BWAPI::Broodwar->getRegion(enemyChokepoints[0]);
		if (!enemyChokepoint) { return; }

		m_centerPosition = enemyChokepoint->getCenter();
		if (!m_centerPosition) { return; }
	}

	std::cout << m_centerPosition.x << " , " << m_centerPosition.y << "\n";

	const int buildRange = 600;
	const int detectRange = 200;

	BWAPI::Broodwar->drawCircleMap(unit->getPosition(), detectRange, BWAPI::Colors::Red, false);
	BWAPI::Broodwar->drawCircleMap(unit->getPosition(), buildRange, BWAPI::Colors::Green, false);

	BWAPI::Unitset enemyUnits = SmartUtils::SmartDetectEnemy(detectRange, unit);
	if (!enemyUnits.empty())
	{
		BWAPI::Unit enemy = enemyUnits.getClosestUnit();
		if (enemy && enemy->exists())
		{
			BWAPI::Position enemyPos = enemy->getPosition();
			int distanceFromCampLocationToEnemy = enemyPos.getDistance(m_centerPosition);
			if (unit->getDistance(enemyPos) > distanceFromCampLocationToEnemy && distanceFromCampLocationToEnemy <= detectRange)
			{
				m_centerPosition = unit->getPosition();
			}
		}
	}

	BWAPI::Broodwar->drawCircleMap(m_centerPosition, 10, BWAPI::Colors::Purple, true);

	if (m_centerPosition.getDistance(unit->getPosition()) >= buildRange)
	{
		std::cout << "inside\n";

		SmartUtils::SmartMove(unit, m_centerPosition);
		return;
	}

	BWAPI::Unit pylon = nullptr;
	BWAPI::Unitset units = unit->getUnitsInRadius(buildRange);
	for (BWAPI::Unit u : units)
	{
		if (!u) { continue; }

		if (u->getType() == BWAPI::UnitTypes::Protoss_Pylon)
		{
			pylon = u;
			break;
		}
	}

	if (!pylon)
	{
		BuildManager::Instance().Build(unit->getPosition(), unit, BWAPI::UnitTypes::Protoss_Pylon);
		return;
	}
	
	m_centerPosition = pylon->getPosition();

	if (InformationManager::Instance().getCountOfType(BWAPI::UnitTypes::Protoss_Forge) <= 0)
	{
		BuildManager::Instance().Build(BWAPI::UnitTypes::Protoss_Forge);
		return;
	}

	BuildManager::Instance().Build(pylon->getPosition(), unit, BWAPI::UnitTypes::Protoss_Photon_Cannon);
}

BWAPI::Unit UnitManager::getBuildUnit(BWAPI::UnitType builderType)
{
	std::vector<int> units = InformationManager::Instance().getAllUnitsOfType(builderType);

	for (int unitID : units)
	{
		BWAPI::Unit unit = BWAPI::Broodwar->getUnit(unitID);
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

void UnitManager::sendCamper()
{
	for (size_t i = 0; i < m_unitOrders.size(); i++)
	{
		if (UnitManager::isSomeoneCamping()) { break; }

		BWAPI::Unit camper = UnitManager::getBuildUnit(BWAPI::Broodwar->self()->getRace().getWorker());
		if (!camper) { return; }

		m_unitOrders[camper->getID()] = UnitOrder::CAMP;
		m_campers.insert(camper->getID());
	}
}

bool UnitManager::isSomeoneCamping()
{
	for (int unitID : InformationManager::Instance().getAllUnitsOfType(BWAPI::Broodwar->self()->getRace().getWorker()))
	{
		BWAPI::Unit unit = BWAPI::Broodwar->getUnit(unitID);

		if (!unit || !unit->exists() || !unit->isCompleted())
		{
			continue;
		}

		UnitOrder order = m_unitOrders[unit->getID()];

		if (order == UnitOrder::CAMP)
		{
			return true;
		}
	}

	return false;
}

void UnitManager::onDead(BWAPI::Unit unit)
{
	if (!unit) { return; }

	UnitOrder order = m_unitOrders[unit->getID()];

	if (order == UnitOrder::CAMP)
	{
		UnitManager::sendCamper();
	}
}

bool UnitManager::isCamper(int unitID)
{
	std::set<int>::iterator it = m_campers.find(unitID);
	if (it == m_campers.end()) {
		return false;
	}
	return true;
}

void UnitManager::rally(BWAPI::Unit unit)
{
	std::vector<int> chokePoints = InformationManager::Instance().getBases()[0].getChokePoints();
	if (chokePoints.empty()) { return; }

	BWAPI::Position chokePoint = BWAPI::Broodwar->getRegion(chokePoints[0])->getCenter();
	if (!chokePoint) { return; }

	for (BWAPI::Unit unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (!unit) { continue; }

		if (!unit->exists() || !unit->isCompleted() || unit->getType().isResourceDepot() || unit->getType().isWorker() || !unit->canSetRallyPosition()) { continue; }

		if (unit->getRallyPosition() == chokePoint) { continue; }
		unit->setRallyPoint(chokePoint);
	}
}

int UnitManager::unitsWithOrder(UnitOrder order)
{
	int returnableCount = 0;

	std::map<int, UnitOrder>::iterator it;
	for (it = m_unitOrders.begin(); it != m_unitOrders.end(); it++)
	{
		if (it->second == order)
		{
			returnableCount++;
		}
	}

	return returnableCount;
}
