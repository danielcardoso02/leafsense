# Toolchain file for cross-compiling to Raspberry Pi 4 (64-bit ARM)
# Use with: cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-rpi4.cmake ..

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Buildroot toolchain path
set(BUILDROOT_PATH "$ENV{HOME}/buildroot/buildroot-2025.08")
set(TOOLCHAIN_PATH "${BUILDROOT_PATH}/output/host")
set(SYSROOT_PATH "${BUILDROOT_PATH}/output/staging")

# Cross compiler
set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/bin/aarch64-linux-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/bin/aarch64-linux-g++")

# Sysroot for libraries
set(CMAKE_SYSROOT "${SYSROOT_PATH}")
set(CMAKE_FIND_ROOT_PATH "${SYSROOT_PATH}")

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Qt5 specific settings
set(QT_HOST_PATH "${TOOLCHAIN_PATH}")
set(Qt5_DIR "${SYSROOT_PATH}/usr/lib/cmake/Qt5")
set(Qt5Core_DIR "${SYSROOT_PATH}/usr/lib/cmake/Qt5Core")
set(Qt5Gui_DIR "${SYSROOT_PATH}/usr/lib/cmake/Qt5Gui")
set(Qt5Widgets_DIR "${SYSROOT_PATH}/usr/lib/cmake/Qt5Widgets")
set(Qt5Charts_DIR "${SYSROOT_PATH}/usr/lib/cmake/Qt5Charts")

# OpenCV specific settings
set(OpenCV_DIR "${SYSROOT_PATH}/usr/lib/cmake/opencv4")

# ONNX Runtime path (will need to be installed separately)
set(ONNXRUNTIME_ROOT_DIR "${SYSROOT_PATH}/usr")

# Compiler flags for Raspberry Pi 4 (Cortex-A72)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+crc -mtune=cortex-a72")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+crc -mtune=cortex-a72")

# Release build optimizations
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

message(STATUS "Cross-compiling for Raspberry Pi 4 (aarch64)")
message(STATUS "Toolchain: ${TOOLCHAIN_PATH}")
message(STATUS "Sysroot: ${SYSROOT_PATH}")
