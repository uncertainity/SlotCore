# Trace Validation

Source inputs:
- `checks/traces/test_traces.txt`
- `checks/traces/trace_contract.txt`
- `checks/traces/trace_report.txt`
- `checks/traces/trace_validator.cpp`
- `reference/cpp/trace_checker.cpp`
- `artifacts/implementation/core.cpp`

Validator behavior: each test trace is parsed into trace roots and matched against each contract scenario. Matching requires compatible function names, input shapes, global names/shapes, output shapes, and ordered child subtrees. Extra contract children may be skipped; extra test children must still match.

## Failed Test Traces

### test_core: TC-011 Free Spins retrigger adds five spins

- Test Trace: `test_core: TC-011 Free Spins retrigger adds five spins`
- Matched Scenario: `BaseGame_FreeGameFeature1` subtree `FreeGameFeature_1`
- Failure Point: `FreeGameFeature_1` emits six `waysWinCalculationWithMultiplier` child calls; the contract subtree contains one matching `waysWinCalculationWithMultiplier` child.
- Likely Cause: The test uses forced free-spin windows to cover retrigger behavior. `FreeGameFeature_1` loops once per awarded spin, while the contract scenario models a single feature-spin evaluation shape.

### run_forced_window: CASE_EDGE_003 runOneSpin FreeGameFeature_1

- Test Trace: `run_forced_window: CASE_EDGE_003 runOneSpin FreeGameFeature_1`
- Matched Scenario: `BaseGame_FreeGameFeature1`
- Failure Point: Inside `FreeGameFeature_1`, the trace emits eight `waysWinCalculationWithMultiplier` child calls; the contract scenario contains one matching child call.
- Likely Cause: The forced base window triggers an 8-spin FreeGameFeature_1 session. Forced windows bypass `generatePayWindow`, and the feature loop emits one evaluator call per spin.

### run_forced_window: CASE_EDGE_004 runOneSpin FreeGameFeature_2

- Test Trace: `run_forced_window: CASE_EDGE_004 runOneSpin FreeGameFeature_2`
- Matched Scenario: `BaseGame_FreeGameFeature2`
- Failure Point: Inside `FreeGameFeature_2`, the trace emits four `resolveCollect` child calls; the contract scenario contains one matching `resolveCollect` child.
- Likely Cause: The forced base window triggers a 4-spin Blitz session. `FreeGameFeature_2` loops once per awarded spin and resolves collect once per Blitz spin.

### run_forced_window: CASE_EDGE_010 FreeGameFeature_1 retrigger boundary

- Test Trace: `run_forced_window: CASE_EDGE_010 FreeGameFeature_1 retrigger boundary`
- Matched Scenario: `BaseGame_FreeGameFeature1` subtree `FreeGameFeature_1`
- Failure Point: `FreeGameFeature_1` emits many repeated `waysWinCalculationWithMultiplier` child calls after a retrigger; the contract subtree contains one matching evaluator child.
- Likely Cause: The first forced free-spin window has six SCAT symbols, causing a maximum retrigger award. The implementation continues the feature loop and traces each spin separately.

### run_forced_window: CASE_EDGE_011 FreeGameFeature_2 multi-spin

- Test Trace: `run_forced_window: CASE_EDGE_011 FreeGameFeature_2 multi-spin`
- Matched Scenario: `BaseGame_FreeGameFeature2` subtree `FreeGameFeature_2`
- Failure Point: `FreeGameFeature_2` emits two `resolveCollect` child calls; the contract subtree contains one matching `resolveCollect` child.
- Likely Cause: The test calls `FreeGameFeature_2(2)` directly with two forced Blitz windows, so the implementation traces two per-spin collect resolutions.
