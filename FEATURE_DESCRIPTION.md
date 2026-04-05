# Feature: Merge Consecutive Dives Wizard

## Description
A wizard to identify and merge dives that occurred within a specified time frame (default: 60 minutes).

This feature helps users who have multiple dive computers or who want to combine done dives that should have been a single diving activity.

## Implementation Plan
1.  **Scanning Algorithm:** Design a helper to find candidate dives in the divelog based on time separation.
2.  **Wizard UI:** Implement a new `PresentMergeDivesWizard` to display candidate sets for merging.
3.  **Site Conflicts:|** Handle dive site selection when merged dives have different sites associated.
4.  **Data Merge:|** Use `develog.dives.merge_dives` and `Command::mergeDives` for a seamless experience.
5.  **Validation:** Extensive testing with different dive profiles, deco data, and samples.

## Branch Strategy
- Branch: `feature/merge-consecutive-dives`
- Commits: Atomic commits for algorithm changes, wizard UI, and command integration.
