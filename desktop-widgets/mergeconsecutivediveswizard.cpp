// SPDX-License-Identifier: GPL-2.0
#include "mergeconsecutivediveswizard.h"
#include "commands/command.h"
#include "core/consecutive_dive_detector.h"
#include "core/dive.h"
#include "core/divelog.h"
#include "core/string-format.h"

#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>

MergeConsecutiveDivesWizard::MergeConsecutiveDivesWizard(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);

	// Connect signals
	connect(ui.detectButton, &QPushButton::clicked, this, &MergeConsecutiveDivesWizard::detectConsecutiveDives);
	connect(ui.selectAllButton, &QPushButton::clicked, this, &MergeConsecutiveDivesWizard::selectAll);
	connect(ui.selectNoneButton, &QPushButton::clicked, this, &MergeConsecutiveDivesWizard::selectNone);

	// Setup keyboard shortcuts
	QShortcut *close = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this);
	connect(close, &QShortcut::activated, this, &QDialog::close);
	QShortcut *quit = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this);
	connect(quit, &QShortcut::activated, parent, &QWidget::close);

	// Configure table
	ui.divePairsTable->setColumnWidth(0, 50);   // Merge checkbox
	ui.divePairsTable->setColumnWidth(1, 200);  // First dive
	ui.divePairsTable->setColumnWidth(2, 200);  // Second dive
	ui.divePairsTable->setColumnWidth(3, 100);  // Surface interval
	ui.divePairsTable->setColumnWidth(4, 120);  // Combined duration

	// Disable OK button initially
	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	// Run detection with default threshold
	detectConsecutiveDives();
}

MergeConsecutiveDivesWizard::~MergeConsecutiveDivesWizard()
{
}

void MergeConsecutiveDivesWizard::detectConsecutiveDives()
{
	int threshold = ui.thresholdSpinBox->value();
	auto pairs = find_consecutive_dive_pairs(divelog.dives, threshold);

	detectedPairs.clear();
	for (const auto &pair : pairs) {
		DivePairInfo info;
		info.first = pair.first;
		info.second = pair.second;
		info.surfaceIntervalSeconds = calculate_surface_interval(*pair.first, *pair.second);
		info.combinedDurationSeconds = pair.first->duration.seconds +
					       info.surfaceIntervalSeconds +
					       pair.second->duration.seconds;
		detectedPairs.push_back(info);
	}

	populateTable();
}

void MergeConsecutiveDivesWizard::populateTable()
{
	ui.divePairsTable->setRowCount(0);
	ui.divePairsTable->setRowCount(static_cast<int>(detectedPairs.size()));

	for (size_t i = 0; i < detectedPairs.size(); ++i) {
		const auto &pair = detectedPairs[i];
		int row = static_cast<int>(i);

		// Checkbox for merge selection
		QCheckBox *checkbox = new QCheckBox();
		checkbox->setChecked(true);
		connect(checkbox, &QCheckBox::stateChanged, this, &MergeConsecutiveDivesWizard::onCheckboxChanged);

		QWidget *checkboxWidget = new QWidget();
		QHBoxLayout *layout = new QHBoxLayout(checkboxWidget);
		layout->addWidget(checkbox);
		layout->setAlignment(Qt::AlignCenter);
		layout->setContentsMargins(0, 0, 0, 0);
		ui.divePairsTable->setCellWidget(row, 0, checkboxWidget);

		// First dive info
		ui.divePairsTable->setItem(row, 1, new QTableWidgetItem(formatDiveInfo(pair.first)));

		// Second dive info
		ui.divePairsTable->setItem(row, 2, new QTableWidgetItem(formatDiveInfo(pair.second)));

		// Surface interval
		ui.divePairsTable->setItem(row, 3, new QTableWidgetItem(formatInterval(pair.surfaceIntervalSeconds)));

		// Combined duration
		ui.divePairsTable->setItem(row, 4, new QTableWidgetItem(formatDuration(pair.combinedDurationSeconds)));
	}

	updateSelectedCount();

	if (detectedPairs.empty()) {
		ui.resultsLabel->setText(tr("No consecutive dive pairs found within the specified threshold."));
	} else {
		ui.resultsLabel->setText(tr("Found %n consecutive dive pair(s). Select which to merge:",
					    "", static_cast<int>(detectedPairs.size())));
	}
}

