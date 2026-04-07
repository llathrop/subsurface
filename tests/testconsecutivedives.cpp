// SPDX-License-Identifier: GPL-2.0
#include "testconsecutivedives.h"
#include "core/consecutive_dive_detector.h"
#include "core/dive.h"
#include "core/divelist.h"
#include "core/divelog.h"
#include "core/pref.h"

#include <memory>

void TestConsecutiveDives::initTestCase()
{
	TestBase::initTestCase();
	prefs = default_prefs;
}

void TestConsecutiveDives::cleanup()
{
	clear_dive_file_data();
}

// Helper function to create a test dive
static std::unique_ptr<dive> create_test_dive(timestamp_t when, int duration_seconds, int depth_mm)
{
	auto d = std::make_unique<dive>();
	d->when = when;
	d->duration.seconds = duration_seconds;
	d->dcs.resize(1);
	d->dcs[0].duration.seconds = duration_seconds;
	d->dcs[0].maxdepth.mm = depth_mm;
	d->maxdepth.mm = depth_mm;
	return d;
}

void TestConsecutiveDives::testSurfaceIntervalCalculation()
{
	// Create two dives: first ends at t=1000+600=1600, second starts at t=2000
	// Surface interval should be 2000-1600=400 seconds
	auto d1 = create_test_dive(1000, 600, 20000);  // 10 min dive
	auto d2 = create_test_dive(2000, 600, 25000);  // 10 min dive, starts 400s after first ends

	int interval = calculate_surface_interval(*d1, *d2);
	QCOMPARE(interval, 400);
}

void TestConsecutiveDives::testAreConsecutive()
{
	// Dives within 60 minute threshold
	auto d1 = create_test_dive(1000, 600, 20000);   // ends at 1600
	auto d2 = create_test_dive(2000, 600, 25000);   // starts at 2000, interval = 400s = 6.7 min
	QVERIFY(are_dives_consecutive(*d1, *d2, 60));

	// Dives outside 60 minute threshold
	auto d3 = create_test_dive(1000, 600, 20000);   // ends at 1600
	auto d4 = create_test_dive(10000, 600, 25000);  // starts at 10000, interval = 8400s = 140 min
	QVERIFY(!are_dives_consecutive(*d3, *d4, 60));

	// Dives with very short interval (1 minute threshold)
	QVERIFY(!are_dives_consecutive(*d1, *d2, 1));   // 400s > 60s
}

void TestConsecutiveDives::testFindPairs()
{
	dive_table dives;

	// Add three dives: first two are consecutive, third is not
	auto d1 = create_test_dive(1000, 1800, 20000);    // ends at 2800
	auto d2 = create_test_dive(3000, 1800, 25000);    // starts at 3000, interval = 200s
	auto d3 = create_test_dive(100000, 1800, 15000);  // much later

	dive *d1_ptr = d1.get();
	dive *d2_ptr = d2.get();

	dives.put(std::move(d1));
	dives.put(std::move(d2));
	dives.put(std::move(d3));

	auto pairs = find_consecutive_dive_pairs(dives, 60);

	QCOMPARE(static_cast<int>(pairs.size()), 1);
	QCOMPARE(pairs[0].first, d1_ptr);
	QCOMPARE(pairs[0].second, d2_ptr);
}

void TestConsecutiveDives::testFindGroups()
{
	dive_table dives;

	// Add three consecutive dives that should form one group
	auto d1 = create_test_dive(1000, 1800, 20000);    // ends at 2800
	auto d2 = create_test_dive(3000, 1800, 25000);    // starts at 3000, ends at 4800
	auto d3 = create_test_dive(5000, 1800, 22000);    // starts at 5000, interval = 200s

	dive *d1_ptr = d1.get();
	dive *d2_ptr = d2.get();
	dive *d3_ptr = d3.get();

	dives.put(std::move(d1));
	dives.put(std::move(d2));
	dives.put(std::move(d3));

	auto groups = find_consecutive_dives(dives, 60);

	QCOMPARE(static_cast<int>(groups.size()), 1);
	QCOMPARE(static_cast<int>(groups[0].dives.size()), 3);
	QCOMPARE(groups[0].dives[0], d1_ptr);
	QCOMPARE(groups[0].dives[1], d2_ptr);
	QCOMPARE(groups[0].dives[2], d3_ptr);
	QCOMPARE(groups[0].max_depth_mm, 25000);  // Max of 20000, 25000, 22000
}

void TestConsecutiveDives::testNoConsecutiveDives()
{
	dive_table dives;

	// Add dives that are not consecutive
	auto d1 = create_test_dive(1000, 1800, 20000);
	auto d2 = create_test_dive(100000, 1800, 25000);  // Much later

	dives.put(std::move(d1));
	dives.put(std::move(d2));

	auto pairs = find_consecutive_dive_pairs(dives, 60);
	QVERIFY(pairs.empty());

	auto groups = find_consecutive_dives(dives, 60);
	QVERIFY(groups.empty());
}

void TestConsecutiveDives::testOverlappingDives()
{
	dive_table dives;

	// Create overlapping dives (second starts before first ends)
	auto d1 = create_test_dive(1000, 3000, 20000);    // ends at 4000
	auto d2 = create_test_dive(2000, 1800, 25000);    // starts at 2000, overlaps

	dives.put(std::move(d1));
	dives.put(std::move(d2));

	// Surface interval should be negative for overlapping dives
	int interval = calculate_surface_interval(*dives[0], *dives[1]);
	QVERIFY(interval < 0);

	// Overlapping dives should not be considered consecutive
	auto pairs = find_consecutive_dive_pairs(dives, 60);
	QVERIFY(pairs.empty());
}

void TestConsecutiveDives::testCustomThreshold()
{
	dive_table dives;

	// Add dives with 20 minute surface interval
	auto d1 = create_test_dive(1000, 1800, 20000);    // ends at 2800
	auto d2 = create_test_dive(4000, 1800, 25000);    // starts at 4000, interval = 1200s = 20 min

	dives.put(std::move(d1));
	dives.put(std::move(d2));

	// Should be consecutive with 30 minute threshold
	auto pairs30 = find_consecutive_dive_pairs(dives, 30);
	QCOMPARE(static_cast<int>(pairs30.size()), 1);

	// Should NOT be consecutive with 15 minute threshold
	auto pairs15 = find_consecutive_dive_pairs(dives, 15);
	QVERIFY(pairs15.empty());
}

QTEST_GUILESS_MAIN(TestConsecutiveDives)
