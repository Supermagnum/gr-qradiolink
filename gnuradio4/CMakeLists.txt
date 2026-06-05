# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.22)
project(
  gr-qradiolink4
  VERSION 0.1.0
  LANGUAGES CXX
  DESCRIPTION "GNU Radio 4.0 header-only qradiolink-style blocks")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(GR_QRAD_LINK4_BUILD_TESTS "Build Boost.UT tests for gr-qradiolink4" ON)

if(TARGET gnuradio4::gnuradio-core)
  set(_GR4_QRAD_CORE_TARGET gnuradio4::gnuradio-core)
else()
  find_package(gnuradio4 REQUIRED CONFIG)
  set(_GR4_QRAD_CORE_TARGET gnuradio4::gnuradio-core)
endif()

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
  pkg_check_modules(CODEC2 QUIET IMPORTED_TARGET codec2)
  pkg_check_modules(ZMQ QUIET IMPORTED_TARGET libzmq)
endif()

add_library(gnuradio4-gr-qradiolink INTERFACE)
add_library(gnuradio4::gr-qradiolink ALIAS gnuradio4-gr-qradiolink)

target_include_directories(
  gnuradio4-gr-qradiolink
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

target_link_libraries(gnuradio4-gr-qradiolink INTERFACE ${_GR4_QRAD_CORE_TARGET})

if(TARGET gnuradio4::gnuradio-blocklib-core)
  target_link_libraries(gnuradio4-gr-qradiolink INTERFACE gnuradio4::gnuradio-blocklib-core)
endif()

if(TARGET PkgConfig::CODEC2)
  target_compile_definitions(gnuradio4-gr-qradiolink INTERFACE GR_QRAD_GR4_HAVE_CODEC2=1)
  target_link_libraries(gnuradio4-gr-qradiolink INTERFACE PkgConfig::CODEC2)
endif()

if(TARGET PkgConfig::ZMQ)
  target_compile_definitions(gnuradio4-gr-qradiolink INTERFACE GR_QRAD_GR4_HAVE_ZMQ=1)
  target_link_libraries(gnuradio4-gr-qradiolink INTERFACE PkgConfig::ZMQ)
endif()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(
  TARGETS gnuradio4-gr-qradiolink
  EXPORT gr-qradiolink4-targets
  INCLUDES
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(
  EXPORT gr-qradiolink4-targets
  FILE gr-qradiolink4-targets.cmake
  NAMESPACE gnuradio4::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gr-qradiolink4")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gr-qradiolink4-config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/gr-qradiolink4-config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gr-qradiolink4")

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/gr-qradiolink4-config-version.cmake"
                                 VERSION "${PROJECT_VERSION}" COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gr-qradiolink4-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/gr-qradiolink4-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gr-qradiolink4")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/gnuradio4-gr-qradiolink.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/gnuradio4-gr-qradiolink.pc"
               @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gnuradio4-gr-qradiolink.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

if(NOT DEFINED GR_QRADIOLINK_GR4_ENABLE_TESTING)
  set(GR_QRADIOLINK_GR4_ENABLE_TESTING ON)
endif()

if(GR_QRAD_LINK4_BUILD_TESTS AND GR_QRADIOLINK_GR4_ENABLE_TESTING)
  include(FetchContent)
  set(BOOST_UT_DISABLE_MODULE ON CACHE BOOL "Disable UT Module Support" FORCE)
  FetchContent_Declare(
    ut
    GIT_REPOSITORY https://github.com/boost-ext/ut.git
    GIT_TAG 53e17f25119598c6458d30351b260193096ba67e
    EXCLUDE_FROM_ALL)
  FetchContent_MakeAvailable(ut)
  enable_testing()
  add_subdirectory(test)
endif()
