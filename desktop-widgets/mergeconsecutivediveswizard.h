// SPDX-License-Identifier: GPL-2.0
#ifndef MERGECONSECUTIVEDIVESWIZARD_H
#define MERGECONSECUTIVEDIVESWIZARD_H

#include <QDialog>
#include <vector>
#include "ui_mergeconsecutivediveswizard.h"

struct dive;

class MergeConsecutiveDivesWizard : public QDialog {
	Q_OBJECT
public:
	explicit MergeConsecutiveDivesWizard(QWidget *parent);
	~MergeConsecutiveDivesWizard();

	// Returns the groups of dives selected for merging
	// Each inner vector contains 2+ dives to merge together
	std::vector<std::vector<dive *>> getSelectedMergeGroups() const;

	// Static convenience method to show wizard and perform merges
	static void showWizardAndMerge(QWidget *parent);

private slots:
	void detectConsecutiveDives();
	void selectAll();
	void selectNone();
	void updateSelectedCount();
	void onCheckboxChanged(int state);

private:
	void populateTable();
	QString formatDiveInfo(const dive *d) const;
	QString formatDuration(int seconds) const;
	QString formatInterval(int seconds) const;

	Ui::MergeConsecutiveDivesWizard ui;

	// Store detected dive pairs for later retrieval
	struct DivePairInfo {
		dive *first;
		dive *second;
		int surfaceIntervalSeconds;
		int combinedDurationSeconds;
	};
	std::vector<DivePairInfo> detectedPairs;
};

#endif // MERGECONSECUTIVEDIVESWIZARD_H
