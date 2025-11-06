# MSC Architecture Overview

MSC implements a compact Non-Axiomatic Reasoning System (NARS) loop that can perceive events, manage procedural knowledge, and select operations under resource constraints. This document explains how the pieces fit together so you can trace control flow from sensor input to motor execution.

---

## 1. Build & Runtime Entry Points

- `./build.sh` compiles every `src/*.c` module into the `MSC` binary with aggressive optimisation and dead-code elimination. (See `build.sh:1`.)
- `./MSC` without arguments now prints CLI usage. (`src/main.c:61`)
- `./MSC --list-tests` enumerates the regression helpers exported by `src/tests_regression.c`.
- `./MSC --test <name>` runs a single regression helper by name.
- `./MSC --run-all-tests` executes the full suite that used to run by default.
- `./MSC pong`, `pongX`, `pong2`, `pong2X`, `testchamber`, `alien` launch the demos in `src/demos_*.c`.

`src/main.c` handles the CLI, registers regression tests, and leaves the heavy lifting to the reasoning modules.

---

## 2. Core Execution Pipeline

MSC follows the same pattern each cycle (`src/Cycle.c:1`):

1. **Input Injection**  
   External code submits events through `MSC_AddInputBelief` or `MSC_AddInputGoal` (`src/MSC.c:28`). Each call:
   - wraps the term, truth-value, and timestamp in an `Event` (`src/Event.c:10`);
   - stores it in the corresponding `FIFO` buffer (`src/FIFO.c`);
   - triggers a single reasoning cycle via `MSC_Cycles(1)`.

2. **Concept Activation**  
   `Cycle_Perform` pulls the newest belief events, ensures their concepts exist (`Memory_Conceptualize`), and updates concept usage statistics before activating inference (`src/Cycle.c:25`).

3. **Link Mining & Inference**  
   - Event sequences are mined for temporal implications `<(&/,pre,op) =/> post>` (`src/Cycle.c:80`), which become the building blocks for future control decisions.
   - Goal events propagate backwards across stored implications for up to `PROPAGATION_ITERATIONS` steps, allowing MSC to reason about required preconditions (`src/Cycle.c:103`).

4. **Decision Making**  
   `Decision_Suggest` (via `Decision_BestCandidate` and optional motor babbling) evaluates candidate operations based on their expected truth-value contribution toward the current goal (`src/Decision.c:59`).  
   If an operation passes the `DECISION_THRESHOLD`, `Decision_Execute` calls the registered callback and logs the execution as a new belief event (`src/Decision.c:9`).

5. **Cleanup & Prioritisation**  
   After each cycle, processed spikes are cleared, and the global concept priority queue is rebuilt to keep highly used concepts near the top (`src/Cycle.c:180`).

---

## 3. Memory Layout & Attention

### 3.1 Global Structures (`src/Memory.c`)
- **Concept Store**: Fixed-size array (`concept_storage`) plus a priority queue interface (`concepts`). Each `Concept` contains:
  - Permanent term descriptor and ID (`src/Concept.c`);
  - Buffers for belief events, goal spikes, precondition tables, and operations.
- **Event Buffers**: Two `FIFO`s keep recent belief and goal events (`src/FIFO.c`).
- **Operations**: Fixed array of registered procedures, each with the term it realises and a function pointer (`src/Memory.h:25`).
- **Parameters**: Propagation thresholds and iteration counts govern how aggressively goals backchain through the implication tables.

Priority queue entries store pointers to concepts plus a `Usage` record that tracks the recency and frequency of activation (`src/Usage.c`). This mirrors the attention mechanisms of later OpenNARS for Applications releases, albeit in a single heap.

### 3.2 Event Processing
- Belief events update the concept’s `belief_spike` for immediate reasoning.
- Goal events are held as pending spikes so `Cycle_PropagateSpikes` can drive anticipations and sequence planning.
- Each `Event` carries a `Stamp` (origin timestamps) to prevent double counting and an implicit occurrence time (`src/Stamp.c`).

---

## 4. Inference Primitives (`src/Inference.c`)

MSC applies a compact subset of the temporal and procedural rules from **NAL-7** and **NAL-8**:

