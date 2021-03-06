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

    std::map<BWAPI::UnitType, int>                        m_unitCountMap;
    std::map<BWAPI::UnitType, std::vector<int>>           m_unitsMap;
    std::vector<BaseManager>                              m_enemyBases;
    std::vector<BaseManager>                              m_bases;

    int           m_usedSupply;
    int           m_totalSupply;
    int           m_mineral;
    int           m_gas;
    bool          m_camperWorking;

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

    const std::vector<BaseManager>           getBases() const;
    const std::vector<int>                   getAllUnitsOfType(BWAPI::UnitType type) const;
    const std::vector<BaseManager>           getEnemyBases() const;
    void                                     addEnemyBase(BWAPI::Position pos);
    void                                     onFrame();
    void                                     onStart();
    void                                     deductResources(BWAPI::UnitType type);
    bool                                     hasEnoughResources(BWAPI::UnitType type);
    void                                     deductResources(BWAPI::UpgradeType type);
    bool                                     hasEnoughResources(BWAPI::UpgradeType type);
    void                                     addBase(BaseManager base);
    int                                      getCountOfType(BWAPI::UnitType type);
    int                                      getUsedSupply();
    int                                      getTotalSupply(bool inProgress = false);
    int                                      getMinerals(bool inProgress = false);
    int                                      getGas(bool inProgress = false);
    void                                     setCamperWorking(bool working);
    bool                                     isCamperWorking();
};