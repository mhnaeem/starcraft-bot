#include "InformationManager.h"

InformationManager::InformationManager()
{
	InformationManager::onFrame();
}

void InformationManager::onFrame()
{

	InformationManager::parseUnitsInfo();

	m_totalSupply = InformationManager::getTotalSupply(true);
	m_usedSupply = InformationManager::getUsedSupply();

	m_gas = m_player->gas();
	m_mineral = m_player->minerals();
}

const std::map<BWAPI::UnitType, int>& InformationManager::getUnitCountMap() const
{
	return m_unitCountMap;
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

	BWAPI::Unitset myUnits = m_player->getUnits();
	for (auto& unit : myUnits)
	{
		if (!unit || !unit->exists()) { continue; }

		const BWAPI::UnitType type = unit->getType();

		try
		{
			m_unitCountMap.at(type) += 1;
			m_unitsMap[type].push_back(unit);
		}
		catch (const std::out_of_range&)
		{
			m_unitCountMap[type] = 1;
			m_unitsMap.insert(std::pair<BWAPI::UnitType, std::vector<BWAPI::Unit>>(type, std::vector<BWAPI::Unit>()));
			m_unitsMap[type].push_back(unit);
		}
	}
}

const std::vector<BWAPI::Unit> InformationManager::getAllUnitsOfType(BWAPI::UnitType type) const
{
	std::vector<BWAPI::Unit> returnable;

	try
	{
		returnable = m_unitsMap.at(type);

	}
	catch (const std::out_of_range&)
	{
		returnable = std::vector<BWAPI::Unit>();
	}
	return returnable;
}

void InformationManager::deductResources(BWAPI::UnitType type)
{
	if (type)
	{
		m_gas -= type.gasPrice();
		m_mineral -= type.mineralPrice();
	}
}

bool InformationManager::hasEnoughResources(BWAPI::UnitType type)
{
	bool returnable = false;

	if (type && type.gasPrice() <= m_gas && type.mineralPrice() <= m_mineral)
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

int InformationManager::getUsedSupply()
{
	return BWAPI::Broodwar->self()->supplyUsed() / 2;
}

int InformationManager::getTotalSupply(bool inProgress)
{
	// start the calculation by looking at our current completed supply
	int totalSupply = BWAPI::Broodwar->self()->supplyTotal();

	// if we don't want to calculate the supply in progress, just return that value
	if (!inProgress) { return totalSupply / 2; }

	// if we do care about supply in progress, check all the currently constructing units if they will add supply
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (!unit) { continue; }

		// ignore units that are fully completed
		if (unit->isCompleted()) { continue; }

		// if they are not completed, then add their supply provided to the total supply
		totalSupply += unit->getType().supplyProvided();
	}

	// one last tricky case: if a unit is currently on its way to build a supply provider, add it
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{

		if (!unit) { return false; }

		// get the last command given to the unit
		const BWAPI::UnitCommand& command = unit->getLastCommand();

		// if it's not a build command we can ignore it
		if (command.getType() != BWAPI::UnitCommandTypes::Build) { continue; }

		// add the supply amount of the unit that it's trying to build
		totalSupply += command.getUnitType().supplyProvided();
	}

	return totalSupply / 2;
}
