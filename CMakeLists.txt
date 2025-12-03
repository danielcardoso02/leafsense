cmake_minimum_required(VERSION 3.16)

project(LeafSense LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)

# --- Cross-compilation detection ---
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for: ${CMAKE_SYSTEM_PROCESSOR}")
    
    # ONNX Runtime for ARM64
    set(ONNXRUNTIME_ROOT_DIR "${PROJECT_SOURCE_DIR}/external/onnxruntime-arm64" 
        CACHE PATH "ONNX Runtime ARM64 directory")
else()
    message(STATUS "Native build for: ${CMAKE_SYSTEM_PROCESSOR}")
    
    # Qt5 path for native build (x86_64)
    set(CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake/Qt5" CACHE PATH "Qt5 installation path")
    
    # ONNX Runtime for x86_64
    set(ONNXRUNTIME_ROOT_DIR "/usr/local" CACHE PATH "ONNX Runtime directory")
endif()

# --- Dependencies ---
find_package(Qt5 COMPONENTS Core Gui Widgets Sql Charts REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

# --- ONNX Runtime ---
# For cross-compilation, use the ARM64 version directly
if(CMAKE_CROSSCOMPILING)
    set(ONNXRUNTIME_LIB "${ONNXRUNTIME_ROOT_DIR}/lib/libonnxruntime.so" CACHE FILEPATH "ONNX Runtime library")
    if(EXISTS ${ONNXRUNTIME_LIB})
        message(STATUS "Found ONNX Runtime (ARM64): ${ONNXRUNTIME_LIB}")
        set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT_DIR}/include" 
            CACHE PATH "ONNX Runtime include directory")
    else()
        message(WARNING "ONNX Runtime ARM64 not found at ${ONNXRUNTIME_LIB} - ML will run in mock mode")
        unset(ONNXRUNTIME_LIB)
        add_compile_definitions(MOCK_ML=1)
    endif()
else()
    # Native build - search normally
    find_library(ONNXRUNTIME_LIB onnxruntime
        PATHS 
            ${ONNXRUNTIME_ROOT_DIR}/lib
            /usr/local/lib 
            /usr/lib
        NO_DEFAULT_PATH
    )
    
    # Fallback to default paths if not found
    if(NOT ONNXRUNTIME_LIB)
        find_library(ONNXRUNTIME_LIB onnxruntime)
    endif()

    if(ONNXRUNTIME_LIB)
        message(STATUS "Found ONNX Runtime: ${ONNXRUNTIME_LIB}")
        set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT_DIR}/include" 
            CACHE PATH "ONNX Runtime include directory")
    else()
        message(WARNING "ONNX Runtime not found - ML will run in mock mode")
        add_compile_definitions(MOCK_ML=1)
    endif()
endif()

# --- Include Paths ---
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/application/gui
    ${PROJECT_SOURCE_DIR}/include/application/gui/theme
    ${PROJECT_SOURCE_DIR}/include/application/ml
    ${PROJECT_SOURCE_DIR}/include/middleware
    ${PROJECT_SOURCE_DIR}/include/drivers
    ${PROJECT_SOURCE_DIR}/include/drivers/sensors
    ${PROJECT_SOURCE_DIR}/include/drivers/actuators
    ${SQLite3_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${ONNXRUNTIME_INCLUDE_DIR}
)

# --- Source Files ---
set(PROJECT_SOURCES
    # Entry Point
    src/main.cpp

    # GUI Headers (for AUTOMOC)
    include/application/gui/login_dialog.h
    include/application/gui/mainwindow.h
    include/application/gui/sensors_display.h
    include/application/gui/health_display.h
    include/application/gui/alerts_display.h
    include/application/gui/leafsense_data_bridge.h
    include/application/gui/settings_window.h
    include/application/gui/info_window.h
    include/application/gui/logs_window.h
    include/application/gui/analytics_window.h
    include/application/gui/theme/theme_manager.h

    # GUI
    src/application/gui/login_dialog.cpp
    src/application/gui/mainwindow.cpp
    src/application/gui/sensors_display.cpp
    src/application/gui/health_display.cpp
    src/application/gui/alerts_display.cpp
    src/application/gui/leafsense_data_bridge.cpp
    src/application/gui/settings_window.cpp
    src/application/gui/info_window.cpp
    src/application/gui/logs_window.cpp
    src/application/gui/analytics_window.cpp
    src/application/gui/theme/theme_manager.cpp
    resources/resources.qrc 

    # Middleware (Backend Logic)
    src/middleware/Master.cpp
    src/middleware/dDatabase.cpp
    src/middleware/dbManager.cpp
    src/middleware/MQueueHandler.cpp
    src/middleware/IdealConditions.cpp

    # Drivers (Mock Hardware)
    src/drivers/sensors/Temp.cpp
    src/drivers/sensors/PH.cpp
    src/drivers/sensors/TDS.cpp
    src/drivers/sensors/ADC.cpp
    src/drivers/sensors/Cam.cpp
    src/drivers/actuators/AlertLed.cpp
    src/drivers/actuators/Pumps.cpp
    src/drivers/actuators/Heater.cpp
    
    # ML (Mock)
    src/application/ml/ML.cpp
)

add_executable(LeafSense ${PROJECT_SOURCES})

# Base libraries
target_link_libraries(LeafSense
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Sql Qt5::Charts
    SQLite::SQLite3
    Threads::Threads
    ${OpenCV_LIBS}
)

# Add ONNX Runtime only if available
if(ONNXRUNTIME_LIB)
    target_link_libraries(LeafSense ${ONNXRUNTIME_LIB})
endif()