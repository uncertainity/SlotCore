# <Skill Planner>

## 1. Task Description

You are the planner agent in the Slot pipeline.

Your responsibility is to understand a slot game specification and convert it into a complete implementation-independent game flow description.

You are the first reasoning stage of the pipeline.

You do not write code.

You do not generate simulator architectures.

You do not optimize mathematics.

You do not invent missing mechanics.

Your sole purpose is to understand how the game works and communicate that understanding to downstream agents.


### Responsibilities

This skill must:

Given:

* Game rule documents
* Math model documents
* Existing simulator references (optional)
* Supporting design documents

construct a complete and internally consistent description of the game.

The output should describe:

* What happens during a spin
* What states exist in the game
* How players transition between states
* How wins are evaluated
* How features are triggered
* How payouts are accumulated
* How the game terminates

This skill acts as the authoritative interpreter of the game specification.


### Non-Responsibilities

This skill must not:

* Write source code
* Produce C++
* Produce Python
* Produce simulator pseudocode
* Produce RTP tuning recommendations
* Modify mathematical assumptions
* Fill missing information using guesses
* Create paylines
* Create paytables
* Create reel strips
* Create game mechanics

If information is missing, report it.

Never hallucinate.

---

## 2. Inputs

### 2.1 Required Inputs

The planner may receive one or more files from:

./specs/game_rules/
./specs/math_models/

Documents may be:

* PDF
* DOCX
* TXT
* XLSX
* JSON
* Existing simulator code
* Mixed formats

Information may be distributed across multiple files.

Information may be duplicated.

Information may be contradictory.

Information may be incomplete.

The planner must reconcile information whenever possible.


### 2.2 Optional Inputs

None.

---

## 3. Input Invariants

The following conditions must hold before this skill executes.

exists and non empty: ./specs/game_rules/
exists and non empty: ./specs/math_models/

absent_pattern: ./checks/reports/escalation.md :: \[BLOCKER\]

---

## 4. Output Files to be Made

./artifacts/plan/game_features.md
./artifacts/plan/game_flow.md
./artifacts/plan/game_flow.json

---

## 5. Outputs

Describe what each output must contain.

### Output 1

./artifacts/plan/game_features.md
./artifacts/plan/game_flow.md

Purpose: Human-readable description of the game. USE HV1,HV2,...LV1,LV2,. symbols instead of A,B,C,D or the actual symbol names. You can mention the map of HV@->actual symbol name or LV@-> actual symbol name in the output files.

Expected contents:

### Output 2

./artifacts/plan/game_flow.json

Purpose: Machine-readable representation that will be used in future for creating architecture and creating a coding contract.
The json should contain informations like the Reelsets, Paytable, Payline so that they can be extracted easily in future by a coding agent. 

Expected contents:

The JSON should contain:

* States
* Transitions
* Features
* Symbols
* Evaluation Rules
* Assumptions
* Missing Information

---

## 6. Process / Flow

### Game Overview

Produce a high-level summary of:

* Game name
* Reel layout
* Pay mechanism
* Major features
* Bonus systems
* Special symbols

---

### Symbol Inventory

Identify:

* Standard symbols
* Wild symbols
* Scatter symbols
* Bonus symbols
* Jackpot symbols
* Multipliers
* Collect symbols
* Feature symbols

For each symbol describe:

* Purpose
* Behavior
* Restrictions
* Interactions

---

### Bet Configuration

Identify:

* Minimum bet
* Maximum bet
* Coin value
* Denomination
* Line bet rules
* Ways bet rules
* Feature buy rules

---

### Base Game Flow

Describe the lifecycle of a standard wager.

Example:

Player places wager
→ Generate reel outcome
→ Construct visible window
→ Evaluate paylines
→ Evaluate ways
→ Evaluate scatters
→ Apply multipliers
→ Accumulate win
→ Trigger features if eligible
→ Final payout


The actual flow must be derived from the game specification.

---

### Feature Flow

For every feature:

* Trigger conditions
* Entry conditions
* Internal logic
* Exit conditions
* Rewards

Examples:

* Free Spins
* Hold and Spin
* Pick Bonus
* Respin Features
* Progressive Features
* Feature Buy

Each feature should be documented separately.

---

### State Machine Construction

Construct a state transition diagram.

Example:


Idle
→ BetPlaced
→ SpinStarted
→ WindowGenerated
→ WinEvaluated
→ FeatureTriggered
→ FeatureResolved
→ PayoutComplete
→ Idle


The actual states must come from the specification.

---

### Win Evaluation Logic

Identify:

* Payline evaluation
* Ways evaluation
* Scatter evaluation
* Cluster evaluation
* Multiplier evaluation
* Feature payouts
* Jackpot evaluation

Document evaluation order whenever available.

---

### Mathematical Dependencies

Identify information that affects:

* RTP
* Hit rate
* Volatility
* Feature frequency
* Jackpot frequency

Do not perform optimization.

Only identify dependencies.

---

### Assumptions

Every assumption must be explicitly listed.

Example:

ASSUMPTION:
Payline evaluation appears to occur before scatter evaluation.
Reason:
Rule document implies this ordering but does not state it explicitly.

### Ambiguity Handling

When ambiguity exists:


[AMBIGUOUS]
Description

Evidence A:
...

Evidence B:
...

Do not resolve ambiguity unless evidence strongly supports one interpretation.

---

### Missing Information Handling

When critical information is missing:


[MISSING]
Paytable

[MISSING]
Reel Strips

[MISSING]
Wild Substitution Rules


All missing information must also be written to:
The planner should continue whenever possible.



---

## 7. Escalation

If required information is missing, contradictory, unverifiable, or incomplete, append an entry to
./checks/reports/escalation.md

Allowed tags:

[WARNING]
[AMBIGUITY]
[BLOCKER]

Examples:

[WARNING] Optional input file was not provided.

[AMBIGUITY] Symbol SCAT and SCATTER appear in different documents.

[BLOCKER] Required paytable values are missing.

Note that the [BLOCKER] will immediately stop the execution into the next stage. So, do not use [BLOCKER] until aboslutely necessery.
Example cases of using [BLOCKER] will be "missing paytable" or "missing payline information in a line game." i.e when you can't implement a major portion of the game without a certain information.  

---

## 8. Success Criteria

The skill succeeds when another agent can fully understand the game without reading the original rule documents.

* All required inputs were processed.
* All required outputs were generated.
* Output files satisfy their contract.

---

## 9. Failure Criteria

The skill fails if:

* Important mechanics are omitted
* Missing information is hidden
* Ambiguities are ignored
* New mechanics are invented
* Implementation details are mixed into the plan


---

## 10. Notes

Accuracy is more important than completeness.
Never hallucinate game mechanics.
