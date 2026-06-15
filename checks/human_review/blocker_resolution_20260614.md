# Human Review - Blocker Resolution - 2026-06-14

## Resolved Blockers

[RESOLVED] Paytable payout units / SCATTER payout behavior.

Human decision:

- If SCATTER wins are not mentioned, SCATTER occurrence does not award a direct SCATTER win.
- Existing COR behavior remains unchanged: collected COR values are multiplied by total bet.
- Standard-symbol paytable factors remain the authoritative paytable factors for ways evaluation.

[RESOLVED] Reel-stop visible-window extraction convention.

Human decision:

- Use the same visible-window extraction convention as `reference/cpp/reflib.cpp`.
- `start_idx` is the top visible symbol on each reel.
- Visible rows are filled downward using `idx = (start_idx + row) % reel_length`.
- The strip wraps forward through increasing strip indexes.
- The visible window is stored row-major as `pay_window[row][reel]`.

## Remaining Non-Blocking Warnings / Ambiguities

- Maximum bet is not specified.
- Coin value and denomination rules are not specified as authoritative simulator configuration.
- Buy Bonus availability and player-facing behavior remain ambiguous.
- Mathematical acceptance thresholds remain incomplete for jackpot frequencies, feature-frequency tolerances, and volatility tolerances.
