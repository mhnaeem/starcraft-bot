#pragma once

#include <BWAPI.h>

namespace DrawTools
{
    void DrawUnitBoundingBoxes();
    void DrawUnitCommands();
    void DrawUnitHealthBars();
    void DrawHealthBar(BWAPI::Unit unit, double ratio, BWAPI::Color color, int yOffset);
    void DrawAllRegions();
    void DrawCircleAroundStart();
}