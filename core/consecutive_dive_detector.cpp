// SPDX-License-Identifier: GPL-2.0
#include "consecutive_dive_detector.h"
#include "dive.h"
#include "divelist.h"

#include <algorithm>

int calculate_surface_interval(const dive &first, const dive &second)
{
	timestamp_t first_end = first.endtime();
	timestamp_t second_start = second.when;

	// Return negative if dives overlap
	if (second_start < first_end)
		return static_cast<int>(second_start - first_end);

	return static_cast<int>(second_start - first_end);
}

bool are_dives_consecutive(const dive &first, const dive &second, int threshold_minutes)
{
	int interval = calculate_surface_interval(first, second);

	// Dives must not overlap and must be within threshold
	return interval >= 0 && interval <= threshold_minutes * 60;
}

std::vector<std::pair<dive *, dive *>> find_consecutive_dive_pairs(const dive_table &dives,
								   int threshold_minutes)
{
	std::vector<std::pair<dive *, dive *>> result;

	if (dives.size() < 2)
		return result;

	// Iterate through consecutive dives in the sorted table
	for (size_t i = 0; i + 1 < dives.size(); ++i) {
		dive *first = dives[i].get();
		dive *second = dives[i + 1].get();

		// Skip invalid dives
		if (first->invalid || second->invalid)
			continue;

		if (are_dives_consecutive(*first, *second, threshold_minutes))
			result.emplace_back(first, second);
	}

	return result;
}

std::vector<consecutive_dive_group> find_consecutive_dives(const dive_table &dives,
							   int threshold_minutes)
{
	std::vector<consecutive_dive_group> result;

	if (dives.size() < 2)
		return result;

	// Track which dives have been added to a group
	std::vector<bool> in_group(dives.size(), false);

	for (size_t i = 0; i < dives.size(); ++i) {
		// Skip if already in a group or invalid
		if (in_group[i] || dives[i]->invalid)
			continue;

		consecutive_dive_group group;
		group.dives.push_back(dives[i].get());
		group.surface_interval_seconds = 0;
		group.total_duration_seconds = dives[i]->duration.seconds;
		group.max_depth_mm = dives[i]->maxdepth.mm;
		in_group[i] = true;

		// Look for consecutive dives following this one
		size_t last_idx = i;
		for (size_t j = i + 1; j < dives.size(); ++j) {
			if (in_group[j] || dives[j]->invalid)
				continue;

			dive *last_dive = dives[last_idx].get();
			dive *candidate = dives[j].get();

			if (are_dives_consecutive(*last_dive, *candidate, threshold_minutes)) {
				int interval = calculate_surface_interval(*last_dive, *candidate);
				group.dives.push_back(candidate);
				group.surface_interval_seconds += interval;
				group.total_duration_seconds += candidate->duration.seconds + interval;
				if (candidate->maxdepth.mm > group.max_depth_mm)
					group.max_depth_mm = candidate->maxdepth.mm;
				in_group[j] = true;
				last_idx = j;
			} else {
				// Dives are sorted by time, so if this one isn't consecutive,
				// later ones won't be either (unless there are time gaps)
				// However, we should check all remaining dives in case of
				// non-contiguous valid dive sequences
				int interval = calculate_surface_interval(*last_dive, *candidate);
				if (interval > threshold_minutes * 60)
					break; // No point checking further
			}
		}

		// Only add groups with 2 or more dives
		if (group.dives.size() >= 2)
			result.push_back(std::move(group));
	}

	return result;
}
