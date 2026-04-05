// SPDX-License-Identifier: GPL-2.0
#ifndef DIVESITE_CLUSTER_H
#define DIVESITE_CLUSTER_H

#include <vector>

struct dive_site;

struct DiveSiteGroup {
	std::vector<dive_site *> sites;
	dive_site *primarySite;

	DiveSiteGroup() : primarySite(nullptr) {}
};

std::vector<DiveSiteGroup> find_dive_site_groups(const std::vector<dive_site *> &sitesWithLocation, unsigned int distanceMeters);

#endif // DIVESITE_CLUSTER_H
