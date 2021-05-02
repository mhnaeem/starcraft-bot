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
    GameManager::rally();
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
    bool cp = true;
    BWAPI::Position chokePoint;
    if (strategy.empty()) { return; }
    auto chokePoints = InformationManager::Instance().getBases()[0].getChokePoints();
    if (chokePoints.empty()) { cp = false; }

    if (cp)
    {
        chokePoint = chokePoints[0]->getCenter();
        if (!chokePoint) { cp = false; }
    }

    auto buildWhatYouCan = [&](BWAPI::UnitType type)
    {
        auto needed = BuildManager::Instance().BuildingsNeeded(type);
        for (auto need : needed)
        {
            auto pos = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

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
        if (InformationManager::Instance().getAllUnitsOfType(type).size() < build.second)
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

void GameManager::rally()
{
    auto chokePoints = InformationManager::Instance().getBases()[0].getChokePoints();
    if (chokePoints.empty()) { return; }

    auto chokePoint = chokePoints[0]->getCenter();
    if (!chokePoint) { return; }

    for (auto unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (!unit) { continue; }

        if (!unit->exists() || !unit->isCompleted() || unit->getType().isResourceDepot() || unit->getType().isWorker() || !unit->canSetRallyPosition()) { continue; }

        if (unit->getRallyPosition() == chokePoint) { continue; }
        unit->setRallyPoint(chokePoint);
    }
}