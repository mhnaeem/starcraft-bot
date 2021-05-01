#pragma once

#include "Grid.hpp"
#include <BWAPI.h>
#include <map>
#include <vector>
#include "BaseManager.h"

class InformationManager
{
    InformationManager();

    void parseUnitsInfo();
    int  getUsedSupply(bool inProgress = false);
    int  getTotalSupply(bool inProgress = false);
    int  getMinerals(bool inProgress = false);
    int  getGas(bool inProgress = false);

    std::map<BWAPI::UnitType, int>                        m_unitCountMap;
    std::map<BWAPI::UnitType, std::vector<BWAPI::Unit>>   m_unitsMap;
    BWAPI::Player                                         m_player = BWAPI::Broodwar->self();
    std::vector<BaseManager>                              m_enemyBases;
    std::vector<BaseManager>                              m_bases;

    int           m_usedSupply;
    int           m_totalSupply;
    int           m_mineral;
    int           m_gas;

public:

    static InformationManager& Instance()
    {
        static InformationManager instance;
        return instance;
    }

    const int usedSupply();
    const int totalSupply();
    const int mineral();
    const int gas();

    const std::vector<BaseManager>&          getBases() const;
    const std::map<BWAPI::UnitType, int>&    getUnitCountMap() const;
    const std::vector<BWAPI::Unit>           getAllUnitsOfType(BWAPI::UnitType type) const;
    const std::vector<BaseManager>&          getEnemyBases() const;
    void                                     addEnemyBase(BWAPI::Position pos);
    void                                     onFrame();
    void                                     onStart();
    void                                     deductResources(BWAPI::UnitType type);
    bool                                     hasEnoughResources(BWAPI::UnitType type);
    void                                     deductResources(BWAPI::UpgradeType type);
    bool                                     hasEnoughResources(BWAPI::UpgradeType type);
    void                                     addBase(BaseManager base);
};