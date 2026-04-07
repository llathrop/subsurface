// SPDX-License-Identifier: GPL-2.0
#ifndef CONSECUTIVE_DIVE_DETECTOR_H
#define CONSECUTIVE_DIVE_DETECTOR_H

#include <vector>

struct dive;
struct dive_table;

// Represents a group of consecutive dives that could be merged
struct consecutive_dive_group {
	std::vector<dive *> dives;	// Dives in this group, ordered by time
	int surface_interval_seconds;	// Time between first and last dive end/start
	int total_duration_seconds;	// Combined duration if merged
	int max_depth_mm;		// Maximum depth across all dives
};

// Find groups of consecutive dives within the specified time threshold
// threshold_minutes: maximum surface interval to consider dives as consecutive (default 60)
// Returns vector of groups, where each group contains 2 or more dives
std::vector<consecutive_dive_group> find_consecutive_dives(const dive_table &dives,
							   int threshold_minutes = 60);

// Find consecutive dive pairs only (simpler output for basic UI)
// Returns pairs of dive pointers that are within the threshold
std::vector<std::pair<dive *, dive *>> find_consecutive_dive_pairs(const dive_table &dives,
								   int threshold_minutes = 60);

// Calculate surface interval between two dives in seconds
// Returns negative value if dives overlap
int calculate_surface_interval(const dive &first, const dive &second);

// Check if two dives are within the threshold to be considered consecutive
bool are_dives_consecutive(const dive &first, const dive &second, int threshold_minutes);

#endif // CONSECUTIVE_DIVE_DETECTOR_H
