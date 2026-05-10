# SPDX-License-Identifier: GPL-3.0-or-later
# Official CMake entry point for the GR4 port (-S cmake_gr4).
cmake_minimum_required(VERSION 3.27)

get_filename_component(GR_QRL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

project(gr-qradiolink-gr4 VERSION 4.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GR4_SOURCE_DIR
    "/mnt/2e9a1e9f-2097-408c-ab9a-a01b32f11d28/github-projects/gnuradio4"
    CACHE PATH "Path to the gnuradio4 source tree")

if(NOT EXISTS "${GR4_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "GR4_SOURCE_DIR does not point to a gnuradio4 checkout: ${GR4_SOURCE_DIR}")
endif()

set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

add_subdirectory("${GR4_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/gnuradio4_build" EXCLUDE_FROM_ALL)

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(CODEC2 IMPORTED_TARGET codec2)
    pkg_check_modules(LIBM17 IMPORTED_TARGET libm17)
    pkg_check_modules(ZMQ IMPORTED_TARGET libzmq)
endif()

add_subdirectory("${GR_QRL_ROOT}/gr4" gr_qr4)

include(GNUInstallDirs)
install(DIRECTORY "${GR_QRL_ROOT}/gr4/include/gnuradio-4.0/qradiolink"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gnuradio-4.0")

option(GR_QRADIOLINK_GR4_ENABLE_TESTING "Build GR4 unit tests" ON)
if(GR_QRADIOLINK_GR4_ENABLE_TESTING)
    enable_testing()
    add_subdirectory("${GR_QRL_ROOT}/gr4/tests" gr_qr4_tests)
endif()
