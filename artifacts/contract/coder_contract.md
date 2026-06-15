# Vegas Blitz Coder Contract

Simulator refinement pass date: 2026-06-14.

This is the implementation handoff contract for the Coder. It is sufficient for baseline simulator implementation without consulting original game documents. Do not write or infer mechanics outside this contract. Do not modify mathematics.

## Canonical Constants And Symbols

- `NO_OF_REELS = 6`
- `NO_OF_ROWS = 4`
- `STANDARD_SYMBOL_COUNT = 11`
- `PAYTABLE_SYMBOL_COUNT = 12`
- `BASE_BET_FIXED_COINS = 20`

Canonical `Symbol` enum order:

```text
HV1
HV2
HV3
HV4
HV5
LV1
LV2
LV3
LV4
LV5
LV6
WILD
SCAT
COR
COLLECT
```

Alias rule: planner symbol `SCATTER` maps to implementation symbol `SCAT`. All inbound source/planner data using `SCATTER` must be normalized to `SCAT` before simulator evaluation. `SCAT` does not alias any standard symbol.

Standard ways symbols are `HV1, HV2, HV3, HV4, HV5, LV1, LV2, LV3, LV4, LV5, LV6`.

Special-symbol rules:

- `WILD` substitutes for standard ways symbols only.
- `WILD` does not substitute for `SCAT`, `COR`, or `COLLECT`.
- Base-game `WILD` appears only on reels 2 through 6.
- Free Spins `WILD` appears only on reels 2 through 4.
- `COR` values are collected only when at least one `COLLECT` is visible.
- `COLLECT` can land only on reel 1 and/or reel 6 and is active only in base game and Blitz Spins.

## Canonical Reelset Contract

- `BG1_Reels` maps to `planner.reel_strips.base_B1`.
- `BG2_Reels` maps to `planner.reel_strips.base_B2`.
- `FG1_Reels` maps to `planner.reel_strips.free_F1`.
- Each reelset has shape `[6][variable]`.
- Each reelset contains six ordered reel strips, one per reel.
- Each `ReelSize` vector has shape `[6]`.
- Visible-window extraction: for each reel, `start_idx` is the top visible symbol. Fill rows downward with `(start_idx + row) % reel_length`. Store as `pay_window[row][reel]`.
- `pay_window` shape is `[4x6]`.

## Canonical probTable Contract

All weights are integer weighted-choice tables unless stated otherwise. The Coder must preserve the listed order.

| Field | Shape | Values | Purpose |
| --- | --- | --- | --- |
| `ReelsetWeights` | `[2]` | `[58, 42]` for `[B1, B2]` | Select base reelset. |
| `corTrigger` | `[2]` | `[1, 9]` for `[yes, no]` | Decide whether additional base-game COR injection occurs. |
| `numCorWeight` | `[5]` | `[240, 200, 150, 100, 80]` for `[1, 2, 3, 4, 5]` | Select number of additional COR symbols. |
| `wildMult` | `[3]` | weights `[480, 545, 360]` for multipliers `[2, 3, 5]` | Assign Free Spins WILD multipliers. |
| `freeGamenumCollect` | `[2]` | `[75, 25]` for `[1, 2]` COLLECT symbols | Select Blitz Spins COLLECT count. |
| `freeGameCOROccur` | `[2]` | `[26, 74]` for `[yes, no]` | Per available Blitz Spins position COR occurrence. |
| `goodCORTable` | `[22]` | `[0, 0, 0, 0, 0, 0, 0, 606, 893, 600, 600, 500, 500, 500, 500, 400, 300, 100, 30, 10, 10, 3]` | Assign exactly one good COR value per Blitz Spin. |
| `badCORTable` | `[22]` | `[17500, 7000, 3000, 2500, 2000, 2000, 1500, 1000, 1000, 500, 500, 100, 100, 50, 50, 50, 30, 20, 10, 0, 0, 0]` | Assign all non-good Blitz Spin COR values. |
| `CORValues` | `[22]` | `[0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 5, 6, 7, 7.5, 8, 9, 10, 12.5, 25, 50, 125, 250, 500, 2000]` | COR value multipliers of total bet. |
| `baseCORValueWeights.values` | `[22]` | same as `CORValues` | Base-game additional COR value choices. |
| `baseCORValueWeights.weights` | `[22]` | `[7000, 6000, 6000, 5000, 3000, 3000, 2500, 2000, 1000, 1000, 1000, 100, 100, 50, 50, 50, 30, 20, 10, 10, 5, 1]` | Assign base-game visible COR values. |

