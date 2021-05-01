#pragma once

#include "BaseManager.h"
#include <map>

class BuildManager
{
    // <building type, <unitID, oldBuildingCount>>
    std::map<BWAPI::UnitType, std::pair<int, int>>          m_buildingsInProgress;

    BuildManager();

    BWAPI::Unit   getBuildUnit(BWAPI::TilePosition buildPos, BWAPI::UnitType builderType);
    void          trackBuilds();

public:

    static BuildManager& Instance()
    {
        static BuildManager instance;
        return instance;
    }

    void        onStart();
    void        onFrame();
    bool        Build(const BaseManager* baseManager, BWAPI::UnitType type);
    bool        Build(BWAPI::Position pos, BWAPI::UnitType type);
    bool        Build(BWAPI::UnitType type);
    bool        isBuildInProgress(BWAPI::UnitType type);
};