cmake_minimum_required(VERSION 3.10)
project(fastrtps_utils_examples)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (NOT TARGET fstcdr)
    find_package(fastcdr CONFIG REQUIRED)
endif()
if (NOT TARGET fastrtps)
    find_package(fastrtps CONFIG REQUIRED)
endif()

if (NOT COMMAND fastrtps_idlgen)
    find_package(fastrtps_idlgen REQUIRED)
endif()

fastrtps_idlgen(
    NAME hello_world_idl_library
    IDLS
        idls/hello_world.idl
        idls/hello_world_include.idl 
    #INSTALL
    INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/idls
    DEPENDS
)

add_subdirectory(subscriber)
add_subdirectory(publisher)