Jackpot-equivalent COR values are represented in `CORValues`: Mini `10x`, Minor `25x`, Major `250x`, Grand `2000x` total bet.

## PayTable Contract

The paytable is symbol-keyed for:

```text
HV1, HV2, HV3, HV4, HV5,
LV1, LV2, LV3, LV4, LV5, LV6,
SCAT
```

Matrix view shape: `[12x7]`.

- Rows correspond to the 12 payable symbols in the order above.
- Columns correspond to symbol counts `0, 1, 2, 3, 4, 5, 6`.
- Missing counts pay `0`.
- Standard ways symbols use counts according to ways rules.
- `SCAT` row is used for scatter pays.
- `WILD`, `COR`, and `COLLECT` do not have direct paytable rows.
- Human review resolved direct SCAT behavior: if SCATTER wins are not mentioned for a flow, no direct SCATTER win is awarded for that flow. The matrix row remains available for the Coder because planner output contains exact `SCATTER` paytable factors.

| Symbol | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| HV1 | 0 | 0 | 20 | 30 | 40 | 50 | 100 |
| HV2 | 0 | 0 | 10 | 20 | 30 | 40 | 50 |
| HV3 | 0 | 0 | 10 | 20 | 30 | 40 | 50 |
| HV4 | 0 | 0 | 4 | 16 | 24 | 30 | 40 |
| HV5 | 0 | 0 | 4 | 16 | 24 | 30 | 40 |
| LV1 | 0 | 0 | 0 | 10 | 20 | 24 | 30 |
| LV2 | 0 | 0 | 0 | 10 | 20 | 24 | 30 |
| LV3 | 0 | 0 | 0 | 8 | 16 | 20 | 24 |
| LV4 | 0 | 0 | 0 | 8 | 16 | 20 | 24 |
| LV5 | 0 | 0 | 0 | 6 | 10 | 16 | 20 |
| LV6 | 0 | 0 | 0 | 6 | 10 | 16 | 20 |
| SCAT | 0 | 0 | 0 | 20 | 40 | 60 | 100 |

## Function Contracts

### generatePayWindow

Purpose: Build a visible `[4x6]` pay window from a selected six-reel strip set.

Inputs: `Reelset` shape `[6][variable]`, `ReelSize` shape `[6]`, RNG.

Outputs: `pay_window` shape `[4x6]`.

Dependencies: `NO_OF_REELS`, `NO_OF_ROWS`.

Pseudocode:

```text
for reel from 0 to NO_OF_REELS - 1:
  choose start_idx uniformly from 0 to ReelSize[reel] - 1
  for row from 0 to NO_OF_ROWS - 1:
    strip_idx = (start_idx + row) modulo ReelSize[reel]
    pay_window[row][reel] = Reelset[reel][strip_idx]
return pay_window
```

Invariants: Output has exactly 4 rows and 6 reels. Every symbol must be a canonical enum value after SCATTER-to-SCAT normalization.

Trace emission: Emit function entry, `Reelset`, `ReelSize`, `NO_OF_ROWS`, `NO_OF_REELS`, `pay_window`, and function exit.

### additionalCOR

Purpose: Optionally inject extra `COR` symbols into a base-game pay window.

Inputs: mutable `pay_window` shape `[4x6]`, RNG.

Outputs: updated `pay_window` shape `[4x6]`.

Dependencies: `probTable.corTrigger`, `probTable.numCorWeight`, `probTable.baseCORValueWeights.values`, `probTable.baseCORValueWeights.weights`.

Pseudocode:

```text
choose trigger from [yes, no] using probTable.corTrigger
if trigger is no:
  return pay_window unchanged
choose number_to_add from [1, 2, 3, 4, 5] using probTable.numCorWeight
build candidate positions where symbol is not COLLECT, WILD, or SCAT
select up to number_to_add distinct candidate positions
for each selected position:
  set pay_window[position] = COR
  assign a COR value using baseCORValueWeights
return pay_window
```

Invariants: Never overwrite `COLLECT`, `WILD`, or `SCAT`. Do not change window shape. If there are fewer eligible positions than requested, fill only eligible positions and record no invented replacement rule.

