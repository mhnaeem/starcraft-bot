#pragma once

#include "BaseManager.h"
#include "InformationManager.h"
#include <BWAPI.h>
#include <vector>
#include <algorithm>

BaseManager::BaseManager(BWAPI::Position baseLocation)
{
	const auto updateRegions = [&]()
	{
		m_regions = std::vector<BWAPI::Region>();
		BWAPI::Region centerRegion = BWAPI::Broodwar->getRegionAt(m_baseLocation.x, m_baseLocation.y);

		std::vector<BWAPI::Region> queue = std::vector<BWAPI::Region>();
		queue.push_back(centerRegion);

		int depth = 0;
		int queueIndex = 0;
		while (true)
		{
			if (queue.empty() || queueIndex == queue.size())
			{
				break;
			}

			BWAPI::Region& region = queue[queueIndex];
			m_regions.push_back(region);
			queueIndex++;

			if (depth <= m_depthOfRegions) {
				for (auto expRegion : region->getNeighbors())
				{
					if (std::find(queue.begin(), queue.end(), expRegion) == queue.end())
					{
						queue.push_back(expRegion);
					}
				}
				depth++;
			}
		}
	};

	m_baseLocation = baseLocation;
	updateRegions();
}

const std::vector<BWAPI::Region>& BaseManager::getRegions() const
{
	return m_regions;
}