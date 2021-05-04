#pragma once

#include <BWAPI.h>

class GameManager
{
	void    maintainSupplyCapacity();
	void    maintainGas();

public:

	GameManager();

	void                                               onStart();
	void                                               onFrame();
};