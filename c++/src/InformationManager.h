#pragma once

#include "Grid.hpp"
#include <BWAPI.h>
#include <map>
#include <vector>

class InformationManager
{
    InformationManager();

    void parseUnitsInfo();
    int  getUsedSupply();
    int  getTotalSupply(bool inProgress = false);

    std::map<BWAPI::UnitType, int>                        m_unitCountMap;
    std::map<BWAPI::UnitType, std::vector<BWAPI::Unit>>   m_unitsMap;
    BWAPI::Player                                         m_player = BWAPI::Broodwar->self();

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

    const std::map<BWAPI::UnitType, int>&    getUnitCountMap() const;
    const std::vector<BWAPI::Unit>           getAllUnitsOfType(BWAPI::UnitType type) const;
    void                                     onFrame();
    void                                     deductResources(BWAPI::UnitType type);
    bool                                     hasEnoughResources(BWAPI::UnitType type);
    void                                     deductResources(BWAPI::UpgradeType type);
    bool                                     hasEnoughResources(BWAPI::UpgradeType type);
};