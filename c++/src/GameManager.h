#pragma once

#include <BWAPI.h>
#include <vector>
#include "BaseManager.h"

class GameManager
{
	void    maintainSupplyCapacity();
	void    maintainGas();
	void    rally();

public:

	GameManager();

	void                                               onStart();
	void                                               onFrame();
	std::vector<std::pair<BWAPI::UnitType, int>>       balancedStrategy();
	void                                               followStrategy(std::vector<std::pair<BWAPI::UnitType, int>> strategy);
};