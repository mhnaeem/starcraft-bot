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
	m_enroute = std::map<BWAPI::TilePosition, int>();
}

void UnitManager::onStart()
{
	m_unitOrders.clear();
	m_campers.clear();
	m_enroute.clear();
	m_scoutConfusionAngle = 30.0;
	m_cornerIndex = 0;
	setupScouts();
}

void UnitManager::onFrame()
{
	idleWorkersCollectMinerals();
	replenishCampers();
	setRallyUnits();
	runOrders();
	//printInfo();
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
		case UnitOrder::CAMP_MOVE:
			camp(unit, true);
			break;
		default:
			break;
		}
	}
}

void UnitManager::idleWorkersCollectMinerals()
{
	int gasCollectors = UnitManager::unitsWithOrder(UnitOrder::COLLECT_GAS);
	int maxGasCollectors = 2;
	if (InformationManager::Instance().getGas() >= 600)
	{
		maxGasCollectors--;
	}

	std::vector<int> workers = InformationManager::Instance().getAllUnitsOfType(BWAPI::Broodwar->self()->getRace().getWorker());
	for (int workerID : workers)
	{
		BWAPI::Unit worker = BWAPI::Broodwar->getUnit(workerID);

		if (!worker || !worker->exists() || !worker->isCompleted()) { continue; }
		std::map<int, UnitOrder>::iterator it = m_unitOrders.find(worker->getID());
		if (it == m_unitOrders.end()) {
			// if not in list then make them collect minerals
			m_unitOrders[worker->getID()] = UnitOrder::COLLECT_MINERALS;
			continue;
		}

		UnitOrder command = m_unitOrders[worker->getID()];
		if (worker->isIdle() && command != UnitOrder::SCOUT && command != UnitOrder::SCOUT_CONFUSION_MICRO && command != UnitOrder::CAMP)
		{
			m_unitOrders[worker->getID()] = UnitOrder::COLLECT_MINERALS;
		}
	}

	if (InformationManager::Instance().getCountOfType(BWAPI::Broodwar->self()->getRace().getRefinery()) == 0) { return; }
	
	for (int workerID : workers)
	{
		BWAPI::Unit worker = BWAPI::Broodwar->getUnit(workerID);
		if (!worker || !worker->exists() || !worker->isCompleted()) { return; }

		if (gasCollectors < maxGasCollectors)
		{
			if (m_unitOrders[worker->getID()] == UnitOrder::COLLECT_MINERALS)
			{
				m_unitOrders[worker->getID()] = UnitOrder::COLLECT_GAS;
				gasCollectors++;
			}
		}
		else if (gasCollectors > maxGasCollectors)
		{
			if (m_unitOrders[worker->getID()] == UnitOrder::COLLECT_GAS)
			{
				m_unitOrders[worker->getID()] = UnitOrder::COLLECT_MINERALS;
				gasCollectors--;
			}
		}

		if (gasCollectors == maxGasCollectors)
		{
			break;
		}
	}

}

