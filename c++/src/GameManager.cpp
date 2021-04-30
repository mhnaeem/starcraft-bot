#pragma once

#include "GameManager.h"
#include "UnitManager.h"
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

    UnitManager::Instance().onStart();
}

void GameManager::onFrame()
{
    for (auto base : InformationManager::Instance().getBases())
    {
        base.onFrame();
    }
    UnitManager::Instance().onFrame();
}