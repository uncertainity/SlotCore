#!/usr/bin/env bash
set -uo pipefail

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



AGENTS_FILE="./AGENTS.md"
MEMORY_FILE="./summary.md"

check_file() {
    local file="$1"

    if [[ ! -f "$file" ]]; then
        echo "[ERROR] Missing file: $file"
        return 1
    elif [[ ! -s "$file" ]]; then
        echo "[WARNING] File exists but is empty: $file"
        return 0
    else
        echo "[OK] File found: $file"
        return 0
    fi
}

check_required_folder() {
    local folder="$1"

    if [[ ! -d "$folder" ]]; then
        echo "[ERROR] Missing folder: $folder"
        return 1
    elif [[ -z "$(ls -A "$folder")" ]]; then
        echo "[ERROR] Folder is empty: $folder"
        return 1
    else
        echo "[OK] Folder found: $folder"
        return 0
    fi
}

check_optional_folder() {
    local folder="$1"

    if [[ ! -d "$folder" ]]; then
        echo "[WARNING] Optional folder missing: $folder"
        return 1
    elif [[ -z "$(ls -A "$folder")" ]]; then
        echo "[WARNING] Optional folder is empty: $folder"
        return 1
    else
        echo "[OK] Optional folder found: $folder"
        return 0
    fi
}

check_required_folder "$GAME_RULES_DIR"; err_GAMERULE=$?
check_optional_folder "$MATH_MODELS_DIR"; err_MATHMODEL=$?
check_optional_folder "$ORACLE_DIR"; err_TESTCASE=$?

check_file "$AGENTS_FILE"; err_AGENTS=$?
check_required_folder "$REFERENCE_CPP_DIR"; err_CPP=$?
check_required_folder "$REFERENCE_NUMBA_DIR"; err_NUMBA=$?

set -e

if [[ "$err_GAMERULE" -ne 0 ]]; then
    echo "[FAILED] Required GAMERULE missing. Fix errors before running Codex."
    exit 1
fi

if [[ "$err_AGENTS" -ne 0 ]]; then
    echo "[FAILED] Required AGENTS.md missing. Fix errors before running Codex."
    exit 1
fi


if [[ "$err_CPP" -ne 0 && "$err_NUMBA" -ne 0 ]]; then
    echo "[FAILED] Required REFERENCE LIBRARY missing. Fix errors before running Codex."
    exit 1
fi


if [[ "$err_MATHMODEL" -ne 0 ]]; then
    echo "[WARNING] No Math Model has been provided."
fi

if [[ "$err_TESTCASE" -ne 0 ]]; then
    echo "[WARNING] No Test Case Specification has been provided."
fi

echo
echo "[PASSED] All required inputs are present."