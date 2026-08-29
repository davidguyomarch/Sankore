#!/bin/bash
# Open-Sankoré Community Edition
#
# Copyright (C) 2026 David Guyomarch
#
# SPDX-License-Identifier: GPL-3.0-only

# Run Open-Sankoré unit tests with summary table and code coverage
# Usage: ./run_tests.sh

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TEST_BIN="$SCRIPT_DIR/build/OpenSankoreTests"
OBJECTS_DIR="$SCRIPT_DIR/build/objects"

if [ ! -x "$TEST_BIN" ]; then
    echo "Error: test binary not found. Run 'make' first."
    exit 1
fi

# Reset coverage counters
lcov --zerocounters --directory "$OBJECTS_DIR" --quiet 2>/dev/null

# Run tests and capture output
OUTPUT=$("$TEST_BIN" 2>&1)
EXIT_CODE=$?

# Print full output
echo "$OUTPUT"

# Parse results and build summary
echo ""
echo "╔══════════════════════════════════════════════════════╗"
echo "║              TEST RESULTS SUMMARY                   ║"
echo "╠══════════════════════════╦════════╦════════╦════════╣"
echo "║ Suite                    ║ Passed ║ Failed ║ Skipped║"
echo "╠══════════════════════════╬════════╬════════╬════════╣"

TOTAL_PASSED=0
TOTAL_FAILED=0
TOTAL_SKIPPED=0

# Extract suite names
SUITE_INDEX=0
while IFS= read -r line; do
    if echo "$line" | grep -q "Start testing of"; then
        SUITE_NAME=$(echo "$line" | sed 's/.*Start testing of \(.*\) \*.*/\1/')
        SUITES[$SUITE_INDEX]="$SUITE_NAME"
        SUITE_INDEX=$((SUITE_INDEX + 1))
    fi
done <<< "$OUTPUT"

# Extract results per suite
RESULT_INDEX=0
while IFS= read -r line; do
    if echo "$line" | grep -q "^Totals:"; then
        PASSED=$(echo "$line" | grep -o '[0-9]* passed' | grep -o '[0-9]*')
        FAILED=$(echo "$line" | grep -o '[0-9]* failed' | grep -o '[0-9]*')
        SKIPPED=$(echo "$line" | grep -o '[0-9]* skipped' | grep -o '[0-9]*')
        SUITE_NAME="${SUITES[$RESULT_INDEX]}"
        printf "║ %-24s ║ %6s ║ %6s ║ %6s ║\n" "$SUITE_NAME" "$PASSED" "$FAILED" "$SKIPPED"
        TOTAL_PASSED=$((TOTAL_PASSED + PASSED))
        TOTAL_FAILED=$((TOTAL_FAILED + FAILED))
        TOTAL_SKIPPED=$((TOTAL_SKIPPED + SKIPPED))
        RESULT_INDEX=$((RESULT_INDEX + 1))
    fi
done <<< "$OUTPUT"

echo "╠══════════════════════════╬════════╬════════╬════════╣"
printf "║ %-24s ║ %6s ║ %6s ║ %6s ║\n" "TOTAL" "$TOTAL_PASSED" "$TOTAL_FAILED" "$TOTAL_SKIPPED"
echo "╚══════════════════════════╩════════╩════════╩════════╝"

if [ $TOTAL_FAILED -eq 0 ]; then
    echo ""
    echo "✓ All tests passed!"
else
    echo ""
    echo "✗ $TOTAL_FAILED test(s) failed!"
fi

# --- Code Coverage ---
echo ""
echo "Generating code coverage report..."

# Check if lcov is available
if ! command -v lcov &> /dev/null; then
    echo "Warning: lcov not installed. Skipping coverage report."
    exit $EXIT_CODE
fi

# Capture coverage data
lcov --capture --directory "$OBJECTS_DIR" --output-file "$SCRIPT_DIR/coverage_raw.info" --quiet 2>/dev/null

if [ ! -f "$SCRIPT_DIR/coverage_raw.info" ]; then
    echo "Warning: could not generate coverage data."
    exit $EXIT_CODE
fi

# Filter to only include our stubs (the code under test)
lcov --extract "$SCRIPT_DIR/coverage_raw.info" "*/stubs/*" "*/src/frameworks/*" "*/src/core/*" "*/src/document/*" \
     --output-file "$SCRIPT_DIR/coverage.info" --quiet 2>/dev/null

# Remove test files themselves and moc files from coverage
lcov --remove "$SCRIPT_DIR/coverage.info" "*/tst_*" "*/moc_*" "*/build/*" \
     --output-file "$SCRIPT_DIR/coverage.info" --quiet 2>/dev/null

# Generate HTML report
genhtml "$SCRIPT_DIR/coverage.info" --output-directory "$SCRIPT_DIR/coverage_report" --quiet 2>/dev/null

# Parse and display coverage summary
echo ""
echo "╔══════════════════════════════════════════════════════╗"
echo "║              CODE COVERAGE SUMMARY                  ║"
echo "╠══════════════════════════════╦═══════════╦══════════╣"
echo "║ File                         ║ Lines     ║ Coverage ║"
echo "╠══════════════════════════════╬═══════════╬══════════╣"

TOTAL_HIT=0
TOTAL_FOUND=0

# Parse lcov info file for per-file coverage
if [ -f "$SCRIPT_DIR/coverage.info" ]; then
    CURRENT_FILE=""
    FILE_HIT=0
    FILE_FOUND=0

    while IFS= read -r line; do
        case "$line" in
            SF:*)
                CURRENT_FILE="${line#SF:}"
                CURRENT_FILE=$(basename "$CURRENT_FILE")
                FILE_HIT=0
                FILE_FOUND=0
                ;;
            LF:*)
                FILE_FOUND="${line#LF:}"
                ;;
            LH:*)
                FILE_HIT="${line#LH:}"
                ;;
            end_of_record)
                if [ "$FILE_FOUND" -gt 0 ]; then
                    PERCENT=$(awk "BEGIN {printf \"%.1f\", ($FILE_HIT/$FILE_FOUND)*100}")
                    printf "║ %-28s ║ %4s/%-4s ║ %6s%% ║\n" "$CURRENT_FILE" "$FILE_HIT" "$FILE_FOUND" "$PERCENT"
                    TOTAL_HIT=$((TOTAL_HIT + FILE_HIT))
                    TOTAL_FOUND=$((TOTAL_FOUND + FILE_FOUND))
                fi
                ;;
        esac
    done < "$SCRIPT_DIR/coverage.info"
fi

echo "╠══════════════════════════════╬═══════════╬══════════╣"
if [ "$TOTAL_FOUND" -gt 0 ]; then
    TOTAL_PERCENT=$(awk "BEGIN {printf \"%.1f\", ($TOTAL_HIT/$TOTAL_FOUND)*100}")
    printf "║ %-28s ║ %4s/%-4s ║ %6s%% ║\n" "TOTAL" "$TOTAL_HIT" "$TOTAL_FOUND" "$TOTAL_PERCENT"
else
    printf "║ %-28s ║ %9s ║ %8s ║\n" "TOTAL" "N/A" "N/A"
fi
echo "╚══════════════════════════════╩═══════════╩══════════╝"

if [ -d "$SCRIPT_DIR/coverage_report" ]; then
    echo ""
    echo "HTML report: tests/coverage_report/index.html"
fi

# Cleanup intermediate files
rm -f "$SCRIPT_DIR/coverage_raw.info"

exit $EXIT_CODE