Trace emission: Emit entry, input/output `pay_window`, `probTable.corTrigger`, `probTable.numCorWeight`, `probTable.baseCORValueWeights.values`, `probTable.baseCORValueWeights.weights`, and exit.

### waysWinCalculation

Purpose: Evaluate standard left-to-right 4096 ways wins and direct scatter paytable factors for the final visible window.

Inputs: `pay_window` shape `[4x6]`.

Outputs: `win` scalar.

Dependencies: `PayTable` shape `[12x7]`, `STANDARD_SYMBOL_COUNT`, `PAYTABLE_SYMBOL_COUNT`.

Pseudocode:

```text
win = 0
for each standard symbol:
  ways = 1
  consecutive_reels = 0
  for reel from 0 to 5:
    count matching positions where symbol equals current standard symbol or WILD
    if count is zero:
      break
    ways = ways * count
    consecutive_reels = consecutive_reels + 1
  win = win + PayTable[current standard symbol][consecutive_reels] * ways
count SCAT symbols anywhere in pay_window
win = win + PayTable[SCAT][SCAT_count]
return win
```

Invariants: `WILD` substitutes only for standard symbols. `WILD` does not contribute to `SCAT`, `COR`, or `COLLECT`. Counts outside `0..6` are invalid.

Trace emission: Emit entry, input `pay_window`, `PayTable`, `STANDARD_SYMBOL_COUNT`, `PAYTABLE_SYMBOL_COUNT`, output `win`, and exit.

### waysWinCalculationWithMultiplier

Purpose: Evaluate Free Spins ways/scatter wins and apply visible WILD multipliers only to non-SCAT wins.

Inputs: `pay_window` shape `[4x6]`.

Outputs: `win` scalar.

Dependencies: `PayTable` shape `[12x7]`, `probTable.wildMult`, `STANDARD_SYMBOL_COUNT`, `PAYTABLE_SYMBOL_COUNT`.

Pseudocode:

```text
assign multiplier 2, 3, or 5 to each visible WILD using probTable.wildMult
standard_win = ways evaluation for standard symbols only
scatter_win = PayTable[SCAT][number of visible SCAT symbols]
total_wild_multiplier = product of all visible WILD multipliers
if no WILD is visible:
  total_wild_multiplier = 1
win = standard_win * total_wild_multiplier + scatter_win
return win
```

Invariants: Multipliers apply to all non-SCAT wins whether or not a WILD participates in a winning combination. Multipliers never apply to SCAT pays or retrigger awards. `COR` and `COLLECT` must not appear in Free Spins windows.

Trace emission: Emit entry, input `pay_window`, `probTable.wildMult`, `PayTable`, `STANDARD_SYMBOL_COUNT`, `PAYTABLE_SYMBOL_COUNT`, output `win`, and exit.

### resolveCollect

Purpose: Resolve `COLLECT` behavior for base game and Blitz Spins.

Inputs: `pay_window` shape `[4x6]`, `cor_values` shape `[4x6]`.

Outputs: `collect_win` scalar.

Dependencies: `BASE_BET_FIXED_COINS`, `probTable.CORValues` for value-domain validation.

Pseudocode:

```text
find all COLLECT positions on reel 1 and reel 6
find all visible COR positions and their assigned COR value multipliers
collect_win = 0
for each COLLECT position:
  for each COR position:
    collect_win = collect_win + COR value multiplier * total bet
return collect_win
```

Invariants: Each `COLLECT` independently collects all visible `COR` values. `COLLECT` on reels other than 1 or 6 is invalid. `COR` values must come from `CORValues`.

Trace emission: Emit entry, input `pay_window`, input `cor_values`, `BASE_BET_FIXED_COINS`, `probTable.CORValues`, output `collect_win`, and exit.

### FreeGameFeature_1

Purpose: Execute Free Spins after player selects the Free Spins option.

Inputs: `num_of_free_spins` scalar.

Outputs: `session_win` scalar.

Dependencies: `FG1_Reels`, `FG1_ReelSize`, `probTable.wildMult`, `PayTable`, retrigger awards `{2:5, 3:8, 4:12, 5:20, 6:30}`.

Pseudocode:

