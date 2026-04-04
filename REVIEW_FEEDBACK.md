# Review Feedback: Consolidate Dive Sites Wizard

-- **Clustering Algorithm:** The current O(N^2) expansion algorithm is lazy. While site counts are manageable for now, we should at least use a more efficient Spatial Index or a simple sorted scan if we expect this to scale.
- - **UI Confirmation:** The prompt for purging unused sites after *each* consolidation is irritating. Add a "Auto-purge unused sites" checkbox to the main dialog instead.
-  - **Testing:** There are no automated tests for the clustering logic. We can't merge this without verifying that it doesn't merge Bali with Bonaire by accident.
-  - **Code Style:** Mixing `vector` and `QVector` is sloppy. Stick to `std::vector` where possible, as per the project's direction.
-  - **Error Handling:** What happens if the merge command fails? The UI should reflect the actual state of the document.

---
Prepare a fix for these issues before resubmitting.
