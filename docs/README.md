# MSC Documentation Hub

This directory collects reference material for the Minimal Sensorimotor Component (MSC). It complements the inline comments and regression tests so contributors can understand, extend, and verify the system without reverse‑engineering every source file.

The documentation set is organised as follows:

| File | Purpose |
| --- | --- |
| `architecture.md` | High-level walkthrough of MSC’s control loop, memory layout, and inference workflow. |
| `data-structures.md` | Detailed descriptions of the core data types (terms, stamps, events, concepts, implications, queues, tables). |
| `testing-guide.md` | Summary of the built-in regression suite and demos, including instructions for adding new checks. |

Start with `architecture.md` for a system overview, then dive into `data-structures.md` when you need implementation details, and finish with `testing-guide.md` to learn how to exercise the code. Feel free to extend this directory as the codebase grows.