```text
spins_remaining = num_of_free_spins
session_win = 0
while spins_remaining > 0:
  spins_remaining = spins_remaining - 1
  pay_window = generatePayWindow(FG1_Reels, FG1_ReelSize)
  spin_win = waysWinCalculationWithMultiplier(pay_window)
  session_win = session_win + spin_win
  scat_count = count SCAT symbols in pay_window
  if scat_count is 2, 3, 4, 5, or 6:
    spins_remaining = spins_remaining + retrigger_award[scat_count]
return session_win
```

Invariants: No `COR` or `COLLECT` appears in Free Spins. Free Spins exit only when all initial and retriggered spins are exhausted.

Trace emission: Emit entry, input `num_of_free_spins`, dependencies, nested calls to `generatePayWindow` and `waysWinCalculationWithMultiplier`, output `session_win`, and exit. Runtime may repeat nested spin traces for each actual spin; the machine contract below defines the structural minimum for the feature path.

### FreeGameFeature_2

Purpose: Execute Blitz Spins after player selects the Blitz Spins option.

Inputs: `num_of_free_spins` scalar.

Outputs: `session_win` scalar.

Dependencies: `probTable.freeGamenumCollect`, `probTable.freeGameCOROccur`, `probTable.goodCORTable`, `probTable.badCORTable`, `probTable.CORValues`, `BASE_BET_FIXED_COINS`.

Pseudocode:

```text
spins_remaining = num_of_free_spins
session_win = 0
while spins_remaining > 0:
  spins_remaining = spins_remaining - 1
  create empty [4x6] blitz window
  choose collect_count from [1, 2] using freeGamenumCollect
  place COLLECT symbols on reel 1 and/or reel 6, maximum one per reel
  for each non-COLLECT position:
    choose whether COR appears using freeGameCOROccur
  if no COR appears:
    force exactly one COR into an eligible non-COLLECT position
  choose one visible COR to use goodCORTable
  assign every other visible COR using badCORTable
  spin_collect_win = resolveCollect(blitz window, cor_values)
  session_win = session_win + spin_collect_win
return session_win
```

Invariants: Each Blitz Spin has at least one `COLLECT` and at least one `COR`. Only `COR`, jackpot-bearing `COR`, and `COLLECT` appear. Good table is used exactly once per spin. Blitz Spins have no retrigger.

Trace emission: Emit entry, input `num_of_free_spins`, listed probTable dependencies, `BASE_BET_FIXED_COINS`, nested `resolveCollect`, output `session_win`, and exit. Runtime may repeat nested spin traces for each actual spin; the machine contract below defines the structural minimum for the feature path.

### runOneSpin

Purpose: Execute one paid base-game spin and any selected triggered feature.

Inputs: `selectedFeature` scalar enum/value, used only when 3 or more base-game `SCAT` symbols trigger feature selection.

Outputs: `total_win` scalar.

Dependencies: `probTable.ReelsetWeights`, base reelsets, base reel sizes, feature award tables, all called function dependencies.

Pseudocode:

```text
choose base reelset B1 or B2 using probTable.ReelsetWeights
pay_window = generatePayWindow(selected base reelset, selected ReelSize)
pay_window = additionalCOR(pay_window)
base_win = waysWinCalculation(pay_window)
collect_win = resolveCollect(pay_window, cor_values)
total_win = base_win + collect_win
scat_count = count SCAT symbols in pay_window
if scat_count >= 3:
  if selectedFeature is Free Spins:
    feature_spins = free_spins_award[scat_count]
    total_win = total_win + FreeGameFeature_1(feature_spins)
  if selectedFeature is Blitz Spins:
    feature_spins = blitz_spins_award[scat_count]
    total_win = total_win + FreeGameFeature_2(feature_spins)
return total_win
```

Invariants: Base feature selection is offered only for 3 or more `SCAT`. Free Spins awards are `3:8, 4:12, 5:20, 6:30`. Blitz Spins awards are `3:3, 4:4, 5:6, 6:8`. Buy Bonus is not in scope.

Trace emission: Emit entry, optional `selectedFeature`, `probTable.ReelsetWeights`, nested calls in execution order, output `total_win`, and exit.

## Machine-Verifiable Stack Trace Contract

### BaseGame_NoFreeGame

