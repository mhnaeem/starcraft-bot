#pragma once

#include "GameManager.h"

GameManager::GameManager()
{
	m_bases = std::vector<BaseManager>();
}

void GameManager::onStart()
{
    m_bases.clear();

    BWAPI::Unit resourceDepot = nullptr;
    for (auto unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit && unit->getType() == BWAPI::Broodwar->self()->getRace().getResourceDepot())
        {
            resourceDepot = unit;
        }
    }
    if (resourceDepot)
    {
        BaseManager originalBase(resourceDepot->getPosition());
        m_bases.push_back(originalBase);
    }
}

void GameManager::onFrame()
{
    for (auto& base : m_bases)
    {
        base.onFrame();
    }
}