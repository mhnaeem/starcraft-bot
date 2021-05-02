#pragma once

#include "GameManager.h"
#include "UnitManager.h"
#include "BuildManager.h"
#include "InformationManager.h"

GameManager::GameManager()
{

}

void GameManager::onStart()
{
    BWAPI::Unit resourceDepot = nullptr;
    for (auto unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit && unit->getType().isResourceDepot())
        {
            resourceDepot = unit;
            break;
        }
    }
    if (resourceDepot)
    {
        BaseManager originalBase(resourceDepot->getPosition());
        InformationManager::Instance().addBase(originalBase);
    }

    BuildManager::Instance().onStart();
    UnitManager::Instance().onStart();
}

void GameManager::onFrame()
{
    BuildManager::Instance().onFrame();
    for (auto base : InformationManager::Instance().getBases())
    {
        base.onFrame();
    }
    GameManager::maintainSupplyCapacity();
    GameManager::maintainGas();
    GameManager::followStrategy(GameManager::balancedStrategy());
    UnitManager::Instance().onFrame();
}

void GameManager::maintainSupplyCapacity()
{
    const int totalSupply = InformationManager::Instance().totalSupply();
    const int usedSupply = InformationManager::Instance().usedSupply();
    const int unusedSupply = totalSupply - usedSupply;

    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
    const int numOfSupplyProviders = InformationManager::Instance().getAllUnitsOfType(supplyProviderType).size();

    if (unusedSupply > 2 && numOfSupplyProviders != 0) { return; }

    BuildManager::Instance().Build(supplyProviderType);
}

void GameManager::maintainGas()
{
    const int totalSupply = InformationManager::Instance().totalSupply();

    const BWAPI::UnitType refineryType = BWAPI::Broodwar->self()->getRace().getRefinery();
    const int numOfRefineries = InformationManager::Instance().getAllUnitsOfType(refineryType).size();

    if (totalSupply <= 25 || numOfRefineries != 0) { return; }

    BWAPI::Unit geyser = BWAPI::Broodwar->getGeysers().getClosestUnit();
    if (!geyser) { return; }

    BuildManager::Instance().Build(geyser->getPosition(), refineryType);
}

void GameManager::followStrategy(std::vector<std::pair<BWAPI::UnitType, int>> strategy)
{
    if (strategy.empty()) { return; }

    std::vector<BaseManager> bases = InformationManager::Instance().getBases();

    if (bases.empty()) { return; }

    BWAPI::Position pos = bases[0].getLocation();

    auto buildWhatYouCan = [&](BWAPI::UnitType type)
    {
        auto needed = BuildManager::Instance().BuildingsNeeded(type);
        for (auto need : needed)
        {
            BuildManager::Instance().Build(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), need);
        }
    };

    for (auto build : strategy)
    {
        BWAPI::UnitType type = build.first;
        if (InformationManager::Instance().getAllUnitsOfType(type).size() <= build.second)
        {
            buildWhatYouCan(type);
            if (!type.isBuilding())
            {
                SmartUtils::SmartTrain(type);
            }
        }
    }
}

std::vector<std::pair<BWAPI::UnitType, int>> GameManager::balancedStrategy()
{
    std::vector<std::pair<BWAPI::UnitType, int>> strat = std::vector<std::pair<BWAPI::UnitType, int>>();
    
    auto add = [&](BWAPI::UnitType type, int count)
    {
        strat.push_back(std::pair<BWAPI::UnitType, int>(type, count));
    };

    add(BWAPI::UnitTypes::Protoss_Zealot, 10);
    add(BWAPI::UnitTypes::Protoss_Photon_Cannon, 10);
    add(BWAPI::UnitTypes::Protoss_Dragoon, 10);

    return strat;
}
