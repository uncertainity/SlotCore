# Vegas Blitz Game Flow

## Planner Scope

This artifact describes the implementation-independent game flow for Vegas Blitz. It uses the game rules DOCX and the Vegas Blitz parsheet. The parsheet files in `specs/game_rules/` and `specs/math_models/` are byte-identical and are treated as one source.

Planner rerun date: 2026-06-14.

This Planner run does not advance the pipeline beyond Planner. Existing downstream validation blockers remain unresolved, and no Simulator Design, Implementation Generation, or validation stage was run.

## Game Overview

- Game name: Vegas Blitz
- Window: 6 reels by 4 rows
- Pay mechanism: 4096 ways
- Minimum game cost: 20 fixed coins
- Declared long-term expected payback in rules: 94.11%
- Base game uses weighted reelset selection between B1 and B2.
- Main features: Collect, Free Spins, Blitz Spins.
- Feature entry: 3 or more SCATTER symbols in the base game opens a feature selection screen.

## Symbol Normalization

The source symbol names are normalized for downstream artifacts:

| Source | Planner label |
| --- | --- |
| PIC1 | HV1 |
| PIC2 | HV2 |
| PIC3 | HV3 |
| PIC4 | HV4 |
| PIC5 | HV5 |
| ACE | LV1 |
| KING | LV2 |
| QUEEN | LV3 |
| JACK | LV4 |
| TEN | LV5 |
| NINE | LV6 |
| SCAT | SCATTER |
| COIN | COR |
| WILD | WILD |
| COLLECT | COLLECT |

Standard symbols are HV1-HV5 and LV1-LV6.

## Special Symbols

WILD substitutes for HV1-HV5 and LV1-LV6. It does not substitute for SCATTER, COR, or COLLECT. In the base game, WILD can appear only on reels 2-6. In Free Spins, WILD can appear only on reels 2-4.

SCATTER pays anywhere. In the base game, 3 or more SCATTER symbols trigger the feature selection screen. In Free Spins, 2 or more SCATTER symbols can retrigger additional Free Spins.

COR symbols award their displayed value only when at least one COLLECT symbol is also present. COR values are multiplied by total bet. Jackpot-bearing COR values are collected in the same way as other COR values.

COLLECT can land only on reel 1 and/or reel 6. Each COLLECT independently collects all visible COR values and jackpot-bearing COR values. COLLECT is active only in the base game and Blitz Spins.

Jackpot values are Mini 10x total bet, Minor 25x total bet, Major 250x total bet, and Grand 2000x total bet. Jackpots are won only through the Collect feature.

## Bet Configuration

- Minimum bet: 20 fixed coins
- Parsheet base bet: 20
- Maximum bet: missing
- Coin value / denomination: missing
- Pay mechanism: ways, not paylines
- Feature-buy availability: ambiguous. Buy Bonus tables exist in the parsheet, but the DOCX rules do not describe Buy Bonus availability or UI rules.

## Base Game Flow

1. Player places a valid wager.
2. Base game starts.
3. Select base reelset B1 or B2 using parsheet weights:
   - B1 weight 58
   - B2 weight 42
4. Generate a 6x4 visible window from the selected base reelset.
5. Check whether additional COR symbols are injected:
   - Trigger yes weight 1
   - Trigger no weight 9
6. If additional COR injection triggers, add COR symbols anywhere except positions already occupied by COLLECT, WILD, or SCATTER. Number of added CORs uses these weights:
   - 1 COR: 240
   - 2 CORs: 200
   - 3 CORs: 150
   - 4 CORs: 100
   - 5 CORs: 80
7. The resulting window is the final base-game window.
8. Evaluate left-to-right ways wins.
9. Evaluate anywhere SCATTER wins.
10. Check for COLLECT symbols on reel 1 and reel 6.
11. If COLLECT is present, each COLLECT independently collects every visible COR and jackpot-bearing COR.
12. Display the three win buckets identified by the implementation sheet: line wins, scatter wins, and COR wins.
13. If 3 or more SCATTER symbols appear, show the feature selection screen.
14. Player selects either Free Spins or Blitz Spins.
15. If no feature is triggered, credit total payout and return to idle.

## Pay Evaluation

Standard symbols pay from the leftmost reel to the right in consecutive order. Each symbol can be used once per winning combination. WILD substitutes for standard symbols only.

SCATTER pays anywhere and is not multiplied by Free Spins WILD multipliers.

The paytable values are present in the parsheet and are included in `game_flow.json`. The source does not fully specify payout units for all win classes, so payout-unit semantics remain unresolved.

## Collect Feature

The Collect feature is active in the base game and Blitz Spins only.

Entry condition: at least one COLLECT symbol appears on reel 1 and/or reel 6 while COR symbols or jackpot-bearing COR symbols are visible.

Resolution:

1. Identify all COLLECT symbols on reels 1 and 6.
2. Identify all visible COR and jackpot-bearing COR symbols.
3. Each COLLECT independently collects all visible COR and jackpot values.
4. Collected COR values are multiplied by total bet.
5. Add collected values to the COR win bucket.

COR and COLLECT do not appear during Free Spins.

## Feature Selection

Base-game SCATTER counts award one of two player-selected feature options:

| SCATTER count | Option 1: Free Spins | Option 2: Blitz Spins |
| --- | ---: | ---: |
| 3 | 8 | 3 |
| 4 | 12 | 4 |
| 5 | 20 | 6 |
| 6 | 30 | 8 |

