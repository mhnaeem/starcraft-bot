#pragma once

#include <BWAPI.h>
#include <vector>
#include "BaseManager.h"

class GameManager
{
	std::vector<BaseManager>     m_bases;

public:

	GameManager();

	void     onStart();
	void     onFrame();
};