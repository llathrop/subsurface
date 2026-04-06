#!/bin/bash
# Subsurface UAT Script for Dive Site Consolidation

BUILD_DIR="/home/llathrop/.gemini/tmp/llathrop/subsurface_build"
SUBSURFACE_BIN="$BUILD_DIR/subsurface"

if [ ! -f "$SUBSURFACE_BIN" ]; then
    echo "Error: Subsurface binary not found at $SUBSURFACE_BIN"
    echo "Please build the project first."
    exit 1
fi

echo "--- Subsurface UAT: Consolidate Dive Sites ---"
echo "1. Launching Subsurface..."
echo "2. Please open a log file with many nearby dive sites (e.g., from a dive trip)."
echo "3. Go to the 'Dive Sites' list view."
echo "4. Click the 'Consolidate Dive Sites' button (or equivalent menu item)."
echo "5. In the dialog:"
echo "   - Set a distance (e.g., 20m or 60ft)."
echo "   - Click 'Find Sites'."
echo "   - Verify that groups of close sites are found."
echo "   - Select a site and click 'Set as Primary'."
echo "   - Check/Uncheck 'Auto-purge unused sites'."
echo "   - Click 'Consolidate Selected Group' or 'Consolidate All'."
echo "6. Verify in the main dive list that dives have been moved to the primary sites."
echo "7. Verify that unused sites were purged (if selected)."
echo ""
echo "Press Enter to launch Subsurface..."
read

# Set library path for libdivecomputer if needed
export LD_LIBRARY_PATH="/home/llathrop/Projects/subsurface-add_UI-consoildate_dive_sites/install-root/lib:$LD_LIBRARY_PATH"

"$SUBSURFACE_BIN"
