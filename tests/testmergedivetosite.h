// SPDX-License-Identifier: GPL-2.0
#ifndef TESTMERGEDIVETOSITE_H
#define TESTMERGEDIVETOSITE_H

#include "testbase.h"

class TestMergeDiveToSite : public TestBase {
	Q_OBJECT
private slots:
	void initTestCase();
	void cleanup();

	void testMergeSingleDiveToSite();
	void testMergeMultipleDivesToSite();
	void testMergePreservesTargetSiteData();
	void testMergeUpdatesAssociatedDives();
};

#endif // TESTMERGEDIVETOSITE_H
