#pragma once

#include <BWAPI.h>
#include <map>
#include <vector>
#include <set>

enum class UnitOrder
{
	COLLECT_MINERALS,
	COLLECT_GAS,
	SCOUT_CONFUSION_MICRO,
	BUILD,
	CAMP,
	CAMP_MOVE,
	SCOUT,
	RALLY
};

class UnitManager
{
	// <unit id, UnitOrders>
	std::map<int, UnitOrder>		      m_unitOrders;
	std::set<int>                         m_campers;
	double								  m_scoutConfusionAngle = 30.0;
	BWAPI::Position                       m_centerPosition = BWAPI::Positions::Invalid;

	UnitManager();

	void    setupScouts();
	void    runOrders();
	bool    performScouting(BWAPI::Unit scout);
	void    performScoutConfusionMicro(BWAPI::Unit scout);
	bool    collectMinerals(BWAPI::Unit worker);
	bool    collectGas(BWAPI::Unit worker);
	void    idleWorkersCollectMinerals();
	void    camp(BWAPI::Unit unit, bool move = false);
	void    rally(BWAPI::Unit unit);
	void    sendCamper();
	bool    isSomeoneCamping();
	void    printInfo();
	void    replenishCampers();
	int     unitsWithOrder(UnitOrder order);

public:

	static UnitManager& Instance()
	{
		static UnitManager instance;
		return instance;
	}

	void          attack();
	void          onFrame();
	void          onStart();
	void          onCreate(BWAPI::Unit unit);
	void          onDead(BWAPI::Unit unit);
	void          setOrder(int unitID, UnitOrder order);
	bool          isCamper(int unitID);
	UnitOrder     getOrder(int unitID);
	BWAPI::Unit   getBuildUnit(BWAPI::UnitType builderType);
};