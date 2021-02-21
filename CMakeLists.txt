cmake_minimum_required(VERSION 3.15)
project(fastrtps_idlgen)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Find requirements
find_package(fastcdr REQUIRED CONFIG)
find_package(fastrtps REQUIRED CONFIG)

include(cmake/fastrtps_idlgen.cmake)

option(BUILD_EXAMPLES "Build sample example" ON)


add_library(fastrtps_utils INTERFACE
    include/fastrtps_utils/publisher.hpp
    include/fastrtps_utils/subscriber.hpp
    include/fastrtps_utils/publisher_dds.hpp
    include/fastrtps_utils/subscriber_dds.hpp
)
add_library(fastrtps_utils::fastrtps_utils ALIAS fastrtps_utils)
target_include_directories(fastrtps_utils
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/fastrtps_utils>
)
target_link_libraries(fastrtps_utils INTERFACE fastrtps fastcdr)


if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


configure_file(cmake/fastrtps_utils-config.cmake.in fastrtps_utils-config.cmake @ONLY)
install(TARGETS fastrtps_utils EXPORT fastrtps_utils-targets DESTINATION 
    ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(FILES 
        ${CMAKE_CURRENT_BINARY_DIR}/fastrtps_utils-config.cmake 
    DESTINATION 
        lib/cmake/fastrtps_utils)
    
install(EXPORT fastrtps_utils-targets NAMESPACE fastrtps_utils:: DESTINATION lib/cmake/fastrtps_utils)

install(DIRECTORY
	include/fastrtps_utils
    DESTINATION include/fastrtps_utils)
# install cmake configuration files
install(FILES
        cmake/fastrtps_idlgen-config.cmake 
        cmake/fastrtps_idlgen.cmake 
    DESTINATION 
        share/fastrtps_idlgen)



