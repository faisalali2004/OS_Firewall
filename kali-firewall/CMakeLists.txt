cmake_minimum_required(VERSION 3.10)
project(kali-firewall)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(NFQUEUE REQUIRED libnetfilter_queue)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

include_directories(
    core
    gui
    process_monitor
    /usr/include/nlohmann
    ${NFQUEUE_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
)

add_executable(firewall
    main.cpp
    core/packet_capture.cpp
    core/rule_engine.cpp
    core/dpi_engine.cpp
    core/traffic_shaper.cpp
    core/logger.cpp
    process_monitor/proc_tracker.cpp
    gui/mainwindow.cpp
    gui/log_viewer.cpp
    gui/rule_editor.cpp
    gui/traffic_shaper_ui.cpp
    gui/dpi_manager.cpp
    gui/dashboard.cpp
)

target_link_libraries(firewall
    Qt5::Widgets
    ${NFQUEUE_LIBRARIES}
    ${SQLITE3_LIBRARIES}
)