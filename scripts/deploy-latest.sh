#!/bin/bash
# deploy-latest.sh
# Downloads an Open-Sankore Windows build from GitHub Actions or a release
# into ../sankore-install/ (shared with UTM VM)
#
# Usage:
#   ./scripts/deploy-latest.sh              # latest successful CI build
#   ./scripts/deploy-latest.sh 29185891237  # specific CI run ID
#   ./scripts/deploy-latest.sh --release    # latest release
#   ./scripts/deploy-latest.sh --release v4.0.0  # specific release tag

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
INSTALL_DIR="$(dirname "$PROJECT_DIR")/sankore-install"
MODE="${1:-}"
ARG2="${2:-}"

echo "=== Open-Sankore: Deploy Build ==="
echo "Target: $INSTALL_DIR"
echo ""

# Clean and recreate
rm -rf "$INSTALL_DIR"
mkdir -p "$INSTALL_DIR"

cd "$PROJECT_DIR"

if [ "$MODE" = "--release" ]; then
    # Download from a GitHub Release
    if [ -n "$ARG2" ]; then
        TAG="$ARG2"
    else
        TAG=$(gh release view --json tagName -q '.tagName')
    fi
    echo "Downloading release $TAG..."
    gh release download "$TAG" --pattern "*windows-x64.zip" --dir "$INSTALL_DIR"

    # Unzip
    ZIP_FILE=$(find "$INSTALL_DIR" -name "*.zip" | head -1)
    if [ -n "$ZIP_FILE" ]; then
        unzip -q "$ZIP_FILE" -d "$INSTALL_DIR"
        rm "$ZIP_FILE"
    fi
    BUILD_VERSION="$TAG"
else
    # Download from GitHub Actions
    RUN_ID="$MODE"
    if [ -n "$RUN_ID" ]; then
        echo "Downloading artifact from run #$RUN_ID..."
        gh run download "$RUN_ID" --name open-sankore-windows-x64 --dir "$INSTALL_DIR"
    else
        echo "Downloading latest CI artifact..."
        gh run download --name open-sankore-windows-x64 --dir "$INSTALL_DIR"
    fi

    # Get build info
    if [ -n "$RUN_ID" ]; then
        BUILD_VERSION=$(gh run view "$RUN_ID" --json headSha -q '.headSha[0:8]')
    else
        BUILD_VERSION=$(gh run list --workflow=build-windows.yml --status success --limit 1 --json headSha -q '.[0].headSha[0:8]')
    fi
fi

# Copy the test runner script
cp "$SCRIPT_DIR/run-test.bat" "$INSTALL_DIR/"

FILE_COUNT=$(ls "$INSTALL_DIR" | wc -l | tr -d ' ')

echo ""
echo "================================================"
echo "  Build: $BUILD_VERSION"
echo "  Files: $FILE_COUNT deployed"
echo "  Path:  $INSTALL_DIR"
echo "================================================"
echo ""
echo "In your Windows VM, run: Z:\\sankore-install\\run-test.bat"
