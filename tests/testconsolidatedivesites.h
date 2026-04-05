// SPDX-License-Identifier: GPL-2.0
#ifndef TESTCONSOLIDATEDIVESITES_H
#define TESTCONSOLIDATEDIVESITES_H

#include "testbase.h"

class TestConsolidateDiveSites : public TestBase {
	Q_OBJECT
private slots:
	void testClustering();
	void testDistanceOptimization();
};

#endif // TESTCONSOLIDATEDIVESITES_H
