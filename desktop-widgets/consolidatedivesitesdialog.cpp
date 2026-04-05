// SPDX-License-Identifier: GPL-2.0
#include "consolidatedivesitesdialog.h"
#include "core/divelog.h"
#include "core/divesite.h"
#include "core/pref.h"
#include "core/units.h"
#include "commands/command.h"

#include <QTreeWidgetItem>
#include <QMessageBox>
#include <algorithm>
#include <set>
#include <deque>

// Default distance threshold: 10 meters (approximately 33 feet)
static const double DEFAULT_DISTANCE_METERS = 10.0;
static const double DEFAULT_DISTANCE_FEET = 33.0;
static const double FEET_TO_METERS = 0.3048;
static const double METERS_TO_FEET = 3.28084;

// Conversion factor: 1 degree latitude is approx 111320 meters
// 1 micro-degree is approx 0.11132 meters
// 1 meter is approx 8.98 micro-degrees
static const double METERS_TO_UDEG_LAT = 9.0;

ConsolidateDiveSitesDialog::ConsolidateDiveSitesDialog(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);

	// Set up units based on user preferences
	if (prefs.units.length == units::FEET) {
		ui.distanceSpinBox->setValue(DEFAULT_DISTANCE_FEET);
		ui.distanceUnitLabel->setText(tr("ft"));
		ui.distanceSpinBox->setSuffix(QString());
	} else {
		ui.distanceSpinBox->setValue(DEFAULT_DISTANCE_METERS);
		ui.distanceUnitLabel->setText(tr("m"));
		ui.distanceSpinBox->setSuffix(QString());
	}

	// Connect signals
	connect(ui.scanButton, &QPushButton::clicked, this, &ConsolidateDiveSitesDialog::scanForGroups);
	connect(ui.setPrimaryButton, &QPushButton::clicked, this, &ConsolidateDiveSitesDialog::setPrimaryClicked);
	connect(ui.consolidateGroupButton, &QPushButton::clicked, this, &ConsolidateDiveSitesDialog::consolidateGroupClicked);
	connect(ui.consolidateAllButton, &QPushButton::clicked, this, &ConsolidateDiveSitesDialog::consolidateAllClicked);
	connect(ui.groupsTreeWidget, &QTreeWidget::itemSelectionChanged, this, &ConsolidateDiveSitesDialog::onSelectionChanged);

	// Initial button states
	updateButtonStates();
}

unsigned int ConsolidateDiveSitesDialog::getDistanceThresholdMeters() const
{
	double value = ui.distanceSpinBox->value();
	if (prefs.units.length == units::FEET) {
		// Convert feet to meters
		return static_cast<unsigned int>(value * FEET_TO_METERS);
	}
	return static_cast<unsigned int>(value);
}

QString ConsolidateDiveSitesDialog::formatDistance(unsigned int meters) const
{
	if (prefs.units.length == units::FEET) {
		double feet = meters * METERS_TO_FEET;
		return QString::number(feet, 'f', 1) + " " + tr("ft");
	}
	return QString::number(meters) + " " + tr("m");
}

void ConsolidateDiveSitesDialog::scanForGroups()
{
	unsigned int distanceMeters = getDistanceThresholdMeters();
	findSiteGroups(distanceMeters);
	updateTreeWidget();
	updateButtonStates();
}

void ConsolidateDiveSitesDialog::findSiteGroups(unsigned int distanceMeters)
{
	std::vector<dive_site *> sites;
	for (const auto &ds : divelog.sites) {
		sites.push_back(ds.get());
	}
	groups = find_dive_site_groups(sites, distanceMeters);
}

void ConsolidateDiveSitesDialog::updateTreeWidget()
{
	ui.groupsTreeWidget->clear();

	int groupNum = 1;
	for (const DiveSiteGroup &group : groups) {
		// Create group header item
		QTreeWidgetItem *groupItem = new QTreeWidgetItem(ui.groupsTreeWidget);
		groupItem->setText(0, tr("Group %1 (%2 sites)").arg(groupNum).arg(group.sites.size()));
		groupItem->setData(0, Qt::UserRole, QVariant::fromValue(groupNum - 1)); // Store group index
		groupItem->setExpanded(true);

		// Find the reference site (primary) for distance calculations
		dive_site *refSite = group.primarySite ? group.primarySite : group.sites[0];

		// Add each site in the group
		for (dive_site *site : group.sites) {
			QTreeWidgetItem *siteItem = new QTreeWidgetItem(groupItem);

			// Site name with primary indicator
			QString name = QString::fromStdString(site->name);
			if (site == group.primarySite) {
				name += tr(" (Primary)");
				QFont font = siteItem->font(0);
				font.setBold(true);
				siteItem->setFont(0, font);
			}
			siteItem->setText(0, name);

			// Number of dives at this site
			siteItem->setText(1, QString::number(site->dives.size()));

			// Distance from primary site
			if (site == refSite) {
				siteItem->setText(2, "-");
			} else {
				unsigned int dist = get_distance(refSite->location, site->location);
				siteItem->setText(2, formatDistance(dist));
			}

			// Store pointer to dive site
			siteItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(reinterpret_cast<quintptr>(site)));
		}

		groupNum++;
	}

	// Adjust column widths
	ui.groupsTreeWidget->resizeColumnToContents(0);
	ui.groupsTreeWidget->resizeColumnToContents(1);
	ui.groupsTreeWidget->resizeColumnToContents(2);
}