QString MergeConsecutiveDivesWizard::formatDiveInfo(const dive *d) const
{
	QString dateStr = QString::fromStdString(formatDiveDateAndTime(d));
	QString depthStr = QString::fromStdString(formatDepth(d->maxdepth));
	QString durationStr = formatDuration(d->duration.seconds);

	QString info;
	if (d->number > 0)
		info = tr("#%1: %2, %3, %4").arg(d->number).arg(dateStr).arg(depthStr).arg(durationStr);
	else
		info = tr("%1, %2, %3").arg(dateStr).arg(depthStr).arg(durationStr);

	return info;
}

QString MergeConsecutiveDivesWizard::formatDuration(int seconds) const
{
	int hours = seconds / 3600;
	int minutes = (seconds % 3600) / 60;
	int secs = seconds % 60;

	if (hours > 0)
		return tr("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
	else
		return tr("%1:%2").arg(minutes).arg(secs, 2, 10, QChar('0'));
}

QString MergeConsecutiveDivesWizard::formatInterval(int seconds) const
{
	int minutes = seconds / 60;
	int secs = seconds % 60;

	if (minutes > 0)
		return tr("%1 min %2 sec").arg(minutes).arg(secs);
	else
		return tr("%1 sec").arg(secs);
}

void MergeConsecutiveDivesWizard::selectAll()
{
	for (int row = 0; row < ui.divePairsTable->rowCount(); ++row) {
		QWidget *widget = ui.divePairsTable->cellWidget(row, 0);
		if (widget) {
			QCheckBox *checkbox = widget->findChild<QCheckBox *>();
			if (checkbox)
				checkbox->setChecked(true);
		}
	}
}

void MergeConsecutiveDivesWizard::selectNone()
{
	for (int row = 0; row < ui.divePairsTable->rowCount(); ++row) {
		QWidget *widget = ui.divePairsTable->cellWidget(row, 0);
		if (widget) {
			QCheckBox *checkbox = widget->findChild<QCheckBox *>();
			if (checkbox)
				checkbox->setChecked(false);
		}
	}
}

void MergeConsecutiveDivesWizard::onCheckboxChanged(int)
{
	updateSelectedCount();
}

void MergeConsecutiveDivesWizard::updateSelectedCount()
{
	int count = 0;
	for (int row = 0; row < ui.divePairsTable->rowCount(); ++row) {
		QWidget *widget = ui.divePairsTable->cellWidget(row, 0);
		if (widget) {
			QCheckBox *checkbox = widget->findChild<QCheckBox *>();
			if (checkbox && checkbox->isChecked())
				++count;
		}
	}

	ui.selectedCountLabel->setText(tr("%n pair(s) selected", "", count));
	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(count > 0);
}

std::vector<std::vector<dive *>> MergeConsecutiveDivesWizard::getSelectedMergeGroups() const
{
	std::vector<std::vector<dive *>> result;

	for (int row = 0; row < ui.divePairsTable->rowCount(); ++row) {
		QWidget *widget = ui.divePairsTable->cellWidget(row, 0);
		if (widget) {
			QCheckBox *checkbox = widget->findChild<QCheckBox *>();
			if (checkbox && checkbox->isChecked() && static_cast<size_t>(row) < detectedPairs.size()) {
				std::vector<dive *> group;
				group.push_back(detectedPairs[row].first);
				group.push_back(detectedPairs[row].second);
				result.push_back(std::move(group));
			}
		}
	}

	return result;
}

void MergeConsecutiveDivesWizard::showWizardAndMerge(QWidget *parent)
{
	MergeConsecutiveDivesWizard wizard(parent);

	if (wizard.exec() != QDialog::Accepted)
		return;

	auto mergeGroups = wizard.getSelectedMergeGroups();
	if (mergeGroups.empty())
		return;

	// Confirm the merge
	int totalPairs = static_cast<int>(mergeGroups.size());
	QMessageBox::StandardButton reply = QMessageBox::question(
		parent,
		QObject::tr("Confirm merge"),
		QObject::tr("This will merge %n dive pair(s). This action can be undone.\n\nProceed?",
			    "", totalPairs),
		QMessageBox::Yes | QMessageBox::No);

	if (reply != QMessageBox::Yes)
		return;

	// Perform the merges using the command system
	// We need to merge in reverse order to avoid invalidating dive pointers
	// as dives are removed from the list
	for (auto it = mergeGroups.rbegin(); it != mergeGroups.rend(); ++it) {
		QVector<dive *> divesToMerge;
		for (dive *d : *it)
			divesToMerge.push_back(d);
		Command::mergeDives(divesToMerge);
	}
}