```text
->runOneSpin
  Global/Struct probTable.ReelsetWeights: std::vector<int, std::allocator<int> > shape:[2]
  ->generatePayWindow
    input Reelset: Symbol** shape:[6][variable]
    input ReelSize: std::vector<int, std::allocator<int> > shape:[6]
    Global/Struct NO_OF_ROWS: int shape:[1]
    Global/Struct NO_OF_REELS: int shape:[1]
    output pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
  <- generatePayWindow
  ->additionalCOR
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    Global/Struct probTable.corTrigger: std::vector<int, std::allocator<int> > shape:[2]
    Global/Struct probTable.numCorWeight: std::vector<int, std::allocator<int> > shape:[5]
    Global/Struct probTable.baseCORValueWeights.values: std::vector<double, std::allocator<double> > shape:[22]
    Global/Struct probTable.baseCORValueWeights.weights: std::vector<int, std::allocator<int> > shape:[22]
    output pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
  <- additionalCOR
  ->waysWinCalculation
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    Global/Struct PayTable: int [12][7] shape:[12x7]
    Global/Struct STANDARD_SYMBOL_COUNT: int shape:[1]
    Global/Struct PAYTABLE_SYMBOL_COUNT: int shape:[1]
    output win: int shape:[1]
  <- waysWinCalculation
  ->resolveCollect
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    input cor_values: std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > > shape:[4x6]
    Global/Struct BASE_BET_FIXED_COINS: int shape:[1]
    Global/Struct probTable.CORValues: std::vector<double, std::allocator<double> > shape:[22]
    output collect_win: int shape:[1]
  <- resolveCollect
  output total_win: int shape:[1]
<- runOneSpin
```

### BaseGame_FreeGameFeature1

```text
->runOneSpin
  input selectedFeature: FeatureChoice shape:[1]
  Global/Struct probTable.ReelsetWeights: std::vector<int, std::allocator<int> > shape:[2]
  ->generatePayWindow
    input Reelset: Symbol** shape:[6][variable]
    input ReelSize: std::vector<int, std::allocator<int> > shape:[6]
    Global/Struct NO_OF_ROWS: int shape:[1]
    Global/Struct NO_OF_REELS: int shape:[1]
    output pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
  <- generatePayWindow
  ->additionalCOR
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    Global/Struct probTable.corTrigger: std::vector<int, std::allocator<int> > shape:[2]
    Global/Struct probTable.numCorWeight: std::vector<int, std::allocator<int> > shape:[5]
    Global/Struct probTable.baseCORValueWeights.values: std::vector<double, std::allocator<double> > shape:[22]
    Global/Struct probTable.baseCORValueWeights.weights: std::vector<int, std::allocator<int> > shape:[22]
    output pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
  <- additionalCOR
  ->waysWinCalculation
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    Global/Struct PayTable: int [12][7] shape:[12x7]
    Global/Struct STANDARD_SYMBOL_COUNT: int shape:[1]
    Global/Struct PAYTABLE_SYMBOL_COUNT: int shape:[1]
    output win: int shape:[1]
  <- waysWinCalculation
  ->resolveCollect
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    input cor_values: std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > > shape:[4x6]
    Global/Struct BASE_BET_FIXED_COINS: int shape:[1]
    Global/Struct probTable.CORValues: std::vector<double, std::allocator<double> > shape:[22]
    output collect_win: int shape:[1]
  <- resolveCollect
  ->FreeGameFeature_1
    input num_of_free_spins: int shape:[1]
    Global/Struct FG1_Reels: Symbol** shape:[6][variable]
    Global/Struct FG1_ReelSize: std::vector<int, std::allocator<int> > shape:[6]
    Global/Struct probTable.wildMult: std::vector<int, std::allocator<int> > shape:[3]
    ->generatePayWindow
      input Reelset: Symbol** shape:[6][variable]
      input ReelSize: std::vector<int, std::allocator<int> > shape:[6]
      Global/Struct NO_OF_ROWS: int shape:[1]
      Global/Struct NO_OF_REELS: int shape:[1]
      output pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    <- generatePayWindow
    ->waysWinCalculationWithMultiplier
      input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
      Global/Struct probTable.wildMult: std::vector<int, std::allocator<int> > shape:[3]
      Global/Struct PayTable: int [12][7] shape:[12x7]
      Global/Struct STANDARD_SYMBOL_COUNT: int shape:[1]
      Global/Struct PAYTABLE_SYMBOL_COUNT: int shape:[1]
      output win: int shape:[1]
    <- waysWinCalculationWithMultiplier
    output session_win: int shape:[1]
  <- FreeGameFeature_1
  output total_win: int shape:[1]
<- runOneSpin
```

### BaseGame_FreeGameFeature2

