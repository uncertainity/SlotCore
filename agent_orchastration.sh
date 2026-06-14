#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="${ROOT_DIR}/logs"


SPECS_DIR="./specs"
REFERENCE_DIR="./reference"
ARTIFACTS_DIR="./artifacts"
CHECKS_DIR="./checks"
MATH_DIR="./math"
LOGS_DIR="./logs"

GAME_RULES_DIR="${SPECS_DIR}/game_rules"
MATH_MODELS_DIR="${SPECS_DIR}/math_models"

REFERENCE_CPP_DIR="${REFERENCE_DIR}/cpp"
REFERENCE_NUMBA_DIR="${REFERENCE_DIR}/numba"
REFERENCE_EXAMPLES_DIR="${REFERENCE_DIR}/examples"

PLAN_DIR="${ARTIFACTS_DIR}/plan"
CONTRACT_DIR="${ARTIFACTS_DIR}/contract"
IMPLEMENTATION_DIR="${ARTIFACTS_DIR}/implementation"
OPTIMIZED_DIR="${ARTIFACTS_DIR}/optimized"
DEMO_DIR="${ARTIFACTS_DIR}/demo"

ORACLE_DIR="${CHECKS_DIR}/oracle"
GENERATED_TESTS_DIR="${CHECKS_DIR}/generated_tests"
REPORTS_DIR="${CHECKS_DIR}/reports"
TRACES_DIR="${CHECKS_DIR}/traces"

HUMAN_REVIEW_DIR="${CHECKS_DIR}/human_review"


mkdir -p "$LOG_DIR"
mkdir -p "$HUMAN_REVIEW_DIR"


TS=$(date +"%Y%m%d_%H%M%S")
PIPELINE_LOG_FILE="${LOG_DIR}/pipeline_${TS}.log"
SUMMARY_LOG_FILE="${LOG_DIR}/summary_${TS}.log"
HUMAN_REVIEW_FILE="${HUMAN_REVIEW_DIR}/interactive_pass_hist.txt"

complete_log(){
    echo "[$(date '+%F %T')] $*" | tee -a "$PIPELINE_LOG_FILE"
}

log(){
    echo "[$(date '+%F %T')] $*" | tee -a "$SUMMARY_LOG_FILE"
}


append_interactive_pass(){
    {
        echo ""
        echo "=================================================="
        echo "Interactive Pass - $(date '+%F %T')"
        echo "=================================================="
        echo ""
        cat "./agentprompts/interactive_pass.txt"
        echo ""
    } >> "$HUMAN_REVIEW_FILE"
}

run_codex() {
    local STAGE="$1"
    local PROMPT="$2"

    complete_log "Running stage: ${STAGE}"
    log "Running stage: ${STAGE}"
    codex exec "$(cat "$PROMPT")" 2>&1 | tee -a "$PIPELINE_LOG_FILE"
    TOKENS=$(
    awk '/tokens used/ {getline; t=$0} END {print t}' \
    "$PIPELINE_LOG_FILE"
    )
    log "${STAGE}: ${TOKENS}"
    complete_log "Finished stage: ${STAGE}"
    log "Finished stage: ${STAGE}"
}

check_file() {
    local FILE="$1"

    if [[ ! -s "$FILE" ]]; then
        log "ERROR: Missing required artifact: $FILE"
        exit 1
    fi
}


###############################################################################
# Input Validation
###############################################################################

log "Running validate_specs.sh"

bash ./validate_specs.sh \
    2>&1 | tee -a "$PIPELINE_LOG_FILE"

log "Input validation passed"


planner(){
    run_codex \
    "planner" \
    "./agentprompts/run_game_specification.txt"
    
    echo "Planner has been completed."
    log "Planner has been completed."

    check_file "${ARTIFACTS_DIR}/plan/game_flow.md"
    check_file "${ARTIFACTS_DIR}/plan/game_flow.json"
}

planner_verifier(){
    run_codex \
    "tester" \
    "./agentprompts/run_specification_validation.txt"

    check_file "${REPORTS_DIR}/planner_verifier_report.md"
    check_file "${REPORTS_DIR}/escalation.md"
    check_file "${GENERATED_TESTS_DIR}/proposed_test_cases.txt"
    if grep -q "\[BLOCKER\]" "${REPORTS_DIR}/escalation.md"; then
        echo "[BLOCKER] found in escalation.md"
        return 1
    fi
        
    echo "Planner Verifier has been completed."
    log "Planner Verifier has been completed."
    return 0
}


planner_verification_loop(){
    MAX_RETRIES=3
    attempt=1

    while (( attempt <= MAX_RETRIES )); do 
        echo "[INFO] planner/verifier attempt ${attempt}/${MAX_RETRIES}"
        planner
        if planner_verifier; then
            echo "[SUCCESS] planner output passed tester"
            break
        fi
        echo "[RETRY] planner verifier found blocker; rerunning planner"
        ((attempt++))
    done 

    if (( attempt > MAX_RETRIES )); then
        echo "[ERROR] planner could not pass verification after ${MAX_RETRIES} attempts"
        exit 1
    fi
    log "No of attempts in Planner-Tester Loop = ${attempt}"
    log "Planner + Verifier pipeline completed successfully"
}


