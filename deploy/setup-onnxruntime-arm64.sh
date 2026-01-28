#!/bin/bash
#
# LeafSense - ONNX Runtime ARM64 Setup Script
# Downloads and prepares ONNX Runtime for Raspberry Pi 4
#

set -e

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
ONNX_VERSION="1.16.3"
ONNX_DIR="$SCRIPT_DIR/../external/onnxruntime-arm64"

echo "==========================================="
echo "  ONNX Runtime Setup for ARM64"
echo "==========================================="
echo ""

mkdir -p "$ONNX_DIR"
cd "$ONNX_DIR"

# Check if already downloaded
if [ -f "lib/libonnxruntime.so" ]; then
    echo "✓ ONNX Runtime already present"
    echo "  Location: $ONNX_DIR"
    exit 0
fi

echo "[1/3] Downloading ONNX Runtime ${ONNX_VERSION} for ARM64..."

# ONNX Runtime provides pre-built ARM64 binaries
ONNX_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-linux-aarch64-${ONNX_VERSION}.tgz"

if ! wget -q --show-progress "$ONNX_URL" -O onnxruntime-arm64.tgz; then
    echo "ERROR: Failed to download ONNX Runtime"
    echo "Trying alternative source..."
    
    # Alternative: Build from source or use different version
    echo "Please download manually from: https://github.com/microsoft/onnxruntime/releases"
    exit 1
fi

echo ""
echo "[2/3] Extracting..."
tar -xzf onnxruntime-arm64.tgz --strip-components=1
rm onnxruntime-arm64.tgz

echo ""
echo "[3/3] Organizing files..."

# Create standard directory structure
mkdir -p lib include

# Move files if needed (depends on archive structure)
if [ -d "lib" ] && [ -f "lib/libonnxruntime.so.${ONNX_VERSION}" ]; then
    # Already in correct structure
    echo "✓ Library files found"
fi

# Create symlink for easier linking
cd lib
if [ -f "libonnxruntime.so.${ONNX_VERSION}" ]; then
    ln -sf "libonnxruntime.so.${ONNX_VERSION}" libonnxruntime.so
fi
cd ..

echo ""
echo "==========================================="
echo "  ONNX Runtime Setup Complete!"
echo "==========================================="
echo ""
echo "Location: $ONNX_DIR"
echo ""
echo "To use in CMake:"
echo "  set(ONNXRUNTIME_ROOT_DIR \"$ONNX_DIR\")"
echo ""
echo "Files:"
ls -la lib/ 2>/dev/null || echo "(lib directory)"
ls -la include/ 2>/dev/null || echo "(include directory)"
echo ""
