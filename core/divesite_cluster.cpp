// SPDX-License-Identifier: GPL-2.0
#include "divesite_cluster.h"
#include "divesite.h"
#include "units.h"
#include <algorithm>
#include <deque>

// 1 micro-degree is approx 0.11132 meters
// 1 meter is approx 8.98 micro-degrees
static const double METERS_TO_UDEG_LAT = 10.0;

std::vector<DiveSiteGroup> find_dive_site_groups(const std::vector<dive_site *> &sitesIn, unsigned int distanceMeters)
{
	std::vector<DiveSiteGroup> result;

	// Filter and copy for sorting
	std::vector<dive_site *> sitesWithLocation;
	for (dive_site *ds : sitesIn) {
		if (ds->has_gps_location()) {
			sitesWithLocation.push_back(ds);
		}
	}

	if (sitesWithLocation.empty())
		return result;

	// Optimization: sort sites by latitude for faster neighbor lookup
	std::sort(sitesWithLocation.begin(), sitesWithLocation.end(),
		[](dive_site *a, dive_site *b) {
			return a->location.lat.udeg < b->location.lat.udeg;
		});

	// Track which sites have been assigned to a group using indices in sitesWithLocation
	std::vector<bool> assigned(sitesWithLocation.size(), false);
	int udegDelta = static_cast<int>(distanceMeters * METERS_TO_UDEG_LAT);

	for (size_t i = 0; i < sitesWithLocation.size(); ++i) {
		if (assigned[i])
			continue;

		// Potential group starts here
		std::vector<dive_site *> groupSites;
		std::deque<size_t> queue;

		queue.push_back(i);
		assigned[i] = true;
		groupSites.push_back(sitesWithLocation[i]);

		// BFS expansion
		while (!queue.empty()) {
			size_t currIdx = queue.front();
			queue.pop_front();
			dive_site *currSite = sitesWithLocation[currIdx];

			// Find candidate range based on latitude
			int minLat = currSite->location.lat.udeg - udegDelta;
			int maxLat = currSite->location.lat.udeg + udegDelta;

			auto itLow = std::lower_bound(sitesWithLocation.begin(), sitesWithLocation.end(), minLat,
				[](dive_site *s, int val) { return s->location.lat.udeg < val; });
			auto itHigh = std::upper_bound(sitesWithLocation.begin(), sitesWithLocation.end(), maxLat,
				[](int val, dive_site *s) { return val < s->location.lat.udeg; });

			for (auto it = itLow; it != itHigh; ++it) {
				size_t candIdx = std::distance(sitesWithLocation.begin(), it);
				if (assigned[candIdx])
					continue;

				dive_site *candSite = *it;
				unsigned int dist = get_distance(currSite->location, candSite->location);
				if (dist <= distanceMeters) {
					assigned[candIdx] = true;
					groupSites.push_back(candSite);
					queue.push_back(candIdx);
				}
			}
		}

		// Only create a group if there are multiple sites
		if (groupSites.size() > 1) {
			DiveSiteGroup group;
			group.sites = groupSites;
			// Default: select the site with the most dives as primary
			group.primarySite = *std::max_element(groupSites.begin(), groupSites.end(),
				[](dive_site *a, dive_site *b) {
					return a->dives.size() < b->dives.size();
				});
			result.push_back(group);
		}
	}

	return result;
}
