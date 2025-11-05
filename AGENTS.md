# Repository Guidelines

## Project Structure & Module Organization
- `src/` holds all C99 modules; each `*.c` file pairs with a matching header to expose public routines (for example, `Memory.c` and `Memory.h` manage concept storage, while `MSC.c` orchestrates the control loop).
- `main.c` bundles regression-style tests and demo entry points; keep new simulations modular and register them near the `main` command switch.
- `build.sh` compiles every source file into the `MSC` binary using dead-code elimination; adjust compiler flags there rather than compiling ad hoc.
- The root also includes `README.md` for high-level context and `LICENSE` for redistribution terms—update both when adding major capabilities.

## Build, Test, and Development Commands
- `./build.sh` compiles the project with `gcc -std=c99` plus aggressive warnings; fix new warnings before committing.
- `./MSC` runs the default smoke tests embedded in `main.c` and exits with non-zero status if any assertion fails.
- `./MSC pong`, `./MSC pong2`, or `./MSC testchamber` launch interactive demos that validate behavior changes against goal-seeking scenarios.

## Coding Style & Naming Conventions
- Follow the existing four-space indentation and K&R brace placement shown in `src/main.c`.
- Use descriptive CamelCase for functions (`MSC_AddInputGoal`) and ALL_CAPS for macros and global constants (`MSC_DEFAULT_TRUTH`).
- Keep module-level state within the corresponding `*.c` file; expose only the minimal API via headers and include guards.
- Run `./build.sh` after formatting changes to ensure the strict compiler flags stay satisfied.

## Testing Guidelines
- Extend the assertion-based test harness in `src/main.c` by adding `XYZ_Test` helpers and invoking them in `main()`.
- Prefer deterministic seeds (`srand`) when simulating environments so regressions are reproducible.
- When adding new demos, provide a brief usage note in `README.md` and verify they can run after the regression block completes.

## Commit & Pull Request Guidelines
- Model commit messages after the existing log (`Update: <scope>`) so history stays consistent and searchable.
- Group related source and header edits into a single commit; mention affected modules explicitly.
- For pull requests, describe behavioral changes, list reproduction steps (e.g., commands above), and link any tracked issues or experiments.
