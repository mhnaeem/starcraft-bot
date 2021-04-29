#pragma once

#include <BWAPI.h>
#include <map>
#include <vector>

enum UnitOrder
{
	COLLECT_MINERALS,
	SCOUT
};

class UnitManager
{
	// <unit id, UnitOrders>
	std::map<BWAPI::Unit, UnitOrder>      m_unitOrders;

	UnitManager();

	void    setupScouts();
	void    runOrders();
	bool    performScouting(BWAPI::Unit scout);
	bool    collectMinerals(BWAPI::Unit worker);
	void    idleWorkersCollectMinerals();

public:

	static UnitManager& Instance()
	{
		static UnitManager instance;
		return instance;
	}

	void    onFrame();
	void    onStart();

};