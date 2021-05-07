#pragma once

#include "BaseManager.h"
#include <map>

class BuildManager
{
    // <building type, unitID>
    std::map<BWAPI::UnitType, int>          m_buildingsInProgress;

    BuildManager();

    void          trackBuilds();

public:

    static BuildManager& Instance()
    {
        static BuildManager instance;
        return instance;
    }

    void                           onStart();
    void                           onFrame();
    void                           onCreate(BWAPI::Unit unit);
    void                           purgeCamper();
    bool                           Build(BaseManager* baseManager, BWAPI::UnitType type);
    bool                           Build(BWAPI::Position pos, BWAPI::UnitType type);
    bool                           Build(BWAPI::Position pos, BWAPI::Unit builder, BWAPI::UnitType type);
    bool                           Build(BWAPI::UnitType type);
    bool                           isBuildInProgress(BWAPI::UnitType type);
    std::set<BWAPI::UnitType>      BuildingsNeeded(BWAPI::UnitType building);
};