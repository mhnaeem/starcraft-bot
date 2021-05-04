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

    if (totalSupply <= 30 || numOfRefineries != 0) { return; }

    BWAPI::Unit geyser = SmartUtils::GetClosestUnitTo(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), BWAPI::Broodwar->getGeysers());
    if (!geyser) { return; }

    BuildManager::Instance().Build(geyser->getPosition(), refineryType);
}
