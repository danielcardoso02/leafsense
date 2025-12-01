cmake_minimum_required(VERSION 3.16)

set(CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake/Qt5" CACHE PATH "Qt5 installation path")

project(LeafSense LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)

# --- Dependencies ---
find_package(Qt5 COMPONENTS Core Gui Widgets Sql Charts REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

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
    src/drivers/actuators/Pumps.cpp
    src/drivers/actuators/Heater.cpp
    
    # ML (Mock)
    src/application/ml/ML.cpp
)

add_executable(LeafSense ${PROJECT_SOURCES})

target_link_libraries(LeafSense
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Sql Qt5::Charts
    SQLite::SQLite3
    Threads::Threads
    ${OpenCV_LIBS}
)