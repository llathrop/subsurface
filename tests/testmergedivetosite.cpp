// SPDX-License-Identifier: GPL-2.0
#include "testmergedivetosite.h"
#include "core/dive.h"
#include "core/divelog.h"
#include "core/divesite.h"
#include "core/pref.h"

void TestMergeDiveToSite::initTestCase()
{
	TestBase::initTestCase();
	prefs = default_prefs;
}

void TestMergeDiveToSite::cleanup()
{
	clear_dive_file_data();
}

void TestMergeDiveToSite::testMergeSingleDiveToSite()
{
	// Create target site with location
	dive_site *targetSite = divelog.sites.create("Target Site");
	targetSite->location = create_location(48.8566, 2.3522); // Paris

	// Create source site with a dive
	dive_site *sourceSite = divelog.sites.create("Source Site");
	sourceSite->location = create_location(48.8570, 2.3530); // Nearby

	// Create a dive at the source site
	auto d = std::make_unique<dive>();
	dive *divePtr = d.get();
	divelog.dives.record_dive(std::move(d));
	sourceSite->add_dive(divePtr);

	QCOMPARE(divelog.sites.size(), 2);
	QCOMPARE(sourceSite->dives.size(), 1);
	QCOMPARE(targetSite->dives.size(), 0);
	QCOMPARE(divePtr->dive_site, sourceSite);

	// Merge source site into target site (simulating the merge operation)
	// Move the dive from source to target
	unregister_dive_from_dive_site(divePtr);
	targetSite->add_dive(divePtr);

	// Verify the dive is now at the target site
	QCOMPARE(divePtr->dive_site, targetSite);
	QCOMPARE(targetSite->dives.size(), 1);
	QCOMPARE(sourceSite->dives.size(), 0);
}

void TestMergeDiveToSite::testMergeMultipleDivesToSite()
{
	// Create target site
	dive_site *targetSite = divelog.sites.create("Target Site");
	targetSite->location = create_location(48.8566, 2.3522);

	// Create source site
	dive_site *sourceSite = divelog.sites.create("Source Site");
	sourceSite->location = create_location(48.8570, 2.3530);

	// Create multiple dives at the source site
	std::vector<dive *> divePtrs;
	for (int i = 0; i < 3; i++) {
		auto d = std::make_unique<dive>();
		dive *divePtr = d.get();
		divePtrs.push_back(divePtr);
		divelog.dives.record_dive(std::move(d));
		sourceSite->add_dive(divePtr);
	}

	QCOMPARE(sourceSite->dives.size(), 3);
	QCOMPARE(targetSite->dives.size(), 0);

	// Merge all dives from source to target
	for (dive *divePtr : divePtrs) {
		unregister_dive_from_dive_site(divePtr);
		targetSite->add_dive(divePtr);
	}

	// Verify all dives moved
	QCOMPARE(targetSite->dives.size(), 3);
	QCOMPARE(sourceSite->dives.size(), 0);
	for (dive *divePtr : divePtrs) {
		QCOMPARE(divePtr->dive_site, targetSite);
	}
}

void TestMergeDiveToSite::testMergePreservesTargetSiteData()
{
	// Create target site with full data
	dive_site *targetSite = divelog.sites.create("Target Site");
	targetSite->location = create_location(48.8566, 2.3522);
	targetSite->description = "Famous dive site";
	targetSite->notes = "Great visibility";

	// Create source site
	dive_site *sourceSite = divelog.sites.create("Source Site");
	sourceSite->location = create_location(48.8570, 2.3530);
	sourceSite->description = "Source description";
	sourceSite->notes = "Source notes";

	// Create a dive at source
	auto d = std::make_unique<dive>();
	dive *divePtr = d.get();
	divelog.dives.record_dive(std::move(d));
	sourceSite->add_dive(divePtr);

	// Store original target site data
	std::string origName = targetSite->name;
	std::string origDesc = targetSite->description;
	std::string origNotes = targetSite->notes;
	location_t origLoc = targetSite->location;

	// Merge source into target
	unregister_dive_from_dive_site(divePtr);
	targetSite->add_dive(divePtr);

	// Verify target site data is preserved
	QCOMPARE(targetSite->name, origName);
	QCOMPARE(targetSite->description, origDesc);
	QCOMPARE(targetSite->notes, origNotes);
	QVERIFY(targetSite->location == origLoc);
}

void TestMergeDiveToSite::testMergeUpdatesAssociatedDives()
{
	// Create target site
	dive_site *targetSite = divelog.sites.create("Target Site");
	targetSite->location = create_location(48.8566, 2.3522);

	// Add existing dive to target
	auto existingDive = std::make_unique<dive>();
	dive *existingPtr = existingDive.get();
	divelog.dives.record_dive(std::move(existingDive));
	targetSite->add_dive(existingPtr);

	// Create source site with dive
	dive_site *sourceSite = divelog.sites.create("Source Site");
	sourceSite->location = create_location(48.8570, 2.3530);

	auto newDive = std::make_unique<dive>();
	dive *newPtr = newDive.get();
	divelog.dives.record_dive(std::move(newDive));
	sourceSite->add_dive(newPtr);

	QCOMPARE(targetSite->dives.size(), 1);

	// Merge source into target
	unregister_dive_from_dive_site(newPtr);
	targetSite->add_dive(newPtr);

	// Both dives should now be at target site
	QCOMPARE(targetSite->dives.size(), 2);
	QCOMPARE(existingPtr->dive_site, targetSite);
	QCOMPARE(newPtr->dive_site, targetSite);

	// Verify both dives are in the site's dive list
	bool foundExisting = false;
	bool foundNew = false;
	for (dive *d : targetSite->dives) {
		if (d == existingPtr) foundExisting = true;
		if (d == newPtr) foundNew = true;
	}
	QVERIFY(foundExisting);
	QVERIFY(foundNew);
}

QTEST_GUILESS_MAIN(TestMergeDiveToSite)
