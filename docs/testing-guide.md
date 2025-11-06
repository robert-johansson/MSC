# Testing & Demonstration Guide

MSC ships with a comprehensive regression harness and a set of interactive demos that exercise key reasoning behaviours. This document explains how to run them, what each covers, and how to extend the suite.

---

## 1. Command-Line Options

| Command | Description |
| --- | --- |
| `./MSC --list-tests` | Prints the names of the regression helpers registered in `src/tests_regression.c`. |
| `./MSC --test <name>` | Runs a single helper by name. |
| `./MSC --run-all-tests` | Executes every helper sequentially (the previous default behaviour). |
| `./MSC --exp1-csv <file>` | Reproduces Experiment 1 (baseline→training→testing) and logs each trial to a CSV file. |
| `./MSC --exp2-csv <file>` | Reproduces Experiment 2 (changing contingencies) and logs each trial to a CSV file. |
| `./MSC --help` or `./MSC -h` | Prints usage plus the test list. |
| `./MSC pong` / `pongX` | Launches the visual/headless Pong demo. |
| `./MSC pong2` / `pong2X` | Launches the multi-op Pong variant. |
| `./MSC testchamber` | Starts the interactive Test Chamber environment. |
| `./MSC alien` | Runs the Space-Invaders-inspired environment. |

Headless variants (`pongX`, `pong2X`) mute per-input logging for easier inspection.

---

## 2. Regression Helpers Overview (`src/tests_regression.c`)

| Test | Purpose |
| --- | --- |
| `Stamp_Test` | Verifies stamp combination and overlap checking. |
| `FIFO_Test` | Ensures the event buffers handle rollover, sequencing, and term integrity. |
| `PriorityQueue_Test` | Checks concept attention heap behaviour and eviction order. |
| `Table_Test` | Validates implication storage, revision, and sorting within tables. |
| `MSC_Alphabet_Test` | Exercises repeated belief insertion and cycle stepping. |
| `MSC_Procedure_Test` | Demonstrates single-step procedure learning and execution. |
| `Memory_Test` | Covers conceptualisation, concept lookup, and event storage. |
| `MSC_Follow_Test` | Runs a long simulation verifying action selection and score keeping. |
| `MSC_Multistep_Test` | Confirms multistep procedure formation with positive feedback. |
| `MSC_Multistep2_Test` | Similar to the above but with separated training phases. |
| `Sequence_Test` | Validates multi-operator sequence learning and selection. |
| `MSC_Exp1_Test` (`exp1`) | Reproduces the simple discrimination experiment (baseline → training → testing). |
| `MSC_Exp1_TrainingOnly` (`exp1_training`) | Runs only the feedback-based training phase as a quick smoke check. |

Each helper resets MSC via `MSC_INIT()` to keep runs isolated.

---

## 3. Demos (`src/demos_*.c`)

### Pong / Pong2
- Register left/right/stop operations and stream ball positions as beliefs.
- Evaluate MSC’s ability to learn paddle control.
- Headless modes (`pongX`, `pong2X`) print only hit/miss counters for metrics gathering.

### TestChamber
- Console-driven environment with switches, doors, and lights.
- Demonstrates explicit goal injection and multi-step planning.
- Useful for testing new implication rules—watch how the agent updates beliefs and goals on every command.

### Alien
- Continuous “Space Invaders” scenario where MSC must align and shoot.
- Highlights probabilistic decision making and updating of negative evidence.

### Simple Discriminations
- Minimal categorisation task mapping `A` to `op_left` and `B` to `op_right`.
- Demonstrates expectation growth for procedural implications as the agent receives rewards.

---

## 4. Adding New Regression Tests

1. Implement the helper in `src/tests_regression.c`. Keep it idempotent:
   - Call `MSC_INIT()` (and tweak global parameters if needed).
   - Inject beliefs/goals/operations.
   - Use `assert` (from `src/Globals.h:6`) to document expectations.
2. Declare the function in `src/tests.h`.
3. Extend the `kRegressionTests` array in `src/main.c` with the new name.
4. Run `./MSC --test <name>` to verify the helper runs in isolation.

Avoid calling demos from regression tests—these should stay fast and deterministic.

---

## 5. Analysing Failing Tests

- **Assertions**: Failures call `assert` which prints the message and halts. Start by matching the message to its source in `src/tests_regression.c` or the underlying module.
- **Verbose Output**: Set `OUTPUT = 1` (global in `src/main.c` or temporarily inside your test) or enable logging in the relevant modules to inspect intermediate steps.
- **Input Logging**: For demos, call `MSC_SetInputLogging(true)` before running to re-enable per-input echoes.

---

## 6. Writing Exploratory Tests

If you need to understand a specific module:
1. Create a sandbox helper in `src/tests_regression.c`.
2. Instantiate the structure directly (e.g., `FIFO fifo = {0};`).
3. Call its API functions and assert invariants (`itemsAmount`, `priority`, etc.).

Keep exploratory checks either behind `#ifdef` guards or remove them after you finish to avoid bloating the standard suite.

---

## 7. Continuous Verification

Before committing:
1. Run `./build.sh`.
2. Run `./MSC --run-all-tests`.
3. Optionally run the demos headless to ensure the loops behave as expected (especially after modifying decision logic or inference rules).

This mirrors the workflow used to validate the architecture refactor and CLI changes.
