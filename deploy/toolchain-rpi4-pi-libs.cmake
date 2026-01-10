# Toolchain file for cross-compiling to Raspberry Pi 4 using extracted Pi libraries
# Use with: cmake -DCMAKE_TOOLCHAIN_FILE=deploy/toolchain-rpi4-pi-libs.cmake ..

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# System cross-compiler (installed via: sudo apt-get install g++-aarch64-linux-gnu)
set(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++")

# Sysroot with Pi libraries
set(PI_SYSROOT_DIR "${CMAKE_SOURCE_DIR}/../pi-sysroot")
set(CMAKE_SYSROOT "${PI_SYSROOT_DIR}")
set(CMAKE_FIND_ROOT_PATH "${PI_SYSROOT_DIR}")

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Add library search paths
link_directories(
    ${PI_SYSROOT_DIR}/usr/lib
)

# Add include paths
include_directories(
    ${PI_SYSROOT_DIR}/usr/include
)

# Qt5 settings - use native Qt for moc, but link against Pi libraries
set(QT_HOST_PATH "/usr")

# Manually set Qt5 libraries to Pi versions
set(Qt5_LIBRARIES
    ${PI_SYSROOT_DIR}/usr/lib/libQt5Core.so
    ${PI_SYSROOT_DIR}/usr/lib/libQt5Gui.so
    ${PI_SYSROOT_DIR}/usr/lib/libQt5Widgets.so
    ${PI_SYSROOT_DIR}/usr/lib/libQt5Charts.so
    ${PI_SYSROOT_DIR}/usr/lib/libQt5Sql.so
    ${PI_SYSROOT_DIR}/usr/lib/libQt5Svg.so
)

# OpenCV - use Pi libraries
set(OpenCV_LIBRARIES
    ${PI_SYSROOT_DIR}/usr/lib/libopencv_core.so
    ${PI_SYSROOT_DIR}/usr/lib/libopencv_imgproc.so
    ${PI_SYSROOT_DIR}/usr/lib/libopencv_imgcodecs.so
    ${PI_SYSROOT_DIR}/usr/lib/libopencv_videoio.so
)
set(OpenCV_INCLUDE_DIRS "${PI_SYSROOT_DIR}/usr/include")

# ONNX Runtime path - use Pi libraries
set(ONNXRUNTIME_ROOT_DIR "${PI_SYSROOT_DIR}/usr")
set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT_DIR}/include")
set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT_DIR}/lib")
set(ONNXRUNTIME_LIBRARIES "${PI_SYSROOT_DIR}/usr/lib/libonnxruntime.so")

# Compiler flags for Raspberry Pi 4 (Cortex-A72)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+crc -mtune=cortex-a72")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+crc -mtune=cortex-a72")

# Release build optimizations
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

message(STATUS "Cross-compiling for Raspberry Pi 4 (aarch64)")
message(STATUS "Using Pi libraries from: ${PI_SYSROOT_DIR}")
