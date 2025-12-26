cmake_minimum_required(VERSION 3.16)
project(naina LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_OAK "Enable DepthAI Oak ingest" ON)
option(ENABLE_PIXHAWK "Enable Pixhawk MAVSDK ingest" ON)

find_package(OpenCV REQUIRED)

add_executable(naina
    src/main.cpp
    src/blackbox_logger.cpp
    src/time_utils.cpp
    src/display.cpp
)

target_include_directories(naina PRIVATE include)
target_link_libraries(naina PRIVATE ${OpenCV_LIBS})

if(ENABLE_OAK)
    find_package(depthai CONFIG REQUIRED)
    target_sources(naina PRIVATE src/oak_ingest.cpp)
    target_compile_definitions(naina PRIVATE NAINA_ENABLE_OAK=1)
    target_link_libraries(naina PRIVATE depthai::opencv depthai::core)
endif()

if(ENABLE_PIXHAWK)
    find_package(MAVSDK REQUIRED)
    target_sources(naina PRIVATE src/pixhawk_ingest.cpp)
    target_compile_definitions(naina PRIVATE NAINA_ENABLE_PIXHAWK=1)
    target_link_libraries(naina PRIVATE MAVSDK::mavsdk)
endif()
