#pragma once

#include <BWAPI.h>
#include <map>
#include <vector>

class BaseManager
{
	int                        m_depthOfRegions = 3;
	std::vector<BWAPI::Region> m_regions;
	BWAPI::Position            m_baseLocation;

public:

	BaseManager(BWAPI::Position baseLocation);

	const std::vector<BWAPI::Region>& BaseManager::getRegions() const;
};