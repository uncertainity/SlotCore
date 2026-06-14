#!/usr/bin/env bash
set -euo pipefail

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

GAME_DIR=""



RULE_FILES=()
MATH_MODELS=()
TEST_CASES=()

# ./stage_specs.sh --game-dir ./Vegas_Blitz --rule-files Parsheet_Vegas_Blitz.xlsx Vegas_Blitz_Game_Rules_UKGC_V1.docx --math-models Parsheet_Vegas_Blitz.xlsx --test-cases Vegas_Blitz.xlsx 

while [[ $# -gt 0 ]]; do
    case "$1" in 
    --game-dir)
        GAME_DIR="$2"
        shift 2
        ;;
    --rule-files)
        shift
        while [[ $# -gt 0 && "$1" != --* ]]; do
            RULE_FILES+=("$1")
            shift
        done
        ;;
    --math-models)
        shift
        while [[ $# -gt 0 && "$1" != --* ]]; do
            MATH_MODELS+=("$1")
            shift
        done
        ;;
    --test-cases)
        shift
        while [[ $# -gt 0 && "$1" != --* ]];do
            TEST_CASES+=("$1")
            shift
        done
        ;;
    *)
        echo "Unknown argument: $1"
        exit 1
        ;;
    esac
done

echo "GAME_DIR  = $GAME_DIR"
echo "Rule files:"
for file in "${RULE_FILES[@]}"; do
    cp "${GAME_DIR}/${file}" "${GAME_RULES_DIR}"
    echo "${file} is moved in ${GAME_RULES_DIR}"
done    

echo "Math models:"
for file in "${MATH_MODELS[@]}"; do
    cp "${GAME_DIR}/${file}" "${MATH_MODELS_DIR}"
    echo "${file} is moved in ${MATH_MODELS_DIR}"
done

echo "Test Cases:"
for file in "${TEST_CASES[@]}"; do
    cp "${GAME_DIR}/${file}" "${ORACLE_DIR}"
    echo "${file} is moved in ${ORACLE_DIR}"
done
