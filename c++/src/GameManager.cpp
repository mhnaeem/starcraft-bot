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
    for (BWAPI::Unit unit : BWAPI::Broodwar->self()->getUnits())
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
    for (BaseManager base : InformationManager::Instance().getBases())
    {
        base.onFrame();
    }
    UnitManager::Instance().onFrame();
    GameManager::maintainSupplyCapacity();
    GameManager::maintainGas();
    GameManager::balancedStrategy();
}

void GameManager::maintainSupplyCapacity()
{
    const int totalSupply = InformationManager::Instance().getTotalSupply(true);
    const int usedSupply = InformationManager::Instance().usedSupply();
    const int unusedSupply = totalSupply - usedSupply;

    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
    const int numOfSupplyProviders = InformationManager::Instance().getCountOfType(supplyProviderType);

    if (unusedSupply > 2 && numOfSupplyProviders != 0) { return; }

    BuildManager::Instance().Build(supplyProviderType);
}

void GameManager::maintainGas()
{
    const int totalSupply = InformationManager::Instance().totalSupply();

    const BWAPI::UnitType refineryType = BWAPI::Broodwar->self()->getRace().getRefinery();
    const int numOfRefineries = InformationManager::Instance().getCountOfType(refineryType);

    if (totalSupply <= 40 || numOfRefineries != 0) { return; }

    BWAPI::Unit geyser = SmartUtils::GetClosestUnitTo(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), BWAPI::Broodwar->getGeysers());
    if (!geyser) { return; }

    BuildManager::Instance().Build(geyser->getPosition(), refineryType);
}

void GameManager::followStrategy(std::vector<std::pair<BWAPI::UnitType, int>> strategy)
{
    bool cp = true;
    BWAPI::Position chokePoint;
    if (strategy.empty()) { return; }
    std::vector<int> chokePoints = InformationManager::Instance().getBases()[0].getChokePoints();
    if (chokePoints.empty()) { cp = false; }

    if (cp)
    {
        chokePoint = BWAPI::Broodwar->getRegion(chokePoints[0])->getCenter();
        if (!chokePoint) { cp = false; }
    }

    auto buildWhatYouCan = [&](BWAPI::UnitType type)
    {
        auto needed = BuildManager::Instance().BuildingsNeeded(type);
        for (BWAPI::UnitType need : needed)
        {
            BWAPI::Position pos = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

            if (cp && chokePoint)
            {
                if (BWAPI::UnitTypes::Protoss_Pylon == need || BWAPI::UnitTypes::Protoss_Photon_Cannon == need)
                {
                    pos = chokePoint;
                }
            }
            BuildManager::Instance().Build(pos, need);
        }
    };

    int done = 0;

    for (auto build : strategy)
    {
        BWAPI::UnitType type = build.first;
        if (InformationManager::Instance().getCountOfType(type) < build.second)
        {
            buildWhatYouCan(type);
            if (!type.isBuilding())
            {
                SmartUtils::SmartTrain(type);
            }
        }
        else
        {
            done++;
        }
    }

    if (done == strategy.size())
    {
        UnitManager::Instance().attack();
    }
}

void GameManager::balancedStrategy()
{
    std::vector<std::pair<BWAPI::UnitType, int>> strat = std::vector<std::pair<BWAPI::UnitType, int>>();
    
    auto add = [&](BWAPI::UnitType type, int count)
    {
        strat.push_back(std::pair<BWAPI::UnitType, int>(type, count));
    };

    add(BWAPI::UnitTypes::Protoss_Pylon, 1);
    add(BWAPI::UnitTypes::Protoss_Forge, 1);
    add(BWAPI::UnitTypes::Protoss_Gateway, 1);
    add(BWAPI::UnitTypes::Protoss_Zealot, 20);

    GameManager::followStrategy(strat);
}
