#!/bin/bash
# Simple cross-compile script for ARM64 GUI version
# Uses aarch64-linux-gnu-g++ and links against Pi libraries

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PI_SYSROOT="$SCRIPT_DIR/../pi-sysroot"
OUTPUT_DIR="$SCRIPT_DIR/build-arm64-gui"
SOURCE_DIR="$SCRIPT_DIR/src"

# Compiler
CC="aarch64-linux-gnu-gcc"
CXX="aarch64-linux-gnu-g++"

# Flags
CFLAGS="-march=armv8-a+crc -mtune=cortex-a72 -fPIC -O3"
CXXFLAGS="$CFLAGS -std=c++17"
LDFLAGS="-L$PI_SYSROOT/usr/lib -Wl,-rpath,/usr/lib"

# Qt5 includes and libraries (from Pi sysroot)
QT5_INCLUDE="-I$PI_SYSROOT/usr/include"
QT5_LIBS="-lQt5Core -lQt5Gui -lQt5Widgets -lQt5Charts -lQt5Sql -lQt5Svg"

# OpenCV
OpenCV_INCLUDE="-I$PI_SYSROOT/usr/include/opencv4"
OpenCV_LIBS="-lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio"

# ONNX Runtime
ONNX_INCLUDE="-I$PI_SYSROOT/usr/include"
ONNX_LIBS="-lonnxruntime"

# SQLite
SQLITE_LIBS="-lsqlite3"

# All includes and libs
INCLUDES="$QT5_INCLUDE $OpenCV_INCLUDE $ONNX_INCLUDE"
LIBS="$QT5_LIBS $OpenCV_LIBS $ONNX_LIBS $SQLITE_LIBS -lpthread -lm"

echo "=================================================="
echo "  Cross-compiling LeafSense for ARM64"
echo "=================================================="
echo "CC: $CC"
echo "CXX: $CXX"
echo "Sysroot: $PI_SYSROOT"
echo ""

mkdir -p "$OUTPUT_DIR"
cd "$OUTPUT_DIR"

# This is a simplified build - for production, use cmake
echo "Building with direct compilation (simplified)..."
echo "Using Qt5, OpenCV, ONNX Runtime from Pi sysroot"
echo ""
echo "NOTE: For full build, run:"
echo "  cd $SCRIPT_DIR"
echo "  mkdir build-arm64-gui && cd build-arm64-gui"
echo "  cmake ../src -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4-pi-libs.cmake -DHEADLESS=OFF"
echo "  make -j\$(nproc)"
echo ""
