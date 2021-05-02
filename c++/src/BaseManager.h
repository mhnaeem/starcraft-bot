#pragma once

#include <BWAPI.h>
#include <map>
#include <vector>
#include <set>
#include "SmartUtils.h"

class BaseManager
{
	int                           m_depthOfRegions = 5;
	std::vector<int>              m_regions;
	std::vector<int>              m_chokePoints;
	BWAPI::Position               m_baseLocation;
	std::set<int>          		  m_units;
	BWAPI::Unit                   m_minerals;
	BWAPI::Unit                   m_gas;

	void                          trainWorkers();

public:

	BaseManager(BWAPI::Position baseLocation);

	void								 onFrame();
	void                                 updateRegions();
	void                                 updateChokePoints();
	void								 updateUnits();
	bool								 train(BWAPI::UnitType type);
	bool                                 build(BWAPI::UnitType type);
	BWAPI::Unit                          getGas();
	BWAPI::Unit					         getMinerals();
	BWAPI::Position				         getLocation();
	const std::vector<int>               getRegions() const;
	const std::vector<int>               getChokePoints() const;
	const std::set<int>                  getUnits() const;
};