#pragma once

#include <BWAPI.h>
#include <vector>
#include "BaseManager.h"

class GameManager
{

public:

	GameManager();

	void     onStart();
	void     onFrame();
};