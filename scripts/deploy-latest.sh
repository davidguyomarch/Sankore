#!/bin/bash
# Open-Sankoré Community Edition
#
# Copyright (C) 2026 David Guyomarch
#
# SPDX-License-Identifier: GPL-3.0-only

# deploy-latest.sh
# Downloads an Open-Sankore Windows build from GitHub Actions or a release
# into ../sankore-install/ (shared with UTM VM)
#
# Usage:
#   ./scripts/deploy-latest.sh                     # latest successful CI build (master)
#   ./scripts/deploy-latest.sh 29185891237         # specific CI run ID
#   ./scripts/deploy-latest.sh --branch <name>     # latest successful build on a branch
#   ./scripts/deploy-latest.sh --pr <number>       # latest successful build on a PR's branch
#   ./scripts/deploy-latest.sh --release           # latest release
#   ./scripts/deploy-latest.sh --release v4.0.0    # specific release tag

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
INSTALL_DIR="$(dirname "$PROJECT_DIR")/sankore-install"
MODE="${1:-}"
ARG2="${2:-}"

WORKFLOW="build-windows.yml"
ARTIFACT="open-sankore-windows-x64"

cd "$PROJECT_DIR"

# ---------------------------------------------------------------------------
# Helper: given a branch, select the most recent successful Windows build run
# and report any more recent in-progress or failed runs on the same branch.
# Sets RESOLVED_RUN_ID and RESOLVED_SHA on success.
# ---------------------------------------------------------------------------
select_run_for_branch() {
    local branch="$1"

    echo "Looking up Windows builds for branch '$branch'..."

    # Pull the recent run history for this branch (newest first).
    local runs
    runs=$(gh run list --workflow="$WORKFLOW" --branch="$branch" --limit 20 \
        --json databaseId,status,conclusion,createdAt,headSha)

    if [ "$(echo "$runs" | jq 'length')" -eq 0 ]; then
        echo "ERROR: no '$WORKFLOW' runs found for branch '$branch'." >&2
        echo "       Has a PR been opened? CI Windows only runs on PRs and master." >&2
        exit 1
    fi

    # Most recent successful run.
    RESOLVED_RUN_ID=$(echo "$runs" | jq -r '[.[] | select(.conclusion=="success")][0].databaseId // empty')
    RESOLVED_SHA=$(echo "$runs" | jq -r '[.[] | select(.conclusion=="success")][0].headSha // empty' | cut -c1-8)

    if [ -z "$RESOLVED_RUN_ID" ]; then
        echo "ERROR: no successful '$WORKFLOW' build found for branch '$branch'." >&2
        echo "       Recent runs:" >&2
        echo "$runs" | jq -r '.[] | "         \(.createdAt)  \(.status)/\(.conclusion // "-")  run \(.databaseId)"' >&2
        exit 1
    fi

    local selected_date
    selected_date=$(echo "$runs" | jq -r --argjson id "$RESOLVED_RUN_ID" \
        '.[] | select(.databaseId==$id) | .createdAt')

    echo "Selected successful build: run $RESOLVED_RUN_ID ($RESOLVED_SHA), $selected_date"

    # Warn about runs that are still in progress (any, since they are newer or concurrent).
    local in_progress
    in_progress=$(echo "$runs" | jq -r '[.[] | select(.status!="completed")] | length')
    if [ "$in_progress" -gt 0 ]; then
        echo ""
        echo "  ⚠  $in_progress build(s) still IN PROGRESS on this branch:"
        echo "$runs" | jq -r '.[] | select(.status!="completed") |
            "       \(.createdAt)  \(.status)  run \(.databaseId)"'
    fi

    # Warn about genuinely FAILED runs more recent than the selected successful one.
    # (cancelled/skipped are ignored — cancelled is the normal outcome for a
    #  push-triggered run superseded by the PR run via the concurrency group.)
    local newer_failed
    newer_failed=$(echo "$runs" | jq -r --arg d "$selected_date" \
        '[.[] | select((.conclusion=="failure" or .conclusion=="timed_out") and .createdAt > $d)] | length')
    if [ "$newer_failed" -gt 0 ]; then
        echo ""
        echo "  ⚠  $newer_failed FAILED build(s) MORE RECENT than the selected one:"
        echo "$runs" | jq -r --arg d "$selected_date" '.[] |
            select((.conclusion=="failure" or .conclusion=="timed_out") and .createdAt > $d) |
            "       \(.createdAt)  \(.conclusion)  run \(.databaseId)"'
        echo ""
        echo "     The deployed build is NOT the newest — a more recent commit failed to build."
    fi

    echo ""
}