| Function | Role |
| --- | --- |
| `Inference_BeliefIntersection` | Combines sequential events into compound terms (sequence formation). |
| `Inference_BeliefInduction` | Learns `<pre =/> post>` implications with temporal offsets. |
| `Inference_EventRevision` | Revises duplicate events to improve confidence. |
| `Inference_ImplicationRevision` | Maintains consistent implication truth/offsets. |
| `Inference_GoalDeduction` | Backchains goals across implications. |
| `Inference_OperationDeduction` | Estimates the desirability of an operation achieving a goal. |
| `Inference_IncreasedActionPotential` | Merges multiple goal spikes while checking for evidential overlap. |
| `Inference_BeliefDeduction` | Applies implications to forward project beliefs. |

Truth calculus lives in `src/Truth.c`; it supplies frequency/confidence metrics, revision, induction, deduction, expectation, and projection operators.

---

## 5. Decision Layer (`src/Decision.c`)

- **Motor Babbling**: Randomly tries an operation when knowledge is insufficient (`Decision_MotorBabbling`), seeding exploration.
- **Operation Evaluation**: For each candidate implication of the goal concept, `Decision_BestCandidate`:
  1. Checks whether the precondition concept currently has a belief spike.
  2. Projects the truth of reaching the goal via the stored implication.
  3. Keeps the operation with the highest expected value above `DECISION_THRESHOLD`.
- **Negative Evidence**: `Decision_AssumptionOfFailure` injects low-confidence implications when an operation fails, reducing its future priority.

The `Decision` struct bundles the chosen operation ID, its desired expectation value, and a pointer to the actual callback (`src/Decision.h:42`).

---

## 6. Knowledge Representation

- **Terms**: Compact sequences of up to two glyph codes (`src/Term.h`). `Encode_Term` assigns unique IDs to strings at runtime.
- **Events**: Instances of beliefs or goals with truth values and stamps (`src/Event.h`).
- **Implications**: Temporal conditional knowledge `<(&/,pre,op,+Δt) =/> post>` stored per concept per operation (`src/Implication.h`).
- **Tables**: Each concept keeps a `Table` of implications for every operation ID (`src/Table.c`), limited to `TABLE_SIZE`.
- **Truth**: Pairs of frequency/confidence values with expectation helper (`src/Truth.c`).
- **Stamps**: Evidence tracking to avoid cyclic self-support (`src/Stamp.c`).
- **Usage**: Recency-based priorities that feed the concept queue (`src/Usage.c`).

See `docs/data-structures.md` for per-field explanations.

---

## 7. CLI Demos & Regression Harness

### Regression Tests (`src/tests_regression.c`)
The suite exercises FIFO behaviour, table sorting, stamp merging, memory conceptualisation, multistep procedure learning, and sequence execution. These tests mirror the ones run from `main.c` and are safe to call individually.

### Demos (`src/demos_*.c`)
- **Pong / Pong2**: Visual and headless sensorimotor games demonstrating operation selection under uncertainty. (`src/demos_pong.c`)
- **TestChamber**: Interactive console world with switches and lights requiring multi-step procedures. (`src/demos_testchamber.c`)
- **Alien**: Continuous environment showing probabilistic action selection. (`src/demos_alien.c`)

Headless variants disable input logging to keep terminal output manageable.

---

## 8. Extending MSC

1. **Add New Operations**
   - Register the callback via `MSC_AddOperation` in your setup code.
   - Ensure the term describing the operation remains stable so learned implications stay valid.

2. **Integrate Sensors**
   - Wrap incoming observations as terms (`Encode_Term`) and feed them via `MSC_AddInputBelief`.
   - For goals, use `MSC_AddInputGoal`.

3. **Modify Parameters**
   - Tweak thresholds in `src/Decision.h` or `src/Memory.h` (e.g., `PROPAGATION_ITERATIONS`) to adjust planning depth and attention.

4. **Add Inference Rules**
   - Extend `src/Inference.c` with new derivations.
   - Update the cycle to call them at the appropriate stage.

5. **Logging**
   - Gate output with `OUTPUT` or `MSC_SetInputLogging` to keep console noise under control.

---

## 9. Relationship to OpenNARS for Applications (ONA)

MSC shares NARS fundamentals with ONA—concept/event memory layout, implication induction, priority-driven attention—but omits higher-level attention layers, rich procedural search, and extensive parameterisation. Treat it as a minimal, auditable substrate useful for experimenting with new inference rules or control strategies before porting them to full ONA.
