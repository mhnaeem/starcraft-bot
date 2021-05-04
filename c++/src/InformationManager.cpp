#pragma once

#include "InformationManager.h"
#include "MapTools.h"
#include "BaseManager.h"

InformationManager::InformationManager()
{

}

void InformationManager::onStart()
{
	m_enemyBases = std::vector<BaseManager>();
	m_bases = std::vector<BaseManager>();
	m_unitCountMap = std::map<BWAPI::UnitType, int>();
	m_unitsMap = std::map<BWAPI::UnitType, std::vector<int>>();
	InformationManager::onFrame();
}

void InformationManager::onFrame()
{
	InformationManager::parseUnitsInfo();

	m_totalSupply = InformationManager::getTotalSupply(false);
	m_usedSupply = InformationManager::getUsedSupply(false);

	m_gas = InformationManager::getGas(false);
	m_mineral = InformationManager::getMinerals(false);
}

const std::vector<BaseManager> InformationManager::getEnemyBases() const
{
	return m_enemyBases;
}

const std::vector<BaseManager> InformationManager::getBases() const
{
	return m_bases;
}

const int InformationManager::usedSupply()
{
	return m_usedSupply;
}

const int InformationManager::totalSupply()
{
	return m_totalSupply;
}

const int InformationManager::mineral()
{
	return m_mineral;
}

const int InformationManager::gas()
{
	return m_gas;
}

void InformationManager::parseUnitsInfo()
{
	m_unitsMap.clear();
	m_unitCountMap.clear();

	BWAPI::Unitset myUnits = BWAPI::Broodwar->self()->getUnits();
	for (BWAPI::Unit unit : myUnits)
	{
		if (!unit) { continue; }
		const BWAPI::UnitType type = unit->getType();
		const int unitID = unit->getID();

		try
		{
			m_unitCountMap.at(type) += 1;
			m_unitsMap[type].push_back(unitID);
		}
		catch (const std::out_of_range&)
		{
			m_unitCountMap[type] = 1;
			m_unitsMap.insert(std::pair<BWAPI::UnitType, std::vector<int>>(type, std::vector<int>()));
			m_unitsMap[type].push_back(unitID);
		}
	}
}

const std::vector<int> InformationManager::getAllUnitsOfType(BWAPI::UnitType type) const
{
	std::vector<int> returnable;

	try
	{
		returnable = m_unitsMap.at(type);

	}
	catch (const std::out_of_range&)
	{
		returnable = std::vector<int>();
	}
	return returnable;
}

int InformationManager::getCountOfType(BWAPI::UnitType type)
{
	int returnable = 0;

	try
	{
		returnable = m_unitCountMap.at(type);

	}
	catch (const std::out_of_range&)
	{
		returnable = 0;
	}
	return returnable;
}

void InformationManager::deductResources(BWAPI::UnitType type)
{
	if (type)
	{
		m_gas -= type.gasPrice();
		m_mineral -= type.mineralPrice();
		m_usedSupply += type.supplyRequired();
	}
}

bool InformationManager::hasEnoughResources(BWAPI::UnitType type)
{
	bool returnable = false;

	if (type && type.gasPrice() <= m_gas && type.mineralPrice() <= m_mineral && type.supplyRequired() <= m_usedSupply)
	{
		returnable = true;
	}
	return returnable;
}

void InformationManager::deductResources(BWAPI::UpgradeType type)
{
	if (type)
	{
		m_gas -= type.gasPrice();
		m_mineral -= type.mineralPrice();
	}
}

bool InformationManager::hasEnoughResources(BWAPI::UpgradeType type)
{
	bool returnable = false;

	if (type && type.gasPrice() <= m_gas && type.mineralPrice() <= m_mineral)
	{
		returnable = true;
	}
	return returnable;
}

int InformationManager::getMinerals(bool inProgress)
{
	int minerals = BWAPI::Broodwar->self()->minerals();

	if (!inProgress) { return minerals; }

	for (BWAPI::Unit unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (!unit || !unit->exists() || !unit->isCompleted()) { continue; }

		const BWAPI::UnitCommand command = unit->getLastCommand();
		if (
			command.getType() == BWAPI::UnitCommandTypes::Build ||
			command.getType() == BWAPI::UnitCommandTypes::Build_Addon ||
			command.getType() == BWAPI::UnitCommandTypes::Train ||
			command.getType() == BWAPI::UnitCommandTypes::Upgrade ||
			command.getType() == BWAPI::UnitCommandTypes::Research
			)
		{
			minerals -= command.getUnitType().mineralPrice();
		}

	}

	return minerals;
}

int InformationManager::getGas(bool inProgress)
{
	int gas = BWAPI::Broodwar->self()->gas();

	if (!inProgress) { return gas; }

	for (BWAPI::Unit unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (!unit || !unit->exists() || !unit->isCompleted()) { continue; }

		const BWAPI::UnitCommand command = unit->getLastCommand();
		if (
			command.getType() == BWAPI::UnitCommandTypes::Build ||
			command.getType() == BWAPI::UnitCommandTypes::Build_Addon ||
			command.getType() == BWAPI::UnitCommandTypes::Train ||
			command.getType() == BWAPI::UnitCommandTypes::Upgrade ||
			command.getType() == BWAPI::UnitCommandTypes::Research
		   )
		{
			gas -= command.getUnitType().gasPrice();
		}

	}

	return gas;
}

int InformationManager::getUsedSupply(bool inProgress)
{
	int usedSupply = BWAPI::Broodwar->self()->supplyUsed();

	if (!inProgress) { return usedSupply / 2; }

	for (BWAPI::Unit unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (!unit || !unit->exists() || !unit->isCompleted()) { continue; }

		const BWAPI::UnitCommand command = unit->getLastCommand();

		if (command.getType() != BWAPI::UnitCommandTypes::Train) { continue; }

		usedSupply += command.getUnitType().supplyRequired();
	}

	return usedSupply / 2;
}

int InformationManager::getTotalSupply(bool inProgress)
{
	// start the calculation by looking at our current completed supply
	int totalSupply = BWAPI::Broodwar->self()->supplyTotal();

	// if we don't want to calculate the supply in progress, just return that value
	if (!inProgress) { return totalSupply / 2; }

	// if we do care about supply in progress, check all the currently constructing units if they will add supply
	for (BWAPI::Unit unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (!unit) { continue; }

		// ignore units that are fully completed
		if (unit->isCompleted()) { continue; }

		// if they are not completed, then add their supply provided to the total supply
		totalSupply += unit->getType().supplyProvided();
	}

	// one last tricky case: if a unit is currently on its way to build a supply provider, add it
	for (BWAPI::Unit unit : BWAPI::Broodwar->self()->getUnits())
	{

		if (!unit) { return false; }

		// get the last command given to the unit
		const BWAPI::UnitCommand command = unit->getLastCommand();

		// if it's not a build command we can ignore it
		if (command.getType() != BWAPI::UnitCommandTypes::Build) { continue; }

		// add the supply amount of the unit that it's trying to build
		totalSupply += command.getUnitType().supplyProvided();
	}

	return totalSupply / 2;
}

void InformationManager::addEnemyBase(BWAPI::Position pos)
{
	if (pos && MapTools::Instance().isValidPosition(pos))
	{
		BaseManager enemy(pos);
		m_enemyBases.push_back(pos);
	}
}

void InformationManager::addBase(BaseManager base)
{
	m_bases.push_back(base);
}