echo "=== Open-Sankore: Deploy Build ==="
echo "Target: $INSTALL_DIR"
echo ""

RUN_ID=""
BUILD_VERSION=""
DO_RELEASE=""

# ---------------------------------------------------------------------------
# Resolve what to deploy (before touching the install dir, so failures here
# don't wipe an existing deployment).
# ---------------------------------------------------------------------------
if [ "$MODE" = "--release" ]; then
    DO_RELEASE=1
    if [ -n "$ARG2" ]; then
        TAG="$ARG2"
    else
        TAG=$(gh release view --json tagName -q '.tagName')
    fi
elif [ "$MODE" = "--branch" ]; then
    if [ -z "$ARG2" ]; then
        echo "ERROR: --branch requires a branch name." >&2
        exit 1
    fi
    select_run_for_branch "$ARG2"
    RUN_ID="$RESOLVED_RUN_ID"
    BUILD_VERSION="$RESOLVED_SHA"
elif [ "$MODE" = "--pr" ]; then
    if [ -z "$ARG2" ]; then
        echo "ERROR: --pr requires a PR number." >&2
        exit 1
    fi
    echo "Resolving PR #$ARG2 to its branch..."
    PR_BRANCH=$(gh pr view "$ARG2" --json headRefName -q '.headRefName')
    if [ -z "$PR_BRANCH" ]; then
        echo "ERROR: could not resolve PR #$ARG2." >&2
        exit 1
    fi
    echo "PR #$ARG2 -> branch '$PR_BRANCH'"
    select_run_for_branch "$PR_BRANCH"
    RUN_ID="$RESOLVED_RUN_ID"
    BUILD_VERSION="$RESOLVED_SHA"
else
    # Legacy behavior: explicit run ID, or latest successful build overall.
    RUN_ID="$MODE"
fi

# Clean and recreate only once we know what to fetch.
rm -rf "$INSTALL_DIR"
mkdir -p "$INSTALL_DIR"

# ---------------------------------------------------------------------------
# Download
# ---------------------------------------------------------------------------
if [ -n "$DO_RELEASE" ]; then
    echo "Downloading release $TAG..."
    gh release download "$TAG" --pattern "*windows-x64.zip" --dir "$INSTALL_DIR"

    ZIP_FILE=$(find "$INSTALL_DIR" -name "*.zip" | head -1)
    if [ -n "$ZIP_FILE" ]; then
        unzip -q "$ZIP_FILE" -d "$INSTALL_DIR"
        rm "$ZIP_FILE"
    fi
    BUILD_VERSION="$TAG"
else
    if [ -n "$RUN_ID" ]; then
        echo "Downloading artifact from run #$RUN_ID..."
        gh run download "$RUN_ID" --name "$ARTIFACT" --dir "$INSTALL_DIR"
        if [ -z "$BUILD_VERSION" ]; then
            BUILD_VERSION=$(gh run view "$RUN_ID" --json headSha -q '.headSha[0:8]')
        fi
    else
        echo "Downloading latest successful CI artifact..."
        RUN_ID=$(gh run list --workflow="$WORKFLOW" --status success --limit 1 --json databaseId -q '.[0].databaseId')
        gh run download "$RUN_ID" --name "$ARTIFACT" --dir "$INSTALL_DIR"
        BUILD_VERSION=$(gh run view "$RUN_ID" --json headSha -q '.headSha[0:8]')
    fi
fi

# Copy the test runner script
cp "$SCRIPT_DIR/run-test.bat" "$INSTALL_DIR/"

FILE_COUNT=$(ls "$INSTALL_DIR" | wc -l | tr -d ' ')

echo ""
echo "================================================"
echo "  Build: $BUILD_VERSION"
[ -n "$RUN_ID" ] && echo "  Run:   $RUN_ID"
echo "  Files: $FILE_COUNT deployed"
echo "  Path:  $INSTALL_DIR"
echo "================================================"
echo ""
echo "In your Windows VM, run: Z:\\sankore-install\\run-test.bat"