void UnitManager::setupScouts()
{
	int scoutCount = 0;
	const std::vector<int>& possibleScouts = InformationManager::Instance().getAllUnitsOfType(BWAPI::Broodwar->self()->getRace().getWorker());
	for (int unitID : possibleScouts)
	{
		BWAPI::Unit unit = BWAPI::Broodwar->getUnit(unitID);
		if (unit && unit->exists() && unit->isCompleted())
		{
			m_unitOrders[unitID] = UnitOrder::SCOUT;
			scoutCount++;
		}

		if (scoutCount == ( BWAPI::Broodwar->getStartLocations().size() -1 ))
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

	auto const getNextPosition = [&]()
	{
		auto rv = m_cornerLocations[m_cornerIndex];
		m_cornerIndex = (m_cornerIndex + 1) % m_cornerLocations.size();
		auto enemyCenterLocation = enemyCenterLocations[0].getLocation();
		enemyCenterLocation.x += rv.first;
		enemyCenterLocation.y += rv.second;
		return enemyCenterLocation;
	};

	auto const getNewPos = [&](double angle, int radius) {
		double radians = angle * 3.14 / 180;
		int newX = enemyCenterLocations[0].getLocation().x + (std::cos(radians) * radius);
		int newY = enemyCenterLocations[0].getLocation().y + (std::sin(radians) * radius);

		return BWAPI::Position(newX, newY);
	};

	if (scout && scout->exists())
	{
		auto const areEnemyWorkersFollowing = [&]()
		{
			auto enemyWorkers = scout->getUnitsInRadius(2048, BWAPI::Filter::Exists && BWAPI::Filter::IsEnemy && BWAPI::Filter::IsWorker);
			int workersMiningMinerals = 0;
			for (auto worker : enemyWorkers)
			{
				if (worker->getOrder() != BWAPI::Orders::AttackUnit) 
				{
					workersMiningMinerals += 1;
				}
			}

			if (enemyWorkers.size() > 0 && (workersMiningMinerals / enemyWorkers.size()) > 0.8)
			{
				return false;
			}
			return true;
		};
		const BWAPI::Unit enemyBase = scout->getUnitsInRadius(1024, BWAPI::Filter::Exists && BWAPI::Filter::IsEnemy && BWAPI::Filter::IsResourceDepot).getClosestUnit();
		
		if (!areEnemyWorkersFollowing())
		{
			SmartUtils::SmartAttack(scout, enemyBase);
		}
		else if (!scout->isMoving())
		{
			SmartUtils::SmartMove(scout, getNextPosition());
		}
	}


}

bool UnitManager::performScouting(BWAPI::Unit scout)
{
	if (!scout || !scout->exists() || !scout->isCompleted())
	{
		return false;
	}

	if (!InformationManager::Instance().getEnemyBases().empty())
	{
		m_unitOrders[scout->getID()] = UnitOrder::COLLECT_MINERALS;
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


	for (BWAPI::TilePosition pos : BWAPI::Broodwar->getStartLocations())
	{
		std::map<BWAPI::TilePosition, int>::iterator it = m_enroute.find(pos);
		if (it != m_enroute.end() && it->second != scout->getID()) {
			continue;
		}

		if (pos && !MapTools::Instance().isExplored(pos))
		{
			bool moved = SmartUtils::SmartMove(scout, pos);
			m_enroute[pos] = scout->getID();
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

	for (auto unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (!unit) { continue; }

		if (!unit->isCompleted() || !unit->exists()) { continue; }

		if (unit->getType().isWorker() || unit->getType().isBuilding()) { continue; }

		auto attackers = SmartUtils::SmartDetectEnemy(unit);
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

void UnitManager::camp(BWAPI::Unit unit, bool move)
{

	auto useFallbackPosition = [&]()
	{
		std::vector<BaseManager> enemyBases = InformationManager::Instance().getEnemyBases();
		if (enemyBases.empty()) { return; }

		m_centerPosition = enemyBases[0].getLocation();
	};

	if (!unit || !unit->exists() || !unit->isCompleted()) { return; }

	if (move)
	{
		SmartUtils::SmartStop(unit);
		m_unitOrders[unit->getID()] = UnitOrder::CAMP;
		return;
	}


	if (m_centerPosition == BWAPI::Positions::Invalid)
	{
		std::vector<BaseManager> enemyBases = InformationManager::Instance().getEnemyBases();
		if (enemyBases.empty())
		{
			useFallbackPosition();
			return;
		}

		std::vector<int> enemyChokepoints = enemyBases[0].getChokePoints();
		if (enemyChokepoints.empty())
		{
			useFallbackPosition();
			return;
		}

		BWAPI::Region enemyChokepoint = BWAPI::Broodwar->getRegion(enemyChokepoints[0]);
		if (!enemyChokepoint)
		{
			useFallbackPosition();
			return;
		}

		m_centerPosition = enemyChokepoint->getCenter();
		if (!m_centerPosition)
		{
			useFallbackPosition();
			return;
		}
	}

	BWAPI::Unitset enemyUnits = SmartUtils::SmartDetectEnemy(300, unit);
	if (!enemyUnits.empty())
	{
		BWAPI::Unit enemy = enemyUnits.getClosestUnit();
		if (enemy && enemy->exists() && unit->getDistance(enemy->getPosition()) > enemy->getDistance(m_centerPosition))
		{
			m_centerPosition = unit->getPosition();
		}
	}

	if (m_centerPosition.getDistance(unit->getPosition()) >= 300)
	{
		SmartUtils::SmartMove(unit, m_centerPosition);
		return;
	}

	BWAPI::Unit pylon = nullptr;
	BWAPI::Unitset units = unit->getUnitsInRadius(300);
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

	if (InformationManager::Instance().getCountOfType(BWAPI::UnitTypes::Protoss_Forge) == 0)
	{
		BuildManager::Instance().Build(BWAPI::UnitTypes::Protoss_Forge);
		return;
	}

	int numOfCannons = InformationManager::Instance().getCountOfType(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	int numOfPylons = InformationManager::Instance().getCountOfType(BWAPI::UnitTypes::Protoss_Pylon);
	int ans = (numOfCannons / numOfPylons) % 5;
	if (numOfCannons > 0 && ans == 1)
	{
		BuildManager::Instance().Build(pylon->getPosition(), unit, BWAPI::UnitTypes::Protoss_Pylon);
		return;
	}

	BuildManager::Instance().Build(pylon->getPosition(), unit, BWAPI::UnitTypes::Protoss_Photon_Cannon);
}


BWAPI::Unit UnitManager::getBuildUnit(BWAPI::UnitType builderType, UnitOrder order)
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
		if (currentOrder == order)
		{
			return unit;
		}
	}

	return nullptr;
}

void UnitManager::sendCamper()
{
	UnitOrder order = UnitOrder::COLLECT_MINERALS;
	BWAPI::UnitType worker = BWAPI::Broodwar->self()->getRace().getWorker();

	if (UnitManager::unitsWithOrder(UnitOrder::SCOUT) >= 1)
	{
		order = UnitOrder::SCOUT;
	}

	for (size_t i = 0; i < InformationManager::Instance().getCountOfType(worker); i++)
	{
		if (i == 1)
		{
			order = UnitOrder::COLLECT_MINERALS;
		}

		BWAPI::Unit camper = UnitManager::getBuildUnit(worker, order);
		if (!camper || !camper->exists() || !camper->isCompleted())
		{
			continue;
		}

		m_unitOrders[camper->getID()] = UnitOrder::CAMP;
		m_campers.insert(camper->getID());
		break;
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

	int unitID = unit->getID();
	UnitOrder order = m_unitOrders[unitID];
	m_campers.erase(unitID);
	m_unitOrders.erase(unitID);
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

void UnitManager::printInfo()
{
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
		default:
			break;
		}

		std::cout << u->getID() << " " << u->getLastCommand().getType().getName() << " " << u->getLastCommand().getUnitType().getName() << " " << x << "\n";
	}
}

void UnitManager::rally(BWAPI::Unit unit)
{
	if (!unit || !unit->exists() || !unit->isCompleted() || unit->getType().isResourceDepot() || unit->getType().isWorker() || !unit->canSetRallyPosition()) { return; }

	std::vector<BaseManager> bases = InformationManager::Instance().getBases();
	if (bases.empty()) { return; }

	std::vector<int> chokePoints = bases[0].getChokePoints();
	if (chokePoints.empty()) { return; }

	BWAPI::Position chokePoint = BWAPI::Broodwar->getRegion(chokePoints[0])->getCenter();
	if (!chokePoint) { return; }

	if (unit->getRallyPosition() == chokePoint) { return; }

	unit->setRallyPoint(chokePoint);
}

void UnitManager::onCreate(BWAPI::Unit unit)
{
}

void UnitManager::replenishCampers()
{
	if (InformationManager::Instance().getEnemyBases().empty()) { return; }

	if (m_campers.empty())
	{
		UnitManager::sendCamper();
		return;
	}

	std::vector<int> toRemove;

	int numOfCampers = 0;
	for (int camperID : m_campers)
	{
		BWAPI::Unit camper = BWAPI::Broodwar->getUnit(camperID);
		if (!camper || !camper->exists() || camper->isStuck() || camper->isGatheringMinerals() || camper->isGatheringGas() || camper->getLastCommand().getType() == BWAPI::UnitCommandTypes::Right_Click_Unit)
		{
			toRemove.push_back(camperID);
			m_unitOrders[camperID] = UnitOrder::COLLECT_MINERALS;
			continue;
		}

		numOfCampers++;
	}

	for (int rem : toRemove)
	{
		m_campers.erase(rem);
	}

	if (numOfCampers < 1)
	{
		UnitManager::sendCamper();
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

void UnitManager::setRallyUnits()
{
	std::vector<BaseManager> bases = InformationManager::Instance().getBases();
	if (bases.empty()) { return; }

	std::vector<int> chokePoints = bases[0].getChokePoints();
	if (chokePoints.empty()) { return; }

	BWAPI::Position chokePoint = BWAPI::Broodwar->getRegion(chokePoints[0])->getCenter();
	if (!chokePoint) { return; }

	std::vector<int> gateways = InformationManager::Instance().getAllUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway);
	if (gateways.empty()) { return; }

	for (int unitID : gateways)
	{
		BWAPI::Unit gateway = BWAPI::Broodwar->getUnit(unitID);

		if (!gateway || !gateway->exists() || !gateway->isCompleted()) { continue; }
		
		if(!gateway->canSetRallyPosition()) { continue; }

		if (gateway->getRallyPosition() == chokePoint) { continue; }

		gateway->setRallyPoint(chokePoint);
	}
}