int ConsolidateDiveSitesDialog::getSelectedGroupIndex() const
{
	QList<QTreeWidgetItem *> selected = ui.groupsTreeWidget->selectedItems();
	if (selected.isEmpty())
		return -1;

	QTreeWidgetItem *item = selected.first();

	// If it's a site item (has parent), get the parent's group index
	if (item->parent()) {
		item = item->parent();
	}

	return item->data(0, Qt::UserRole).toInt();
}

dive_site *ConsolidateDiveSitesDialog::getSelectedSite() const
{
	QList<QTreeWidgetItem *> selected = ui.groupsTreeWidget->selectedItems();
	if (selected.isEmpty())
		return nullptr;

	QTreeWidgetItem *item = selected.first();

	// Only return a site if a site item (not a group header) is selected
	if (!item->parent())
		return nullptr;

	quintptr ptr = item->data(0, Qt::UserRole + 1).value<quintptr>();
	return reinterpret_cast<dive_site *>(ptr);
}

void ConsolidateDiveSitesDialog::onSelectionChanged()
{
	updateButtonStates();
}

void ConsolidateDiveSitesDialog::updateButtonStates()
{
	int groupIndex = getSelectedGroupIndex();
	dive_site *selectedSite = getSelectedSite();

	// "Set as Primary" enabled only if a specific site is selected
	ui.setPrimaryButton->setEnabled(selectedSite != nullptr);

	// "Consolidate Selected Group" enabled if a group is selected and has a primary
	bool canConsolidateGroup = groupIndex >= 0 &&
		groupIndex < static_cast<int>(groups.size()) &&
		groups[groupIndex].primarySite != nullptr;
	ui.consolidateGroupButton->setEnabled(canConsolidateGroup);

	// "Consolidate All" enabled if there are any groups
	ui.consolidateAllButton->setEnabled(!groups.empty());
}

void ConsolidateDiveSitesDialog::setPrimaryClicked()
{
	int groupIndex = getSelectedGroupIndex();
	dive_site *selectedSite = getSelectedSite();

	if (groupIndex < 0 || groupIndex >= static_cast<int>(groups.size()) || !selectedSite)
		return;

	groups[groupIndex].primarySite = selectedSite;
	updateTreeWidget();

	// Re-select the group that was being edited
	if (groupIndex < ui.groupsTreeWidget->topLevelItemCount()) {
		ui.groupsTreeWidget->setCurrentItem(ui.groupsTreeWidget->topLevelItem(groupIndex));
	}
}

void ConsolidateDiveSitesDialog::consolidateGroup(DiveSiteGroup &group)
{
	if (!group.primarySite || group.sites.size() < 2)
		return;

	// Build list of sites to merge (all except primary)
	QVector<dive_site *> sitesToMerge;
	for (dive_site *site : group.sites) {
		if (site == group.primarySite)
			continue;
		sitesToMerge.append(site);
	}

	// Use the existing MergeDiveSites command
	Command::mergeDiveSites(group.primarySite, sitesToMerge);
}

void ConsolidateDiveSitesDialog::consolidateGroupClicked()
{
	int groupIndex = getSelectedGroupIndex();
	if (groupIndex < 0 || groupIndex >= static_cast<int>(groups.size()))
		return;

	DiveSiteGroup &group = groups[groupIndex];

	QString message = tr("This will merge %1 dive sites into '%2'.\n"
		"All dives at the other sites will be moved to '%2'.\n\n"
		"Do you want to continue?")
		.arg(group.sites.size())
		.arg(QString::fromStdString(group.primarySite->name));

	if (QMessageBox::question(this, tr("Consolidate Dive Sites"), message,
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
		return;
	}

	consolidateGroup(group);

	bool autoPurge = ui.autoPurgeCheckBox->isChecked();

	if (autoPurge) {
		Command::purgeUnusedDiveSites();
	}

	// Re-scan to reflect the new state of the divelog
	scanForGroups();
}

void ConsolidateDiveSitesDialog::consolidateAllClicked()
{
	if (groups.empty())
		return;

	// Count total sites to be consolidated
	size_t totalSites = 0;
	for (const DiveSiteGroup &group : groups) {
		totalSites += group.sites.size();
	}

	QString message = tr("This will consolidate %1 groups containing %2 dive sites.\n"
		"Each group will be merged into its primary site.\n\n"
		"Do you want to continue?")
		.arg(groups.size())
		.arg(totalSites);

	if (QMessageBox::question(this, tr("Consolidate All Dive Sites"), message,
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
		return;
	}

	// Consolidate all groups
	for (DiveSiteGroup &group : groups) {
		consolidateGroup(group);
	}

	bool autoPurge = ui.autoPurgeCheckBox->isChecked();
	if (autoPurge) {
		Command::purgeUnusedDiveSites();
	}

	// Re-scan to reflect the new state of the divelog
	scanForGroups();
}

