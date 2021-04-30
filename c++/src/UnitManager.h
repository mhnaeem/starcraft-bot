#pragma once

#include <BWAPI.h>
#include <map>
#include <vector>

enum UnitOrder
{
	COLLECT_MINERALS,
	SCOUT_CONFUSION_MICRO,
	SCOUT
};

class UnitManager
{
	// <unit id, UnitOrders>
	std::map<int, UnitOrder>		      m_unitOrders;
	double								  m_scoutConfusionAngle = 30.0;

	UnitManager();

	void    setupScouts();
	void    runOrders();
	bool    performScouting(BWAPI::Unit scout);
	void    performScoutConfusionMicro(BWAPI::Unit scout);
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