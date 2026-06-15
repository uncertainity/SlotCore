# Specification Validation Escalations

## Planner-Verifier Run - 2026-06-14

[WARNING] The verifier precondition requiring no unresolved blocker marker in `./checks/reports/escalation.md` was not satisfied at stage start because prior unresolved blocker entries already existed. The stage was rerun because it was explicitly requested.

[WARNING] Maximum bet is not specified in the DOCX game rules, parsheet, planner markdown, or planner JSON.

[WARNING] Coin value and denomination rules are not specified as authoritative simulator configuration in the DOCX game rules, parsheet, planner markdown, or planner JSON. Oracle examples contain coin values, but they are not a complete denomination contract.

[RESOLVED] Paytable payout units blocker resolved by human review on 2026-06-14. Human decision: if SCATTER wins are not mentioned, no direct SCATTER win is awarded when SCATTER occurs. Existing COR behavior remains unchanged: collected COR values are multiplied by total bet. Standard-symbol paytable factors remain the authoritative paytable factors for ways evaluation.

[RESOLVED] Reel-stop visible-window extraction convention blocker resolved by human review on 2026-06-14. Human decision: use the same convention as `reference/cpp/reflib.cpp`. `start_idx` is the top visible symbol for the reel, rows are filled downward as `idx = (start_idx + row) % reel_length`, wrapping forward through the strip, and the visible window is stored row-major as `pay_window[row][reel]`.

[WARNING] Oracle workbook test coverage is incomplete for simulator-core verification. It contains payout examples, QA scenarios, staging observations, and UI/API checks, but not complete deterministic full-window tests for every base and feature path.

[WARNING] Mathematical acceptance thresholds are incomplete. RTP values, a feature hit-rate value, and STD values are available in the parsheet/oracle notes, but jackpot frequency targets and simulator pass/fail tolerances for feature frequencies and volatility are not fully defined.

[AMBIGUITY] Buy Bonus data exists in the parsheet, oracle workbook, and staging notes, but the DOCX rules do not describe Buy Bonus availability or player-facing behavior. Human review must confirm whether Buy Bonus is in simulator scope.

[AMBIGUITY] Parsheet references to `CB spins` appear to refer to Blitz/VB Spins based on surrounding labels and DOCX rules, but the source vocabulary is inconsistent.

## Simulator Design Run - 2026-06-14

[RESOLVED] Stale Simulator Design blocker resolved by the Simulator Refinement pass on 2026-06-14. The original blocker stated that Simulator Design could not verify `trace_ref.txt` / `ref_trace.txt` against Planner outputs because neither `./checks/traces/trace_ref.txt` nor `./checks/traces/ref_trace.txt` existed. `./checks/traces/ref_trace.txt` now exists and was used as the canonical trace reference for generating `./artifacts/implementation/coder_contract.md` and `./checks/traces/trace_contract.txt`.

[WARNING] The user-requested raw input paths `./specs/game_rule/` and `./specs/math_model/` do not exist in this repository. The repository contains `./specs/game_rules/` and `./specs/math_models/`, matching `AGENTS.md`; those paths were listed and treated as the available source-document locations.

## Simulator Refinement Pass - 2026-06-14

[RESOLVED] The prior Simulator Design blocker stating that neither `./checks/traces/trace_ref.txt` nor `./checks/traces/ref_trace.txt` exists is stale for this pass. `./checks/traces/ref_trace.txt` now exists and was used as the canonical trace reference for generating `./artifacts/implementation/coder_contract.md` and `./checks/traces/trace_contract.txt`.

[RESOLVED] Prior trace filename warning resolved by human update on 2026-06-14. The corrected reference trace file is now `./checks/traces/ref_trace.txt`, and the machine trace contract is `./checks/traces/trace_contract.txt`; `./checks/traces/corrected_trace_ref.txt` is no longer expected.

[RESOLVED] Prior generated-test filename warning resolved by human update on 2026-06-14. The simulator-stage optional generated-test input remains `./checks/generated_tests/proposed_test_cases.txt` per `./skills/simulator_design.md`; `./checks/generated_tests/test_cases.txt` is no longer expected for this pass.

## Implementation Generation Run - 2026-06-14

[RESOLVED] Primary implementation contract path blocker resolved by human update on 2026-06-14. `./artifacts/contract/coder_contract.md` now exists and is byte-identical to `./artifacts/implementation/coder_contract.md`; the Coder stage should continue treating `./artifacts/contract/coder_contract.md` as the primary implementation contract.
