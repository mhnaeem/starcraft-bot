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
    int  getUsedSupply();
    int  getTotalSupply(bool inProgress = false);

    std::map<BWAPI::UnitType, int>                        m_unitCountMap;
    std::map<BWAPI::UnitType, std::vector<BWAPI::Unit>>   m_unitsMap;
    BWAPI::Player                                         m_player = BWAPI::Broodwar->self();
    std::vector<BWAPI::Position>                          m_enemyPositions;
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
    const std::vector<BWAPI::Position>&      getEnemyLocations() const;
    void                                     addEnemyPosition(BWAPI::Position pos);
    void                                     onFrame();
    void                                     onStart();
    void                                     deductResources(BWAPI::UnitType type);
    bool                                     hasEnoughResources(BWAPI::UnitType type);
    void                                     deductResources(BWAPI::UpgradeType type);
    bool                                     hasEnoughResources(BWAPI::UpgradeType type);
    void                                     addBase(BaseManager base);
};