// SPDX-License-Identifier: GPL-2.0
#ifndef TESTCONSECUTIVEDIVES_H
#define TESTCONSECUTIVEDIVES_H

#include "testbase.h"

class TestConsecutiveDives : public TestBase {
	Q_OBJECT
private slots:
	void initTestCase();
	void cleanup();

	void testSurfaceIntervalCalculation();
	void testAreConsecutive();
	void testFindPairs();
	void testFindGroups();
	void testNoConsecutiveDives();
	void testOverlappingDives();
	void testCustomThreshold();
};

#endif // TESTCONSECUTIVEDIVES_H
