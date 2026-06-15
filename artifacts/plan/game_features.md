# Vegas Blitz Planner Game Features

This file is a compact feature inventory for the Planner stage. The complete state flow and machine-readable data are in `game_flow.md` and `game_flow.json`.

## Symbols

HV1-HV5 are the five picture symbols. LV1-LV6 are ACE, KING, QUEEN, JACK, TEN, and NINE. WILD substitutes for standard symbols only. SCATTER pays anywhere. COR is the cash/coin symbol. COLLECT independently collects all visible COR and jackpot COR values.

## Features

- Collect: active in base game and Blitz Spins; triggered by COLLECT on reel 1 and/or reel 6; each COLLECT independently awards all visible COR values multiplied by total bet.
- Free Spins: selected after 3 or more base-game SCATTER symbols; awards 8/12/20/30 spins for 3/4/5/6 SCATTER; retriggers on 2-6 SCATTER; WILD multipliers 2x/3x/5x apply to non-SCATTER wins.
- Blitz Spins: selected after 3 or more base-game SCATTER symbols; awards 3/4/6/8 spins for 3/4/5/6 SCATTER; only COR, jackpot COR, and COLLECT symbols appear; no retriggers.
- Buy Bonus: parsheet-defined options for 4/5/6 SCATTER at 80x/95x/110x game bet; DOCX rules do not describe availability or UI.

## Missing Or Ambiguous Items

Maximum bet, coin value/denomination, exact reel-window extraction convention, and Buy Bonus availability remain unresolved. See `checks/reports/escalation.md`.
