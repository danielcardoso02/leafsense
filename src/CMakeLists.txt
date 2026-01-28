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
    set(ONNXRUNTIME_ROOT_DIR "${CMAKE_SOURCE_DIR}/external/onnxruntime-arm64" 
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

# --- libgpiod for GPIO control ---
find_library(GPIOD_LIB gpiod
    PATHS 
        /usr/lib
        /usr/local/lib
        ${CMAKE_SYSROOT}/usr/lib
)
if(GPIOD_LIB)
    message(STATUS "Found libgpiod: ${GPIOD_LIB}")
else()
    message(WARNING "libgpiod not found - GPIO actuators will run in mock mode")
endif()

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
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/application/gui
    ${CMAKE_SOURCE_DIR}/include/application/gui/theme
    ${CMAKE_SOURCE_DIR}/include/application/ml
    ${CMAKE_SOURCE_DIR}/include/middleware
    ${CMAKE_SOURCE_DIR}/include/drivers
    ${CMAKE_SOURCE_DIR}/include/drivers/sensors
    ${CMAKE_SOURCE_DIR}/include/drivers/actuators
    ${SQLite3_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${ONNXRUNTIME_INCLUDE_DIR}
)

# --- Source Files ---
set(PROJECT_SOURCES
    # Entry Point
    ${CMAKE_SOURCE_DIR}/src/main.cpp

    # GUI Headers (for AUTOMOC)
    ${CMAKE_SOURCE_DIR}/include/application/gui/login_dialog.h
    ${CMAKE_SOURCE_DIR}/include/application/gui/mainwindow.h
    ${CMAKE_SOURCE_DIR}/include/application/gui/sensors_display.h
    ${CMAKE_SOURCE_DIR}/include/application/gui/health_display.h
    ${CMAKE_SOURCE_DIR}/include/application/gui/alerts_display.h
    ${CMAKE_SOURCE_DIR}/include/application/gui/leafsense_data_bridge.h
    ${CMAKE_SOURCE_DIR}/include/application/gui/settings_window.h
    ${CMAKE_SOURCE_DIR}/include/application/gui/info_window.h
    ${CMAKE_SOURCE_DIR}/include/application/gui/logs_window.h
    ${CMAKE_SOURCE_DIR}/include/application/gui/analytics_window.h
    ${CMAKE_SOURCE_DIR}/include/application/gui/theme/theme_manager.h

    # GUI
    ${CMAKE_SOURCE_DIR}/src/application/gui/login_dialog.cpp
    ${CMAKE_SOURCE_DIR}/src/application/gui/mainwindow.cpp
    ${CMAKE_SOURCE_DIR}/src/application/gui/sensors_display.cpp
    ${CMAKE_SOURCE_DIR}/src/application/gui/health_display.cpp
    ${CMAKE_SOURCE_DIR}/src/application/gui/alerts_display.cpp
    ${CMAKE_SOURCE_DIR}/src/application/gui/leafsense_data_bridge.cpp
    ${CMAKE_SOURCE_DIR}/src/application/gui/settings_window.cpp
    ${CMAKE_SOURCE_DIR}/src/application/gui/info_window.cpp
    ${CMAKE_SOURCE_DIR}/src/application/gui/logs_window.cpp
    ${CMAKE_SOURCE_DIR}/src/application/gui/analytics_window.cpp
    ${CMAKE_SOURCE_DIR}/src/application/gui/theme/theme_manager.cpp
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc 

    # Middleware (Backend Logic)
    ${CMAKE_SOURCE_DIR}/src/middleware/Master.cpp
    ${CMAKE_SOURCE_DIR}/src/middleware/dDatabase.cpp
    ${CMAKE_SOURCE_DIR}/src/middleware/dbManager.cpp
    ${CMAKE_SOURCE_DIR}/src/middleware/MQueueHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/middleware/IdealConditions.cpp

    # Drivers (Mock Hardware)
    ${CMAKE_SOURCE_DIR}/src/drivers/sensors/Temp.cpp
    ${CMAKE_SOURCE_DIR}/src/drivers/sensors/PH.cpp
    ${CMAKE_SOURCE_DIR}/src/drivers/sensors/TDS.cpp
    ${CMAKE_SOURCE_DIR}/src/drivers/sensors/ADC.cpp
    ${CMAKE_SOURCE_DIR}/src/drivers/sensors/Cam.cpp
    ${CMAKE_SOURCE_DIR}/src/drivers/actuators/AlertLed.cpp
    ${CMAKE_SOURCE_DIR}/src/drivers/actuators/Pumps.cpp
    ${CMAKE_SOURCE_DIR}/src/drivers/actuators/Heater.cpp
    
    # ML (Mock)
    ${CMAKE_SOURCE_DIR}/src/application/ml/ML.cpp
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

# Add libgpiod if available
if(GPIOD_LIB)
    target_link_libraries(LeafSense ${GPIOD_LIB})
endif()