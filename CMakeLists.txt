cmake_minimum_required(VERSION 3.16)

project(turism_project VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Gui)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    agency.h
    agency_types.h
    address.cpp
    address.h
    animal.cpp
    animal.h
    client.cpp
    client.h
    client_service.cpp
    client_service.h
    document.cpp
    document.h
    document_service.cpp
    document_service.h
    documents_dialog.cpp
    documents_dialog.h
    tour.cpp
    tour.h
    tour_request.cpp
    tour_request.h
    tourist.cpp
    tourist.h
    travel_agency.cpp
    travel_agency.h
    request_service.cpp
    request_service.h
    validation_service.cpp
    validation_service.h
)

add_executable(turism_project ${PROJECT_SOURCES})

target_link_libraries(turism_project PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
)

set_target_properties(turism_project PROPERTIES
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_target(turism_project)
endif()

# --- Тестовые случаи ---
add_executable(turism_project_tests
    tests/tests.cpp
    address.cpp
    animal.cpp
    client.cpp
    client_service.cpp
    document.cpp
    document_service.cpp
    tour.cpp
    tour_request.cpp
    tourist.cpp
    travel_agency.cpp
    request_service.cpp
    validation_service.cpp
)
target_include_directories(turism_project_tests PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(turism_project_tests PRIVATE Qt${QT_VERSION_MAJOR}::Core)
enable_testing()
add_test(NAME turism_project_tests COMMAND turism_project_tests)