```text
->runOneSpin
  input selectedFeature: FeatureChoice shape:[1]
  Global/Struct probTable.ReelsetWeights: std::vector<int, std::allocator<int> > shape:[2]
  ->generatePayWindow
    input Reelset: Symbol** shape:[6][variable]
    input ReelSize: std::vector<int, std::allocator<int> > shape:[6]
    Global/Struct NO_OF_ROWS: int shape:[1]
    Global/Struct NO_OF_REELS: int shape:[1]
    output pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
  <- generatePayWindow
  ->additionalCOR
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    Global/Struct probTable.corTrigger: std::vector<int, std::allocator<int> > shape:[2]
    Global/Struct probTable.numCorWeight: std::vector<int, std::allocator<int> > shape:[5]
    Global/Struct probTable.baseCORValueWeights.values: std::vector<double, std::allocator<double> > shape:[22]
    Global/Struct probTable.baseCORValueWeights.weights: std::vector<int, std::allocator<int> > shape:[22]
    output pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
  <- additionalCOR
  ->waysWinCalculation
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    Global/Struct PayTable: int [12][7] shape:[12x7]
    Global/Struct STANDARD_SYMBOL_COUNT: int shape:[1]
    Global/Struct PAYTABLE_SYMBOL_COUNT: int shape:[1]
    output win: int shape:[1]
  <- waysWinCalculation
  ->resolveCollect
    input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
    input cor_values: std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > > shape:[4x6]
    Global/Struct BASE_BET_FIXED_COINS: int shape:[1]
    Global/Struct probTable.CORValues: std::vector<double, std::allocator<double> > shape:[22]
    output collect_win: int shape:[1]
  <- resolveCollect
  ->FreeGameFeature_2
    input num_of_free_spins: int shape:[1]
    Global/Struct probTable.freeGamenumCollect: std::vector<int, std::allocator<int> > shape:[2]
    Global/Struct probTable.freeGameCOROccur: std::vector<int, std::allocator<int> > shape:[2]
    Global/Struct probTable.goodCORTable: std::vector<int, std::allocator<int> > shape:[22]
    Global/Struct probTable.badCORTable: std::vector<int, std::allocator<int> > shape:[22]
    Global/Struct probTable.CORValues: std::vector<double, std::allocator<double> > shape:[22]
    Global/Struct BASE_BET_FIXED_COINS: int shape:[1]
    ->resolveCollect
      input pay_window: std::vector<std::vector<Symbol, std::allocator<Symbol> >, std::allocator<std::vector<Symbol, std::allocator<Symbol> > > > shape:[4x6]
      input cor_values: std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > > shape:[4x6]
      Global/Struct BASE_BET_FIXED_COINS: int shape:[1]
      Global/Struct probTable.CORValues: std::vector<double, std::allocator<double> > shape:[22]
      output collect_win: int shape:[1]
    <- resolveCollect
    output session_win: int shape:[1]
  <- FreeGameFeature_2
  output total_win: int shape:[1]
<- runOneSpin
```

## Human-Readable Execution Explanation

A base spin selects B1 or B2 by weight, generates the visible 4-row by 6-reel window, optionally injects extra COR symbols without overwriting COLLECT, WILD, or SCAT, then evaluates standard ways and SCAT pays. The Collect step then lets every visible COLLECT on reel 1 or reel 6 collect every visible COR value.

If the final base window contains 3 or more SCAT symbols, feature selection is offered. Free Spins use F1, assign WILD multipliers, apply those multipliers only to non-SCAT wins, and can retrigger. Blitz Spins generate only COR/COLLECT screens, guarantee at least one of each, use the Good COR table exactly once per spin, and resolve all COR values through Collect. Buy Bonus remains out of simulator scope until human review explicitly includes it.

## Open Warnings And Ambiguities

- `[WARNING]` Maximum bet is not specified.
- `[WARNING]` Coin value and denomination are not specified as authoritative simulator configuration.
- `[WARNING]` Oracle workbook test coverage is incomplete for simulator-core verification.
- `[WARNING]` Mathematical acceptance thresholds are incomplete.
- `[AMBIGUITY]` Buy Bonus data exists, but DOCX rules do not describe Buy Bonus availability or player-facing behavior.
- `[AMBIGUITY]` Parsheet references to `CB spins` appear to refer to Blitz/VB Spins, but source vocabulary is inconsistent.
