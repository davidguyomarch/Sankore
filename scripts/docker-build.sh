#!/bin/bash
# Open-Sankoré Community Edition — Local Docker Build + Test + Coverage
#
# Copyright (C) 2026 David Guyomarch
# SPDX-License-Identifier: GPL-3.0-only
#
# Usage:
#   ./scripts/docker-build.sh                    # ARM64 native: build + test + coverage
#   ./scripts/docker-build.sh --x64              # x64 via QEMU: build + test + coverage
#   ./scripts/docker-build.sh --build-only       # ARM64: build app only (no tests)
#   ./scripts/docker-build.sh --test-only        # ARM64: run tests only (assumes built)
#   ./scripts/docker-build.sh --x64 --build-only # x64: build app only
#   ./scripts/docker-build.sh --clean            # ARM64: clean build
#   ./scripts/docker-build.sh --x64 --clean      # x64: clean build
#
# Requirements:
#   - Docker Desktop with buildx (QEMU for x64 on ARM host)
#   - Image built: docker build -f Dockerfile.dev -t sankore-dev .

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# --- Defaults ---
PLATFORM=""
DOCKER_IMAGE="sankore-dev"
BUILD_APP=true
BUILD_TESTS=true
RUN_TESTS=true
COVERAGE=true
CLEAN=false

# --- Parse arguments ---
while [[ $# -gt 0 ]]; do
    case "$1" in
        --x64)
            PLATFORM="linux/amd64"
            DOCKER_IMAGE="sankore-dev-x64"
            shift ;;
        --arm64)
            PLATFORM="linux/arm64"
            DOCKER_IMAGE="sankore-dev"
            shift ;;
        --build-only)
            BUILD_TESTS=false
            RUN_TESTS=false
            COVERAGE=false
            shift ;;
        --test-only)
            BUILD_APP=false
            shift ;;
        --no-coverage)
            COVERAGE=false
            shift ;;
        --clean)
            CLEAN=true
            shift ;;
        -h|--help)
            head -15 "$0" | tail -11
            exit 0 ;;
        *)
            echo "Unknown option: $1"
            exit 1 ;;
    esac
done

# --- Detect host arch ---
HOST_ARCH=$(uname -m)
if [[ -z "$PLATFORM" ]]; then
    if [[ "$HOST_ARCH" == "arm64" || "$HOST_ARCH" == "aarch64" ]]; then
        PLATFORM="linux/arm64"
        DOCKER_IMAGE="sankore-dev"
    else
        PLATFORM="linux/amd64"
        DOCKER_IMAGE="sankore-dev-x64"
    fi
fi

ARCH_LABEL=$(echo "$PLATFORM" | sed 's|linux/||')
echo ""
echo "╔══════════════════════════════════════════════════╗"
echo "║  Open-Sankoré — Docker Build ($ARCH_LABEL)"
echo "╚══════════════════════════════════════════════════╝"
echo ""

# --- Check Docker image exists ---
if ! docker image inspect "$DOCKER_IMAGE" > /dev/null 2>&1; then
    echo "Image '$DOCKER_IMAGE' not found. Building..."
    echo ""
    docker build -f "$PROJECT_DIR/Dockerfile.dev" \
        --platform "$PLATFORM" \
        -t "$DOCKER_IMAGE" \
        "$PROJECT_DIR"
    echo ""
    echo "✓ Image '$DOCKER_IMAGE' built."
    echo ""
fi

# --- Docker run wrapper ---
docker_run() {
    local cmd="$1"
    docker run --rm \
        --platform "$PLATFORM" \
        -v "$PROJECT_DIR":/src \
        -w /src \
        -e QT_QPA_PLATFORM=offscreen \
        -e LANG=C.UTF-8 \
        -e LC_ALL=C.UTF-8 \
        "$DOCKER_IMAGE" \
        bash -c "$cmd"
}

# --- Timing ---
SECONDS=0

