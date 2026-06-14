# <Skill Name>

## 1. Task Description

Describe the purpose of this skill.

### Responsibilities

This skill must:

* ...
* ...
* ...

### Non-Responsibilities

This skill must not:

* ...
* ...
* ...

---

## 2. Inputs

### 2.1 Required Inputs

./path/to/input_file.md
./path/to/input_directory/

### 2.2 Optional Inputs


./path/to/optional_file.md
./path/to/optional_directory/

---

## 3. Input Invariants

The following conditions must hold before this skill executes.


exists: ./path/to/input_file.md
exists: ./path/to/input_directory/

absent_pattern: ./path/to/report.md :: \[BLOCKER\]
absent_pattern: ./path/to/report.md :: \[ERROR\]

---

## 4. Output Files to be Made

./path/to/output_file.md
./path/to/output_file.json
./path/to/output_file.txt

---

## 5. Outputs

Describe what each output must contain.

### Output 1

Purpose:

Expected contents:

### Output 2

Purpose:

Expected contents:

---

## 6. Process / Flow

1. Read required inputs.
2. Validate input invariants.
3. Execute skill-specific logic.
4. Generate outputs.
5. Validate outputs.
6. Escalate unresolved issues if necessary.

---

## 7. Escalation

If required information is missing, contradictory, unverifiable, or incomplete, append an entry to the escalation report.

Allowed tags:

[WARNING]
[AMBIGUITY]
[BLOCKER]

Examples:

[WARNING] Optional input file was not provided.

[AMBIGUITY] Symbol SCAT and SCATTER appear in different documents.

[BLOCKER] Required paytable values are missing.
```

---

## 8. Success Criteria

The skill succeeds when:

* All required inputs were processed.
* All required outputs were generated.
* No unresolved `[BLOCKER]` remains.
* Output files satisfy their contract.

---

## 9. Failure Criteria

The skill fails if:

* Required inputs are missing.
* Required outputs are not generated.
* Placeholder implementations are produced.
* Contradictory specifications are ignored.
* An unresolved `[BLOCKER]` remains.
* The skill invents missing information instead of escalating.

---

## 10. Notes

Additional implementation guidance, conventions, or project-specific instructions may be placed here. 
