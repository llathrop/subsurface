// SPDX-License-Identifier: GPL-2.0
#ifndef CONSOLIDATEDIVESITESDIALOG_H
#define CONSOLIDATEDIVESITESDIALOG_H

#include <QDialog>
#include <QVector>
#include <vector>

#include "ui_consolidatedivesitesdialog.h"
#include "core/divesite_cluster.h"

struct dive_site;

class ConsolidateDiveSitesDialog : public QDialog {
	Q_OBJECT
public:
	explicit ConsolidateDiveSitesDialog(QWidget *parent = nullptr);

private slots:
	void scanForGroups();
	void onSelectionChanged();
	void setPrimaryClicked();
	void consolidateGroupClicked();
	void consolidateAllClicked();

private:
	Ui::ConsolidateDiveSitesDialog ui;
	std::vector<DiveSiteGroup> groups;

	// Returns the distance threshold in meters based on user input and unit settings
	unsigned int getDistanceThresholdMeters() const;

	// Find groups of dive sites within the specified distance
	void findSiteGroups(unsigned int distanceMeters);

	// Update the tree widget display
	void updateTreeWidget();

	// Get the currently selected group index, or -1 if none
	int getSelectedGroupIndex() const;

	// Get the currently selected site within a group, or nullptr
	dive_site *getSelectedSite() const;

	// Consolidate a single group (merge all sites to primary)
	void consolidateGroup(DiveSiteGroup &group);

	// Format distance for display based on user units
	QString formatDistance(unsigned int meters) const;

	// Update button states based on current selection
	void updateButtonStates();
};

#endif // CONSOLIDATEDIVESITESDIALOG_H