# --- Step 1: Clean (optional) ---
if $CLEAN; then
    echo "🧹 Cleaning build artifacts..."
    docker_run '
        # Clean object files only (not qmake-generated Makefiles)
        rm -rf build/linux/release/objects/*.o build/linux/release/objects/*.gcda build/linux/release/objects/*.gcno
        rm -rf tests/build/objects/*.o tests/build/objects/*.gcda tests/build/objects/*.gcno
        rm -f tests/build/OpenSankoreTests
        rm -f build/linux/release/product/Open-Sankore
    '
    echo "✓ Clean done."
    echo ""
fi

# --- Step 2: Build app ---
if $BUILD_APP; then
    echo "🔨 Building Open-Sankoré ($ARCH_LABEL)..."
    echo ""

    BUILD_CMD='set -e
        # Generate git version header if not present
        if [ ! -f src/core/git_version.h ]; then
            echo "#define GIT_COMMIT \"local\"" > src/core/git_version.h
            echo "#define GIT_VERSION \"local-dev\"" >> src/core/git_version.h
        fi
        qmake6 OpenSankore.pro CONFIG+=no_webengine'

    # Add coverage flags if coverage is enabled
    if $COVERAGE; then
        BUILD_CMD="$BUILD_CMD \"QMAKE_CXXFLAGS+=--coverage\" \"QMAKE_LFLAGS+=--coverage\""
    fi

    BUILD_CMD="$BUILD_CMD && make -j\$(nproc)"

    docker_run "$BUILD_CMD"

    APP_TIME=$SECONDS
    echo ""
    echo "✓ App built in ${APP_TIME}s."
    echo ""
fi

# --- Step 3: Build tests ---
if $BUILD_TESTS; then
    echo "🧪 Building unit tests ($ARCH_LABEL)..."
    echo ""

    docker_run '
        set -e
        cd tests

        # Clean stale gcov files (version mismatch between GCC versions)
        find build/objects -name "*.gcda" -delete 2>/dev/null || true
        find build/objects -name "*.gcno" -delete 2>/dev/null || true

        qmake6 tests.pro

        # Fix moc_predefs.h for current architecture
        PREDEFS_DIR=$(dirname $(find build -name "moc_predefs.h" 2>/dev/null | head -1) 2>/dev/null || echo "build/moc")
        mkdir -p "$PREDEFS_DIR"
        g++ -pipe --coverage -O2 -std=c++20 -Wall -Wextra -fPIC -dM -E \
            -o "$PREDEFS_DIR/moc_predefs.h" \
            $(find /usr -path "*/mkspecs/features/data/dummy.cpp" 2>/dev/null | head -1)

        # --- Pre-generate moc files without moc_predefs.h ---
        # moc from system Qt cannot parse GCC system headers via moc_predefs.h,
        # producing "No relevant classes found" errors. We run moc manually
        # without --include moc_predefs.h for all affected headers.

        MOC_BIN=$(find /usr -name "moc" -path "*/libexec/*" 2>/dev/null | head -1)
        if [ -z "$MOC_BIN" ]; then
            MOC_BIN=$(which moc6 2>/dev/null || which moc 2>/dev/null)
        fi

        QT_INCLUDE=$(pkg-config --variable=includedir Qt6Core 2>/dev/null || echo "/usr/include/x86_64-linux-gnu/qt6")
        QT_MKSPEC=$(pkg-config --variable=mkspecsdir Qt6Core 2>/dev/null || echo "/usr/lib/qt6/mkspecs")

        MOC_COMMON_FLAGS="-DQT_NO_DEBUG -DQT_SVGWIDGETS_LIB -DQT_WIDGETS_LIB -DQT_SVG_LIB \
            -DQT_GUI_LIB -DQT_TESTLIB_LIB -DQT_XML_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB \
            -I${QT_MKSPEC}/linux-g++ -I. -Istubs -I../src \
            -I${QT_INCLUDE} -I${QT_INCLUDE}/QtCore -I${QT_INCLUDE}/QtGui \
            -I${QT_INCLUDE}/QtWidgets -I${QT_INCLUDE}/QtTest \
            -I${QT_INCLUDE}/QtSvg -I${QT_INCLUDE}/QtSvgWidgets \
            -I${QT_INCLUDE}/QtXml -I${QT_INCLUDE}/QtNetwork"

        mkdir -p premoc

        # UBFileSystemUtils (needed on all Linux)
        $MOC_BIN $MOC_COMMON_FLAGS \
            ../src/frameworks/UBFileSystemUtils.h \
            -o premoc/moc_UBFileSystemUtils.cpp

        # UBOEmbedParser (QObject under test for #229 regression)
        $MOC_BIN $MOC_COMMON_FLAGS \
            ../src/web/UBOEmbedParser.h \
            -o premoc/moc_UBOEmbedParser.cpp

        # Test class headers that moc fails to process with moc_predefs.h
        for HEADER in tst_UBGraphicsScene tst_UBVisualRegression tst_UBRecognition tst_UBSmoothStrokeItem; do
            $MOC_BIN $MOC_COMMON_FLAGS \
                ${HEADER}.h \
                -o premoc/moc_${HEADER}.cpp
        done

        # Patch Makefile: strip system C++ include paths from moc commands
        sed -i "/libexec\/moc/s| -I/usr/include/c++/[^ ]*||g" Makefile 2>/dev/null || true
        sed -i "/libexec\/moc/s| -I/usr/lib/gcc/[^ ]*||g" Makefile 2>/dev/null || true
        sed -i "/libexec\/moc/s| -I/usr/local/include||g" Makefile 2>/dev/null || true
        sed -i "/libexec\/moc/s| -I/usr/include/[a-z0-9_-]*-linux-gnu[^ ]*||g" Makefile 2>/dev/null || true
        sed -i "/libexec\/moc/s| -I/usr/include ||g" Makefile 2>/dev/null || true
        sed -i "/libexec\/moc/s| -I/usr/include$||g" Makefile 2>/dev/null || true

        # Patch Makefile: use premoc outputs instead of build/moc for the 4 test classes
        # The Makefile generates moc_tst_*.cpp in build/moc/ but they are empty.
        # Replace the build/moc/ paths with premoc/ for these files.
        for HEADER in tst_UBGraphicsScene tst_UBVisualRegression tst_UBRecognition tst_UBSmoothStrokeItem; do
            sed -i "s|build/moc/moc_${HEADER}.cpp|premoc/moc_${HEADER}.cpp|g" Makefile 2>/dev/null || true
        done

        make -j$(nproc)
    '

    TESTS_BUILD_TIME=$((SECONDS - ${APP_TIME:-0}))
    echo ""
    echo "✓ Tests built in ${TESTS_BUILD_TIME}s."
    echo ""
