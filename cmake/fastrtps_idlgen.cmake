
find_program(JAVA_EXE NAMES java java.exe)
if (JAVA_EXE-NOTFOUND)
    message(FATAL_ERROR "Cannot find java")
endif()
find_program(FASTDDS_GEN NAMES fastddsgen fastddsgen.bat)
if (FASTDDS_GEN-NOTFOUND)
    message(FATAL_ERROR "Cannot find fastddsgen")
endif()

message(STATUS "fastddsgen found: ${FASTDDS_GEN}")
# Find requirements
find_package(fastcdr REQUIRED CONFIG)
find_package(fastrtps REQUIRED CONFIG)



function(fastrtps_idlgen)
    set(options INSTALL VERBOSE)
    set(oneValueArgs NAME)
    set(multiValueArgs IDLS INCLUDES DEPENDS)
    cmake_parse_arguments(IDL_GEN "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )
               
    set(IDLS_ABSOLUTE )
    set(OUT_FILES_HDRS )
    set(OUT_FILES_SRCS )
    foreach(idl ${IDL_GEN_IDLS})
        if (NOT IS_ABSOLUTE ${idl})
            list(APPEND IDLS_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR}/${idl})
        else()
            list(APPEND IDLS_ABSOLUTE ${idl})
        endif()
        
        get_filename_component(idl_FNAME_WEXT ${idl} NAME_WE)
        list(APPEND OUT_FILES_HDRS ${CMAKE_CURRENT_BINARY_DIR}/${idl_FNAME_WEXT}.h
            ${CMAKE_CURRENT_BINARY_DIR}/${idl_FNAME_WEXT}PubSubTypes.h)
        list(APPEND OUT_FILES_SRCS ${CMAKE_CURRENT_BINARY_DIR}/${idl_FNAME_WEXT}.cxx 
            ${CMAKE_CURRENT_BINARY_DIR}/${idl_FNAME_WEXT}PubSubTypes.cxx)
    endforeach()
    if (IDL_GEN_VERBOSE)
        message(STATUS ${IDLS_ABSOLUTE})
        message(STATUS "Generated msg header files: ${OUT_FILES_HDRS}")
        message(STATUS "Generated msg souce files ${OUT_FILES_SRCS}")
    endif()
    
    foreach(dep ${IDL_GEN_DEPENDS})
        message(STATUS "Trying to find dependency include: ${dep}")
        get_target_property(dep_INCLUDES ${dep} INTERFACE_INCLUDE_DIRECTORIES)
        list(APPEND IDL_GEN_INCLUDES ${dep_INCLUDES})
        message(STATUS "Adding include ${dep_INCLUDES}")
    endforeach()
    
    set(INCLUDE_DIRS )
    if (DEFINED IDL_GEN_INCLUDES)
        foreach(INCLUDE_DIR ${IDL_GEN_INCLUDES})
            if (NOT ${INCLUDE_DIR} IN_LIST INCLUDE_DIRS)
                list(APPEND INCLUDE_DIRS -I "${INCLUDE_DIR}")
            endif()
        endforeach()
    endif()
    if (IDL_GEN_VERBOSE)
        message(STATUS "IDL include directories ${INCLUDE_DIRS}")
    endif()
        
    add_custom_command(OUTPUT ${OUT_FILES_HDRS} ${OUT_FILES_SRCS}
        COMMAND
            ${FASTDDS_GEN} -replace ${IDLS_ABSOLUTE} ${INCLUDE_DIRS}
        DEPENDS ${IDLS_ABSOLUTE})
        
    # adding library
    add_library(${IDL_GEN_NAME} ${OUT_FILES_HDRS} ${OUT_FILES_SRCS} ${IDL_GEN_IDLS})
    add_library(${IDL_GEN_NAME}::${IDL_GEN_NAME} ALIAS ${IDL_GEN_NAME})
    set_source_files_properties(${IDL_GEN_IDLS} PROPERTIES HEADER_FILE_ONLY TRUE)
    source_group(idls FILES ${IDL_GEN_IDLS})
    source_group(srcs FILES ${OUT_FILES_SRCS})
    source_group(${IDL_GEN_NAME} FILES ${OUT_FILES_HDRS})
    set_target_properties(${IDL_GEN_NAME}
        PROPERTIES
            DEBUG_POSTFIX _d
            RELWITHDEBINFO_POSTFIX _rd
            MINSIZEREL_POSTFIX _mr
        FOLDER generated_idl_library
    )
    target_compile_options(${IDL_GEN_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/MP>)
    target_link_libraries(${IDL_GEN_NAME} PUBLIC fastcdr fastrtps ${IDL_GEN_DEPENDS})
    target_include_directories(${IDL_GEN_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
            $<INSTALL_INTERFACE:include/${IDL_GEN_NAME}>
    )
    
    if (IDL_GEN_INSTALL)
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${IDL_GEN_NAME}-config.cmake
            "include(CMakeFindDependencyMacro) \n"
            "find_dependency(fastcdr CONFIG) \n"
            "find_dependency(fastrtps CONFIG) \n"
            "include(\${CMAKE_CURRENT_LIST_DIR}/${IDL_GEN_NAME}-targets.cmake) \n"
        )
        install(TARGETS ${IDL_GEN_NAME} EXPORT ${IDL_GEN_NAME}-targets DESTINATION 
            ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
        install(FILES 
                ${CMAKE_CURRENT_BINARY_DIR}/${IDL_GEN_NAME}-config.cmake 
            DESTINATION 
                lib/cmake/${IDL_GEN_NAME})
            
        install(EXPORT ${IDL_GEN_NAME}-targets NAMESPACE ${IDL_GEN_NAME}:: DESTINATION lib/cmake/${IDL_GEN_NAME})
        
        install(FILES
            ${OUT_FILES_HDRS}
            ${IDL_GEN_IDLS}
            DESTINATION include/${IDL_GEN_NAME})
    endif()
endfunction()
