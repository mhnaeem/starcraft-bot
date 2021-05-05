#pragma once

#include <BWAPI.h>
#include <vector>
#include "BaseManager.h"

class GameManager
{
	void    maintainSupplyCapacity();
	void    maintainGas();

public:

	GameManager();

	void                                               onStart();
	void                                               onFrame();
	void                                               balancedStrategy();
	void                                               followStrategy(std::vector<std::pair<BWAPI::UnitType, int>> strategy);
};