fi

# --- Step 4: Run tests ---
if $RUN_TESTS; then
    echo "▶ Running unit tests ($ARCH_LABEL)..."
    echo ""

    # Run tests and capture exit code (don't fail the script on test failure)
    set +e
    docker_run '
        cd tests
        if [ ! -f build/OpenSankoreTests ]; then
            echo "ERROR: test binary not found at tests/build/OpenSankoreTests"
            exit 1
        fi

        ./build/OpenSankoreTests -v2 2>&1
        TEST_EXIT=$?

        echo ""
        echo "=== Tests exit code: $TEST_EXIT ==="
        exit $TEST_EXIT
    '
    TEST_EXIT=$?
    set -e

    TESTS_RUN_TIME=$((SECONDS - ${APP_TIME:-0} - ${TESTS_BUILD_TIME:-0}))
    if [[ $TEST_EXIT -eq 0 ]]; then
        echo ""
        echo "✓ All tests passed in ${TESTS_RUN_TIME}s."
    else
        echo ""
        echo "✗ Some tests failed (exit code: $TEST_EXIT) — ${TESTS_RUN_TIME}s."
    fi
    echo ""
fi

# --- Step 5: Coverage ---
if $COVERAGE && $RUN_TESTS; then
    echo "📊 Generating coverage report ($ARCH_LABEL)..."
    echo ""

    docker_run '
        set -e
        cd tests

        OBJECTS_DIR="build/objects"

        # Check we have .gcda files
        GCDA_COUNT=$(find "$OBJECTS_DIR" -name "*.gcda" 2>/dev/null | wc -l)
        if [ "$GCDA_COUNT" -eq 0 ]; then
            echo "⚠ No coverage data (.gcda files) found."
            echo "  Make sure the app and tests were built with --coverage."
            exit 0
        fi
        echo "Found $GCDA_COUNT .gcda files"

        # Capture raw coverage
        lcov --capture --directory "$OBJECTS_DIR" \
             --output-file build/coverage_raw.info --quiet --ignore-errors inconsistent 2>/dev/null || true

        if [ ! -f build/coverage_raw.info ]; then
            echo "⚠ lcov capture failed."
            exit 0
        fi

        # Filter: keep only src/ code, remove tests/moc/stubs
        lcov --extract build/coverage_raw.info \
             "*/src/frameworks/*" "*/src/core/*" "*/src/document/*" \
             "*/src/adaptors/*" "*/src/web/*" "*/src/domain/*" \
             "*/stubs/*" \
             --output-file build/coverage_filtered.info --quiet --ignore-errors inconsistent,unused 2>/dev/null || true

        if [ ! -s build/coverage_filtered.info ]; then
            # Fallback: remove only system paths instead
            lcov --remove build/coverage_raw.info \
                 "/usr/*" "*/moc_*" "*/premoc/*" "*/build/moc/*" \
                 --output-file build/coverage_filtered.info --quiet --ignore-errors inconsistent,unused 2>/dev/null || true
        fi

        lcov --remove build/coverage_filtered.info \
             "*/tst_*" "*/moc_*" "*/premoc/*" "*/stubs/*" \
             --output-file build/coverage.info --quiet --ignore-errors inconsistent,unused 2>/dev/null || true

        # Summary
        echo ""
        echo "╔══════════════════════════════════════════════════╗"
        echo "║          UNIT TEST CODE COVERAGE                ║"
        echo "╚══════════════════════════════════════════════════╝"
        lcov --summary build/coverage.info --ignore-errors inconsistent 2>&1 || true
        echo ""

        # Per-file breakdown
        echo "╔══════════════════════════════════════════════════════════╗"
        echo "║  File                          Lines     Coverage      ║"
        echo "╠══════════════════════════════════════════════════════════╣"

        TOTAL_HIT=0
        TOTAL_FOUND=0
        CURRENT_FILE=""
        FILE_HIT=0
        FILE_FOUND=0

        while IFS= read -r line; do
            case "$line" in
                SF:*)
                    CURRENT_FILE="${line#SF:}"
                    CURRENT_FILE=$(basename "$CURRENT_FILE")
                    FILE_HIT=0; FILE_FOUND=0 ;;
                LF:*) FILE_FOUND="${line#LF:}" ;;
                LH:*) FILE_HIT="${line#LH:}" ;;
                end_of_record)
                    if [ "$FILE_FOUND" -gt 0 ]; then
                        PERCENT=$(awk "BEGIN {printf \"%.1f\", ($FILE_HIT/$FILE_FOUND)*100}")
                        printf "║  %-28s  %4s/%-4s  %6s%%     ║\n" "$CURRENT_FILE" "$FILE_HIT" "$FILE_FOUND" "$PERCENT"
                        TOTAL_HIT=$((TOTAL_HIT + FILE_HIT))
                        TOTAL_FOUND=$((TOTAL_FOUND + FILE_FOUND))
                    fi ;;
            esac
        done < build/coverage.info

        echo "╠══════════════════════════════════════════════════════════╣"
        if [ "$TOTAL_FOUND" -gt 0 ]; then
            TOTAL_PERCENT=$(awk "BEGIN {printf \"%.1f\", ($TOTAL_HIT/$TOTAL_FOUND)*100}")
            printf "║  %-28s  %4s/%-4s  %6s%%     ║\n" "TOTAL" "$TOTAL_HIT" "$TOTAL_FOUND" "$TOTAL_PERCENT"
        else
            printf "║  %-28s  %9s  %11s  ║\n" "TOTAL" "N/A" "N/A"
        fi
        echo "╚══════════════════════════════════════════════════════════╝"

        # HTML report
        genhtml build/coverage.info \
            --output-directory build/coverage_html \
            --title "Open-Sankoré Unit Test Coverage" \
            --quiet --ignore-errors inconsistent 2>/dev/null || true

        if [ -d build/coverage_html ]; then
            echo ""
            echo "HTML report: tests/build/coverage_html/index.html"
        fi

        # Cleanup
        rm -f build/coverage_raw.info build/coverage_filtered.info
    '

    echo ""
fi

# --- Summary ---
TOTAL_TIME=$SECONDS
echo "╔══════════════════════════════════════════════════╗"
echo "║  Done — Total time: ${TOTAL_TIME}s ($ARCH_LABEL)"
echo "╚══════════════════════════════════════════════════╝"
echo ""
