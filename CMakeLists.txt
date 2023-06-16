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
option(BUILD_TESTING "Add testing on build tree, and need GTest." ON)
option(BUILD_FRAMEWORK "Build as FRAMEWORK on Apple macOS or iOS." OFF)
if (WIN32)
    option(USE_GHC_FILESYSTEM "Use gulrak/filesystem replace c++17/std::filesystem." OFF)
else()
    option(USE_GHC_FILESYSTEM "Use gulrak/filesystem replace c++17/std::filesystem." ON) # std::filesystem introduced in macOS10.15 GCC8.X
endif()

set(PRODUCT_PUBLISHER "" CACHE STRING "Product publisher name. This is part of the log path.")
set(BUILD_MAX_LOG_MODS 16 CACHE STRING "Maximum of logging module number.")

# ---
# config all library and test
file(GLOB_RECURSE TRACEJJ_HEADER_FILES "include/*.h" "include/*.hpp")
file(GLOB_RECURSE SRC_FILES "include/*.h" "include/*.hpp" "src/*.h" "src/*.cpp")
add_library(${PROJECT_NAME} ${SRC_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${TRACEJJ_HEADER_FILES}")
if (BUILD_SHARED_LIBS OR BUILD_FRAMEWORK)
    message(STATUS "tracejj: Build as shared/FRAMEWORK library.")
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACEJJ_BUILD_FOR_SHARED)
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
    )
endif()

if (BUILD_MAX_LOG_MODS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC MAX_LOG_MODS=${BUILD_MAX_LOG_MODS})
    message(STATUS "Maximum of logging module number is ${BUILD_MAX_LOG_MODS}.")
endif()

if (USE_GHC_FILESYSTEM)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GHC_FILESYSTEM)
    find_package(ghc_filesystem 9.0 QUIET)
    if (ghc_filesystem_FOUND)
        message(STATUS "tracejj: Found ghc_filesystem ${ghc_filesystem_VERSION} in \"${ghc_filesystem_DIR}\".")
    else()
        message(STATUS "tracejj: Use built-in filesystem, not found ghc_filesystem in CMAKE_PREFIX_PATH(\"${CMAKE_PREFIX_PATH}\").")
        add_subdirectory(third/filesystem)
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE ghc_filesystem)
endif()
if (PRODUCT_PUBLISHER)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_PUBLISHER="${PRODUCT_PUBLISHER}")
endif()

find_package(fmt 9.0 QUIET)
if (fmt_FOUND)
    message(STATUS "tracejj: Found libfmt ${fmt_VERSION} in \"${fmt_DIR}\".")
else()
    message(STATUS "tracejj: Use built-in libfmt, not found libfmt in CMAKE_PREFIX_PATH(\"${CMAKE_PREFIX_PATH}\").")
    add_subdirectory(third/fmt)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)

if (BUILD_TESTING AND TRACEJJ_MASTER_PROJECT)
    add_subdirectory(test)
endif()

if (TRACEJJ_MASTER_PROJECT)
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
