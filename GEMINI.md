# Subsurface Feature Coordination Plan (GEMINI.md)

## Project Overview
Implementing three key dive and site management features for the Subsurface dive log application.

---

## Agent Coordination Strategy
To ensure efficiency and maintain high engineering standards, the work is divided between a Coordinating Agent and specialized sub-agents.

### 1. Roles
- **Coordinating Agent (Senior Reviewer):**
    - Maintains this `GEMINI.md` master plan.
    - Manages the integration branch: `feature/merged-features`.
    - Performs final architectural reviews and ensures "Consolidate vs. Merge" terminology consistency.
    - Handles cross-cutting changes (e.g., refactoring common logic to `core/`).
- **Implementation Sub-agents:**
    - Assigned to specific feature branches.
    - Responsible for the **Plan -> Act -> Validate** cycle for their feature.
    - Must provide automated tests in the `tests/` directory.

### 2. Parallel Execution Mandate
- Features 1, 2, and 3 are independent enough to be implemented in parallel on their respective branches.
- Once a feature is validated (tests pass), it is merged into `feature/merged-features`.

---

## Technical Standards & Workflow
- **Coding Style:** Rigorous adherence to Subsurface (Qt/C++) standards. Prefer `std::vector` over `QVector` where possible.
- **Atomic Commits:** Every logical step (algorithm change, UI update, test addition) must be a separate, clean commit.
- **Validation:** 
    - **Automated:** New tests must be added to `tests/CMakeLists.txt` and pass.
    - **UAT (User Acceptance Testing):** Every task is incomplete until a `uat_test.sh` script is provided to guide the user through manual UI verification.

---

## Feature Roadmap

### [COMPLETE] Feature 1: Consolidate Nearby Dive Sites
- **Branch:** `feature/consolidate-dive-sites`
- **Core Logic:** `core/divesite_cluster.cpp` (Optimized BFS/Latitude-sort).
- **UI:** `ConsolidateDiveSitesDialog` with Auto-purge support.
- **Verification:** `tests/testconsolidatedivesites.cpp` & `uat_test.sh`.

### [PLANNED] Feature 2: Join Specific Dive to Nearby Site
- **Branch:** `feature/merge-dive-to-nearby-site`
- **Objective:** Enhance the "Near dive sites" UI in `LocationInformationWidget` to allow merging a dive's location into an existing nearby site.
- **Strategy:** Reuse `Command::mergeDiveSites` for undo/redo support.

### [PLANNED] Feature 3: Merge Consecutive Dives Wizard
- **Branch:** `feature/merge-consecutive-dives`
- **Objective:** Detect dives within ~60 mins and offer a wizard to merge them.
- **Strategy:** Implement scanning algorithm in `core/` and a new wizard UI in `desktop-widgets/`.

---

## Build & Test Environment
- **Build Directory:** `/home/llathrop/.gemini/tmp/llathrop/subsurface_build`
- **Critical Dependency:** `libdivecomputer`
    - Include: `/home/llathrop/Projects/subsurface-add_UI-consoildate_dive_sites/install-root/include`
    - Lib: `/home/llathrop/Projects/subsurface-add_UI-consoildate_dive_sites/install-root/lib/libdivecomputer.so`
- **UAT Script:** `./uat_test.sh` (launches Subsurface with correct environment variables).
