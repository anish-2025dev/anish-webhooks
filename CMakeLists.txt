cmake_minimum_required(VERSION 3.20)

project(reliable_event_platform
    VERSION 0.1.0
    LANGUAGES CXX
)

find_package(Boost REQUIRED)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(shared)
add_subdirectory(api)
add_subdirectory(broker)
add_subdirectory(worker)

include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(api PRIVATE shared)
target_link_libraries(broker PRIVATE shared)
target_link_libraries(worker PRIVATE shared)