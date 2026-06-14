# AGENTS.md

# SlotCore

## Purpose

SlotCore is a staged orchestration pipeline for converting slot-game specifications into:

* Validated game specifications
* Simulator architectures
* Implementation contracts
* Tested C++ implementations
* Trace-verifiable execution flows
* Mathematical analysis and tuning artifacts

The repository is organized around explicit contracts and validation gates.

Agents should operate only on the artifacts required for the current stage.

---

# Core Principles

1. Never invent game mechanics.
2. Never invent mathematics.
3. Never invent missing paytables, reel strips, probabilities, or symbols.
4. Escalate uncertainty rather than guessing.
5. Baseline correctness precedes optimization.
6. Contracts are authoritative.
7. Validation is mandatory.
8. Human review overrides assumptions.
9. Every stage must produce explicit artifacts.
10. A fresh session must be able to continue using repository files alone.

---

# Fresh Session Startup

At the start of every new session:

1. Read `./summary.md`
2. Read `./AGENTS.md`
3. Identify the current stage.
4. Read the corresponding skill file.
5. Verify required inputs exist.
6. Continue only from the current stage.

Do not rely on previous chat history.

---

# Repository Structure


.
├── AGENTS.md
├── summary.md
│
├── specs/
│   ├── game_rules/
│   └── math_models/
│
├── reference/
│   ├── cpp/
│   ├── numba/
│   └── examples/
│
├── artifacts/
│   ├── plan/
│   ├── contract/
│   ├── implementation/
│   ├── optimized/
│   └── demo/
│
├── checks/
│   ├── oracle/
│   ├── generated_tests/
│   ├── traces/
│   ├── reports/
│   └── human_review/
│
├── math/
│
├── logs/
│
├── skills/
│
└── agentprompts/

---

# Summary Contract

`summary.md` is the project state file.

It must be updated after every major stage.

Suggested structure:


# SlotCore Summary

## Current Stage

## Completed Stages

## Pending Stages

## Important Decisions

## Known Issues

## Active Blockers

## Generated Artifacts

## Human Review Notes

## Next Recommended Action


---

# Context Policy

Agents must minimize context usage.

Read only:

1. `./summary.md`
2. `./AGENTS.md`
3. The current skill file
4. Required inputs listed in the skill

Do not recursively inspect unrelated directories.

Do not load raw specifications unless the skill explicitly requires them.

---

# Artifact Authority

When conflicts occur, use the following authority order.

Level 1:
summary.md

Level 2:
artifacts/contract/

Level 3:
checks/reports/
checks/traces/

Level 4:
artifacts/plan/

Level 5:
specs/

If multiple sources disagree:

* Do not silently resolve conflicts.
* Write an escalation entry.
* Record the conflict in reports.

---

# Escalation Policy

Escalations must be written to:

./checks/reports/escalation.md

Allowed diagnostics:

[WARNING]
[AMBIGUITY]
[BLOCKER]

Examples:

[WARNING] Optional probability table missing.

[AMBIGUITY] Symbol SCAT and SCATTER both appear.

[BLOCKER] Paytable values missing.

Agents must not invent information to bypass a blocker.

---

# Pipeline Topology

Default pipeline:


Game Specification
→ Specification Validation
→ Simulator Design
→ Contract Refinement
→ Implementation Generation
→ Test Generation
→ Trace Generation
→ Trace Validation
→ Math Validation


Stages may be executed individually when explicitly requested.

---

# Human Review Gates

The repository includes explicit human-review checkpoints.

Examples:


planner_verification_loop
simulator_human_gate
interactive_pass
consensus_loop

Human review may:

* Resolve blockers
* Clarify requirements
* Accept corrected traces
* Refine contracts

Human decisions should be recorded in:


checks/human_review/

---

# Completion Rules

A stage is complete only if:

* Required outputs exist.
* Required outputs are non-empty.
* Escalations are recorded.
* Summary is updated.
* Downstream inputs are available.

---

# Logging Policy

Pipeline execution generates:

logs/pipeline_*.log

Complete execution log.

logs/summary_*.log

High-level stage progress log.

checks/human_review/interactive_pass_hist.txt

History of human interventions.

---

# Skill Ownership

`skills/*.md` define how a stage operates.

Examples:


skills/game_specification.md
skills/specification_validation.md
skills/simulator_design.md
skills/implementation_generation.md
skills/test_generation.md
skills/math_validation.md

AGENTS.md must not duplicate stage-specific logic.

---

# Core Principle

AGENTS.md defines repository behavior.

skills/*.md define stage behavior.

summary.md defines project state.

Do not confuse these responsibilities.
