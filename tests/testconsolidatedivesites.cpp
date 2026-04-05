// SPDX-License-Identifier: GPL-2.0
#include "testconsolidatedivesites.h"
#include "core/divesite_cluster.h"
#include "core/divesite.h"
#include "core/units.h"

void TestConsolidateDiveSites::testClustering()
{
	// Create some dive sites
	// Site A and B are close (5m)
	// Site C is far (1000m)
	// Site D and E are close (8m) but far from others
	
	dive_site dsA("Site A", create_location(52.0, 1.0));
	dive_site dsB("Site B", create_location(52.00005, 1.00005)); // Approx 7m away
	dive_site dsC("Site C", create_location(52.1, 1.1)); // Far
	dive_site dsD("Site D", create_location(45.0, -1.0));
	dive_site dsE("Site E", create_location(45.00006, -1.00006)); // Approx 8m away
	
	std::vector<dive_site *> sites = {&dsA, &dsB, &dsC, &dsD, &dsE};
	
	// Cluster with 10m threshold
	auto groups = find_dive_site_groups(sites, 10);
	
	QCOMPARE(groups.size(), 2u);
	
	// Group 1: A and B
	// Group 2: D and E
	// C should be alone (no group)
	
	bool foundAB = false;
	bool foundDE = false;
	
	for (const auto &group : groups) {
		if (group.sites.size() == 2) {
			bool hasA = std::find(group.sites.begin(), group.sites.end(), &dsA) != group.sites.end();
			bool hasB = std::find(group.sites.begin(), group.sites.end(), &dsB) != group.sites.end();
			bool hasD = std::find(group.sites.begin(), group.sites.end(), &dsD) != group.sites.end();
			bool hasE = std::find(group.sites.begin(), group.sites.end(), &dsE) != group.sites.end();
			
			if (hasA && hasB) foundAB = true;
			if (hasD && hasE) foundDE = true;
		}
	}
	
	QVERIFY(foundAB);
	QVERIFY(foundDE);
}

void TestConsolidateDiveSites::testDistanceOptimization()
{
	// Test chain of sites: A--B--C
	// A-B is 8m, B-C is 8m, but A-C is 16m
	// With 10m threshold, they should all be in one group (BFS expansion)
	// We use coordinates away from (0,0) because Subsurface treats (0,0) as "no location".
	
	dive_site dsA("Site A", create_location(1.0, 1.0));
	dive_site dsB("Site B", create_location(1.00007, 1.0)); // Approx 7.8m
	dive_site dsC("Site C", create_location(1.00014, 1.0)); // Approx 15.6m from A, 7.8m from B
	
	std::vector<dive_site *> sites = {&dsA, &dsB, &dsC};
	auto groups = find_dive_site_groups(sites, 10);
	
	QCOMPARE(groups.size(), 1u);
	QCOMPARE(groups[0].sites.size(), 3u);
}

QTEST_GUILESS_MAIN(TestConsolidateDiveSites)
