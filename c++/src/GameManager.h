#pragma once

#include <BWAPI.h>
#include <vector>
#include "BaseManager.h"

class GameManager
{
	void    maintainSupplyCapacity();

public:

	GameManager();

	void     onStart();
	void     onFrame();
};