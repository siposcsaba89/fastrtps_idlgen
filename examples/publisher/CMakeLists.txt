cmake_minimum_required(VERSION 3.10)
project(publisher)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (NOT TARGET fstcdr)
    find_package(fastcdr CONFIG REQUIRED)
endif()
if (NOT TARGET fastrtps)
    find_package(fastrtps CONFIG REQUIRED)
endif()

if (NOT TARGET fastrtps_utils::fastrtps_utils)
    find_package(fastrtps_utils CONFIG REQUIRED)
endif()

if (NOT TARGET hello_world_idl_library::hello_world_idl_library)
    find_package(hello_world_idl_library CONFIG REQUIRED)
endif()

add_executable(publisher publisher.cpp)
target_link_libraries(publisher PRIVATE
    fastcdr fastrtps
    hello_world_idl_library::hello_world_idl_library
    fastrtps_utils::fastrtps_utils
)

install(TARGETS publisher)