# SPDX-License-Identifier: GPL-3.0-or-later
add_library(gnuradio-qradiolink-gr4 INTERFACE)
add_library(gnuradio4::qradiolink-gr4 ALIAS gnuradio-qradiolink-gr4)

target_include_directories(gnuradio-qradiolink-gr4 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(gnuradio-qradiolink-gr4 INTERFACE gnuradio4::gnuradio-core)

if(TARGET PkgConfig::CODEC2)
    target_link_libraries(gnuradio-qradiolink-gr4 INTERFACE PkgConfig::CODEC2)
    target_compile_definitions(gnuradio-qradiolink-gr4 INTERFACE GR_QRADIOLINK_HAVE_CODEC2)
endif()
if(TARGET PkgConfig::LIBM17)
    target_link_libraries(gnuradio-qradiolink-gr4 INTERFACE PkgConfig::LIBM17)
    target_compile_definitions(gnuradio-qradiolink-gr4 INTERFACE GR_QRADIOLINK_HAVE_LIBM17)
endif()
if(TARGET PkgConfig::ZMQ)
    target_link_libraries(gnuradio-qradiolink-gr4 INTERFACE PkgConfig::ZMQ)
    target_compile_definitions(gnuradio-qradiolink-gr4 INTERFACE GR_QRADIOLINK_HAVE_ZMQ)
endif()

install(TARGETS gnuradio-qradiolink-gr4)
