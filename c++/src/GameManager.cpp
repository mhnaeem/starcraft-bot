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