simulator(){
    run_codex \
    "simulator" \
    "./agentprompts/run_simulator_design.txt"
    if grep -q "\[BLOCKER\]" "${REPORTS_DIR}/escalation.md"; then
        echo "[BLOCKER] found in escalation.md. Human Verification is further required."
    fi
    echo "Simulator has been completed."
    log "Simulator has been completed."

}

simulator_human_gate(){
    simulator
    read -rp "simulator has finished. Waiting for human verification. Update/Check ./agentprompts/run_contract_refinement.txt and
press Enter to proceed to the next stage.
"
    run_codex \
    "simulator_human_gate" \
    "./agentprompts/run_contract_refinement.txt"
    echo "Simulator-Human Gate has been completed."
    log "Simulator-Human Gate has been completed."
}

coder(){
    run_codex \
    "coder" \
    "./agentprompts/run_implementation_generation.txt"
    echo "Coder has been completed."
    log "Coder has been completed."
}

tester(){
    run_codex \
    "tester" \
    "./agentprompts/run_test_generation.txt"
    if grep -q "\[BLOCKER\]" "${REPORTS_DIR}/escalation.md"; then
        echo "[BLOCKER] found in escalation.md."
    fi
    echo "Tester has been completed."
    log "Tester has been completed."

}


interactive_pass() {
    while grep -q "\[BLOCKER\]" "${REPORTS_DIR}/escalation.md"; do

        run_codex \
            "interactive pass" \
            "./agentprompts/interactive_pass.txt"
        
        append_interactive_pass
        : > "./agentprompts/interactive_pass.txt"

        ## -n is for getting the [BLOCKERS] quickly
        grep -n "\[BLOCKER\]" "${REPORTS_DIR}/escalation.md"

        if grep -q "\[BLOCKER\]" "${REPORTS_DIR}/escalation.md"; then
            read -rp "
[BLOCKER] still exists.
Edit escalation.md or interactive_pass.txt.
Press Enter when ready...
"
        fi
    done
    echo "All blockers are removed."
    log "All blockers are removed."
}


consensus_loop() {
    local max_attempts=5
    local attempt=1

    while (( attempt <= max_attempts )); do
        echo "Consensus attempt ${attempt}/${max_attempts}"

        tester

        if ! grep -q "\[BLOCKER\]" "${REPORTS_DIR}/escalation.md"; then
            echo "Consensus loop passed."
            log "Consensus loop passed."
            log "No of attempts in Consensus Loop = ${attempt}"
            return 0
        fi

        echo "[BLOCKER] found after tester."
        echo "Running simulator to resolve contract/schema issue."

        run_codex \
            "consensus repair" \
            "./agentprompts/run_consensus_repair.txt"

        run_codex \
            "coder consensus repair" \
            "./agentprompts/run_implementation_generation.txt"

        ((attempt++))
    done
    echo "Consensus loop failed after ${max_attempts} attempts."
    echo "Starting interactive pass."
    interactive_pass
}


generate_test_trace(){
    run_codex \
            "generate test traces" \
            "./agentprompts/run_test_traces.txt"
    check_file "${TRACES_DIR}/test_traces.txt"
}


usage() {
    cat <<EOF
Usage: $0 <command>

Commands:
  full                  Run full SlotCore pipeline
  planner               Run planner only
  planner-verifier      Run planner-verifier only
  planner-verification  Run planner <-> verification loop
  simulator             Run simulator only
  simulator-gate        Run simulator + human gate
  coder                 Run coder only
  interactive-pass      Run human interactive blocker pass
  tester                Run tester only
  consensus             Run simulator -> coder -> tester consensus loop
EOF
}

main_pipeline() {
    planner_verification_loop
    simulator_human_gate
    coder

    if grep -q "\[BLOCKER\]" "${REPORTS_DIR}/escalation.md"; then
        interactive_pass
    fi
    
    consensus_loop
    generate_test_trace

    echo "Full SlotCore pipeline completed successfully." 
    log "Full SlotCore pipeline completed successfully."
}


cmd="${1:-}"

case "$cmd" in
    full)
        main_pipeline
        ;;

    planner)
        planner
        ;;

    planner-verifier)
        planner_verifier
        ;;

    planner-verification)
        planner_verification_loop
        ;;

    simulator)
        simulator
        ;;

    simulator-gate)
        simulator_human_gate
        ;;

    coder)
        coder
        ;;

    interactive-pass)
        interactive_pass
        ;;

    tester)
        tester
        ;;

    consensus)
        consensus_loop
        ;;

    generate-test-trace)
        generate_test_trace
        ;;

    ""|-h|--help|help)
        usage
        ;;

    *)
        echo "Unknown command: $cmd"
        usage
        exit 1
        ;;
esac




