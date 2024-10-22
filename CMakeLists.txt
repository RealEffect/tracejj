cmake_minimum_required(VERSION 3.10)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

project(tracejj VERSION 0.0.9 DESCRIPTION "A c++ logging library.")

set(CMAKE_CXX_STANDARD 17) #-stdc++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) #-fPIC

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif(MSVC)

set(TRACEJJ_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
if (NOT DEFINED TRACEJJ_MASTER_PROJECT)
    set(TRACEJJ_MASTER_PROJECT OFF)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(TRACEJJ_MASTER_PROJECT ON)
    endif ()
endif()

# ---
# option list
option(TRACEJJ_BUILD_TESTING "Add testing on build tree, and need GTest." ${TRACEJJ_MASTER_PROJECT})
option(TRACEJJ_BUILD_SHARD "Build as shared library." OFF)
option(USE_GHC_FILESYSTEM "Use gulrak/filesystem replace c++17/std::filesystem." OFF)
if (APPLE)
    option(TRACEJJ_BUILD_FRAMEWORK "Build as FRAMEWORK on Apple platform." OFF)
endif()

set(PRODUCT_PUBLISHER "" CACHE STRING "Product publisher name. This is part of the log path.")
set(BUILD_MAX_LOG_MODS 16 CACHE STRING "Maximum of logging module number.")

# ---
# config all library and test
file(GLOB_RECURSE TRACEJJ_HEADER_FILES "include/*.h" "include/*.hpp")
file(GLOB_RECURSE SRC_FILES "include/*.h" "include/*.hpp" "src/*.h" "src/*.cpp")

if (BUILD_SHARED_LIBS OR TRACEJJ_BUILD_SHARD)
    message(STATUS "tracejj: Build as shared library, BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} TRACEJJ_BUILD_SHARD=${TRACEJJ_BUILD_SHARD}.")
    add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACEJJ_BUILD_FOR_SHARED)
else()
    add_library(${PROJECT_NAME} ${SRC_FILES})
endif()

if (APPLE AND TRACEJJ_BUILD_FRAMEWORK)
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${TRACEJJ_HEADER_FILES}")

if (BUILD_MAX_LOG_MODS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC MAX_LOG_MODS=${BUILD_MAX_LOG_MODS})
    message(STATUS "Maximum of logging module number is ${BUILD_MAX_LOG_MODS}.")
endif()

if (USE_GHC_FILESYSTEM)
    if (NOT TARGET ghc_filesystem)
        find_package(ghc_filesystem QUIET)
        if (ghc_filesystem_FOUND)
            message(STATUS "tracejj: Found ghc_filesystem ${ghc_filesystem_VERSION} in \"${ghc_filesystem_DIR}\".")
        else()
            message(STATUS "tracejj: Use built-in filesystem, not found ghc_filesystem in CMAKE_PREFIX_PATH(\"${CMAKE_PREFIX_PATH}\").")
            add_subdirectory(third/filesystem)
        endif()
    endif()
    target_link_libraries(${PROJECT_NAME} PUBLIC ghc_filesystem)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GHC_FILESYSTEM)
endif()
if (PRODUCT_PUBLISHER)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_PUBLISHER="${PRODUCT_PUBLISHER}")
endif()

if (NOT TARGET fmt::fmt)
    find_package(fmt 9.0 QUIET)
    if (fmt_FOUND)
        message(STATUS "tracejj: Found libfmt ${fmt_VERSION} in \"${fmt_DIR}\".")
    else()
        message(STATUS "tracejj: Use built-in libfmt, not found libfmt in CMAKE_PREFIX_PATH(\"${CMAKE_PREFIX_PATH}\").")
        add_subdirectory(third/fmt)
    endif()
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)

if (TRACEJJ_MASTER_PROJECT)
    if (TRACEJJ_BUILD_TESTING)
        add_subdirectory(test)
    endif()
    add_executable(example "example.cpp")
    target_link_libraries(example tracejj)
    install(
            TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}Targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trace++
            PUBLIC_HEADER DESTINATION /trace++
            FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
endif()
