cmake_minimum_required(VERSION 3.10)
project(GarageMonitor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally enable debug logging: cmake -DDEBUG_LOGGING=ON ..
option(DEBUG_LOGGING "Enable debug logging" OFF)
if (DEBUG_LOGGING)
    add_compile_definitions(DEBUG_LOGGING)
endif()

add_library(garage_lib
    Diagnostic.cpp
    Car.cpp
    GarageMonitor.cpp
)
target_include_directories(garage_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(garage main.cpp)
target_link_libraries(garage PRIVATE garage_lib)

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE garage_lib)
