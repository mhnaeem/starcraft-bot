#pragma once

#include <BWAPI.h>
#include <map>
#include <vector>

class BaseManager
{
	int                           m_depthOfRegions = 5;
	std::vector<BWAPI::Region>    m_regions;
	std::vector<BWAPI::Region>    m_chokePoints;
	BWAPI::Position               m_baseLocation;

public:

	BaseManager(BWAPI::Position baseLocation);

	void                                 updateRegions();
	void                                 updateChokePoints();
	const std::vector<BWAPI::Region>&    getRegions() const;
	const std::vector<BWAPI::Region>&    getChokePoints() const;
};