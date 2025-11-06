# Data Structures & Modules

This reference lists the most important structs, arrays, and helper modules with source pointers so you can navigate the implementation quickly.

---

## Term System

| Structure / Function | Description | Location |
| --- | --- | --- |
| `Term` | Fixed-length array (`char terms[2]`) storing a short symbol sequence. | `src/Term.h:22` |
| `Encode_Term` | Maps string names to unique `Term` IDs on demand. | `src/Encode.c:4` |
| `Term_Sequence` | Concatenates two terms (used for building `(&/,pre,op)` constructs). | `src/Term.c:16` |

Terms are deliberately tiny to avoid hashing; they index into implication tables and event descriptions.

---

## Truth & Stamp

| Structure / Function | Description | Location |
| --- | --- | --- |
| `Truth` | Pair of `frequency` and `confidence`. | `src/Truth.h:13` |
| `Truth_Expectation` | Converts a `Truth` to a scalar utility (used for decisions). | `src/Truth.c:115` |
| `Truth_Revision`, `Truth_Deduction`, `Truth_Induction`, `Truth_Intersection`, `Truth_Projection` | Implement NAL truth-value calculus. | `src/Truth.c` |
| `Stamp` | Tracks origin evidence (`evidentalBase`) to avoid overlap. | `src/Stamp.h:11` |
| `Stamp_make`, `Stamp_checkOverlap`, `Stamp_print` | Utility functions for stamps. | `src/Stamp.c` |

Every event and implication carries both a truth-value and a stamp; inference helpers combine them consistently.

---

## Events & FIFOs

| Structure / Function | Description | Location |
| --- | --- | --- |
| `Event` | Represents belief or goal evidence, including `Term`, `Truth`, `Stamp`, timestamps, and an optional operation ID. | `src/Event.h:18` |
| `Event_InputEvent` | Constructs new events with fresh stamps. | `src/Event.c:10` |
| `FIFO` | Circular buffer storing recent events per channel (`belief_events`, `goal_events`). | `src/FIFO.h`, `src/FIFO.c` |
| `FIFO_Add`, `FIFO_GetNewestSequence`, `FIFO_GetKthNewestSequence` | Manage sequences of events for implication mining. | `src/FIFO.c` |

`FIFO_SIZE` and `MAX_SEQUENCE_LEN` limit how far back MSC looks when forming temporal relations.

---

## Concepts & Memory

| Structure / Function | Description | Location |
| --- | --- | --- |
| `Concept` | Holds the term’s knowledge: belief spike, goal spike, implication tables per operation, and usage metrics. | `src/Concept.h:24` |
| `concepts` | Priority queue of `Item{void *address, double priority}` referencing concepts. | `src/Memory.c:6`, `src/PriorityQueue.c` |
| `Memory_Conceptualize` | Ensures a concept exists for a term, creating one if necessary. | `src/Memory.c:36` |
| `Memory_addEvent` | Inserts an event into belief or goal FIFOs. | `src/Memory.c:60` |
| `Memory_addOperation` | Registers a callable operation (`Operation{Term, Action}`). | `src/Memory.c:79` |
| `Usage` | Struct capturing `useCount` and `lastUsed` time, converted to priorities. | `src/Usage.h`, `src/Usage.c` |

Each concept owns an array of implication tables indexed by operation ID (`precondition_beliefs[OPERATIONS_MAX]`).

---

## Implications & Tables

| Structure / Function | Description | Location |
| --- | --- | --- |
| `Implication` | Stores a conditional inference: source term, truth, stamp, occurrence offset, debug string, and resolved `Concept` pointer. | `src/Implication.h:14` |
| `Table` | Fixed-size (FIFO-like) container for implications with revision logic. | `src/Table.h`, `src/Table.c` |
| `Table_AddAndRevise` | Inserts an implication, revising existing entries when stamps overlap. | `src/Table.c:58` |
| `Cycle_ReinforceLink` | Builds `<(&/,pre,op,+dt) =/> post>` implications from event sequences. | `src/Cycle.c:120` |

Implications underpin both planning (goal deduction) and motor decision heuristics.

---

## Priority Queue

| Structure / Function | Description | Location |
| --- | --- | --- |
| `PriorityQueue` | Binary heap over `Item{void *address, double priority}`. | `src/PriorityQueue.h:11` |
| `PriorityQueue_Push`, `PriorityQueue_Pop`, `PriorityQueue_Rebuild` | Standard heap operations, with feedback about evicted entries. | `src/PriorityQueue.c` |

The concept queue enables limited-resource attention: recently useful concepts bubble to the top.

---

## Decision & Operations

| Structure / Function | Description | Location |
| --- | --- | --- |
| `Decision` | Contains `operationID`, `desire`, `execute` flag, and resolved `Operation`. | `src/Decision.h:42` |
| `Decision_Suggest` | Aggregates motor babbling and implication-based selection. | `src/Decision.c:102` |
| `Decision_AssumptionOfFailure` | Penalises failing operations by updating implication confidences. | `src/Decision.c:81` |
| `MSC_AddOperation` | Binds a term to a function pointer; called during demo initialisation. | `src/Memory.c:75` |

Operations are index-aligned: `operationID` 1 corresponds to `operations[0]`, etc.

---

## Inference Helpers

See `src/Inference.h` for prototypes. Key data dependencies:

- All inference functions expect consistent stamps; they reject overlapping evidence (`Stamp_checkOverlap`).
- Temporal offsets are measured in cycles and stored as `long occurrenceTimeOffset` inside implications.
- `Truth_Eternalize` converts projected truths into timeless confidence values for concept linkage.

---

## Parameter Summary

| Parameter | Purpose | Default | Location |
| --- | --- | --- | --- |
| `FIFO_SIZE` | Event buffer length per channel. | 1024 | `src/FIFO.h` |
| `TABLE_SIZE` | Max implications per concept/operation. | 32 | `src/Table.h` |
| `PROPAGATION_THRESHOLD` | Minimum expectation for goal spike propagation. | 0.501 | `src/Memory.h` (runtime adjustable) |
| `PROPAGATION_ITERATIONS` | Depth of goal backchaining per cycle. | 5 | `src/Memory.h` |
| `DECISION_THRESHOLD` | Minimum expectation required to execute an operation. | 0.6 | `src/Decision.h` |
| `MOTOR_BABBLING_CHANCE` | Probability of random operation execution. | 0.2 | `src/Decision.h` |
| `MSC_InputLoggingEnabled` | Global toggle for input echoing (headless demos disable it). | true | `src/MSC.c:3` |

Adjust these to experiment with attention, planning depth, or exploration behaviour.

---

## File Map

| Module | Responsibility |
| --- | --- |
| `src/main.c` | CLI wiring, regression registry, demo dispatch. |
| `src/tests_regression.c` | Regression helpers used by `--test`/`--run-all-tests`. |
| `src/demos_*.c` | Interactive environments illustrating MSC in action. |
| `src/MSC.c` | Public API for initialisation, input injection, and cycle stepping. |
| `src/Cycle.c` | Core reasoning loop (event processing, inference, decision execution). |
| `src/Memory.c` | Concept store, FIFOs, operation registry, conceptualisation. |
| `src/Decision.c` | Operation evaluation and failure handling. |
| `src/Inference.c` | Non-axiomatic inference primitives. |
| `src/FIFO.c`, `src/Table.c`, `src/PriorityQueue.c`, `src/Usage.c` | Supporting data structures. |

Use this table alongside the architecture guide to locate code paths when debugging or extending MSC.
