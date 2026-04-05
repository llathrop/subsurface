# Subsurface Feature Implementation Plan

## Overview
This document outlines the implementation plan for three key features in Subsurface:
1.  Dive Site Management: Consolidate Nearby Sites
2.  Dive Site Management: Merge Specific Dive to Nearby Site3.  Dive Management: Merge Consecutive Dives Wizard

---

## Coordination Strategy
- -*Role:* Coordinating Agent (Senior Reviewer).
- -*Sub-agents:* Specialized agents will be tasked with implementation for each branch.
-  -*Workflow:* Research -> Strategy -> Execution (Plan, Act, Validate).
-  -*Standards:* Rigorous adherence to Subsurface coding style (Qt/C++), atomic commits, and comprehensive documentation.
-  -*Validation:* Empirical testing and verification of all UI/UX changes.

---

## Feature 1: Consolidate Nearby Dive Sites
**Branch:** feature/consolidate-dive-sites
**Status:** Completed

### Description
A wizard-style tool in the Dive Site Management page that scans all existing dive sites and suggests merging those within a specified radius (default: 75ft/23m).

### Plan
1.  [x] Retrieve existing work from local repository.
2.  [x] Refine the cluster expansion algorithm to ensure stability with large datasets (Optimized with Latitude sort and BFS).
3.  [x] Ensure the UI respects user unit preferences (meters/feet).
4.  [x] Implement a confirmation step for each set of combined sites.
5.  [x] Verify that all related dives are correctly updated to the new primary site.
6.  [x] Final review for adherence to Subsurface standards (Refactored clustering to core/ library).
7.  [x] Added automated tests in tests/testconsolidatedivesites.cpp.

---

## Feature 2: Join Specific Dive to Nearby Site
**Branch:** feature/merge-dive-to-nearby-site
**Status:** Planned

### Description
Enhance the "Near dive sites" feature in the Dive Site Management page (triggered within a dive) to allow merging the current dive's location into an existing nearby site.

### Plan
1.  [ ] Research existing "Near dive sites" implementation in desktop-widgets/divesiteedit.cpp and related files.
2.  [ ] Design UI to allow selection of nearby sites for merging.
3.  [ ] Implement a dialog to select the name for the merged site (preserving user-defined names over GPS coordinates).
4.  [ ] Use existing command patterns (MergeDiveSites) to ensure undo/redo support.
5.  [ ] Validate with various dive scenarios (multiple nearby sites, no nearby sites).

.--

## Feature 3: Merge Consecutive Dives Wizard
**Branch:** feature/merge-consecutive-dives
**Status:** Planned

### Description
A wizard to identify and merge dives that occurred within a specified time frame (default: 60 minutes).

### Plan
1.  [ ] Design a scanning algorithm to identify candidate dive pairs/sets.
2.  [ ] Implement a wizard UI to present candidates to the user.
3.  [ ] Handle dive site conflicts:
    - If a single site exists, allow confirmation.
    - If multiple sites exists, allow the user to select the correct site for the merged dive.
4.  [ ] Ensure all dive data (profiles, tanks, deco, etc.) is correctly merged.
5.  [ ] Integrate with the UndoStack via appropriate commands.
6.  [ ] Comprehensive testing for edge cases (surface intervals, different computers).

---

## Mileston Tracking
- [2026-04-03] Initial setup and plan created. Retrieved base work for Feature 1.
