#pragma once

#include <BWAPI.h>
#include <map>
#include <vector>
#include <set>

class BaseManager
{
	int                           m_depthOfRegions = 5;
	std::vector<BWAPI::Region>    m_regions;
	std::vector<BWAPI::Region>    m_chokePoints;
	BWAPI::Position               m_baseLocation;
	std::set<BWAPI::Unit>		  m_units;

public:

	BaseManager(BWAPI::Position baseLocation);

	void								 onFrame();
	void                                 updateRegions();
	void                                 updateChokePoints();
	void								 updateUnits();
	const std::vector<BWAPI::Region>&    getRegions() const;
	const std::vector<BWAPI::Region>&    getChokePoints() const;
	const std::set<BWAPI::Unit>&         getUnits() const;
};