## Free Spins Flow

Entry condition: player selects Free Spins after triggering the feature selection screen.

Initial Free Spins award:

- 3 SCATTER: 8 Free Spins
- 4 SCATTER: 12 Free Spins
- 5 SCATTER: 20 Free Spins
- 6 SCATTER: 30 Free Spins

Each Free Spin:

1. Generate a 6x4 window from reelset F1.
2. Assign multipliers to any WILD symbols using weights:
   - 2x: 480
   - 3x: 545
   - 5x: 360
3. Evaluate left-to-right ways wins.
4. Evaluate SCATTER wins.
5. Multiply all visible WILD multipliers together.
6. Apply the total WILD multiplier to non-SCATTER wins whether or not the WILD participates in the winning combination.
7. Do not apply WILD multipliers to SCATTER wins.
8. Check for retriggers.

Free Spins retrigger awards:

| SCATTER count | Additional Free Spins |
| --- | ---: |
| 2 | 5 |
| 3 | 8 |
| 4 | 12 |
| 5 | 20 |
| 6 | 30 |

Free Spins exit when all awarded and retriggered Free Spins are exhausted.

## Blitz Spins Flow

Entry condition: player selects Blitz Spins after triggering the feature selection screen.

Initial Blitz Spins award:

- 3 SCATTER: 3 Blitz Spins
- 4 SCATTER: 4 Blitz Spins
- 5 SCATTER: 6 Blitz Spins
- 6 SCATTER: 8 Blitz Spins

Only COR, jackpot-bearing COR, and COLLECT symbols can land during Blitz Spins. Each Blitz Spin guarantees at least one COR and at least one COLLECT.

Each Blitz Spin:

1. Choose whether 1 or 2 COLLECT symbols appear:
   - 1 COLLECT weight 75
   - 2 COLLECT weight 25
2. Place COLLECT symbols on reel 1 and/or reel 6, with a maximum of one COLLECT per reel.
3. For every non-COLLECT position, check for COR occurrence:
   - Yes weight 26
   - No weight 74
4. If no COR appears, force one COR onto the screen.
5. Assign COR values. The Good table is used exactly once per spin; the Bad table is used for all other CORs.
6. Each COLLECT collects all COR and jackpot-bearing COR symbols on the screen.
7. Multiply collected COR values by total bet.
8. Reset the screen for the next Blitz Spin.

Blitz Spins have no retriggers in the source rules. The feature exits when all awarded Blitz Spins are exhausted.

## Buy Bonus Tables

The parsheet contains Buy Bonus data:

| SCATTER count | Cost in game bets | Option 1 | Option 2 |
| --- | ---: | --- | --- |
| 4 | 80x | 12 FS | 4 spins |
| 5 | 95x | 20 FS | 6 spins |
| 6 | 110x | 30 FS | 8 spins |

[AMBIGUOUS] Buy Bonus availability and UI rules are not described in the DOCX game rules. The tables are retained in `game_flow.json` for traceability, but Planner does not assert that Buy Bonus is an available player-facing mechanic without human confirmation.

## State Machine

```text
idle
-> bet_placed
-> base_spin_started
-> base_window_generated
-> base_window_augmented
-> base_wins_evaluated
-> collect_evaluated
-> payout_complete
-> idle
```

Feature branch:

```text
collect_evaluated
-> feature_selection
-> free_spins_active
-> feature_complete
-> payout_complete
-> idle
```

Alternative feature branch:

```text
collect_evaluated
-> feature_selection
-> blitz_spins_active
-> feature_complete
-> payout_complete
-> idle
```

## Mathematical Dependencies

The following source data affects RTP, hit rate, volatility, and feature frequency:

- Base reelset selection weights B1/B2.
- Reel strips for B1, B2, and F1.
- Visible-window extraction convention from reel stops.
- Paytable values and payout units.
- Additional COR trigger and count weights.
- Base COR value weights.
- SCATTER distribution in base and Free Spins reelsets.
- Free Spins WILD multiplier weights.
- Blitz Spins COLLECT count weights.
- Blitz Spins COR occurrence weights.
- Blitz Spins Good and Bad COR value tables.
- Jackpot values and jackpot-bearing COR weights.
- Buy Bonus tables if human review confirms Buy Bonus is in scope.

## Assumptions

ASSUMPTION: Workbook references to CB spins refer to Blitz/VB Spins.

Reason: surrounding workbook labels and the DOCX rules identify the feature as Blitz Spins or VB spins.

ASSUMPTION: SCAT in the workbook is the same symbol as SCATTER in the DOCX rules.

Reason: workbook SCAT rows match the DOCX Scatter behavior and feature trigger rules.

## Missing Information

[MISSING] Maximum bet is not specified.

[MISSING] Coin value / denomination is not specified.

[MISSING] Exact reel-stop window extraction convention is not explicitly stated, including stop-index semantics, wrap direction, and visible-row ordering.

[MISSING] Paytable payout units are not fully specified for all win classes.

[MISSING] Buy Bonus availability and UI behavior are not specified in the DOCX game rules.

## Ambiguities

[AMBIGUOUS] Buy Bonus tables and costs are present in the parsheet, but Buy Bonus availability and UI rules are absent from the DOCX rules.

[AMBIGUOUS] Paytable unit semantics require clarification before deterministic implementation.

[AMBIGUOUS] Visible-window extraction from reel stops requires clarification before deterministic tests and trace validation.
