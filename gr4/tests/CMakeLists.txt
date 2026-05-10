# SPDX-License-Identifier: GPL-3.0-or-later
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

function(add_gr4_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name}
        PRIVATE
            gnuradio-qradiolink-gr4
            Boost::unit_test_framework
    )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_gr4_test(qa_mod_2fsk)
add_gr4_test(qa_demod_2fsk)
add_gr4_test(qa_gr4_smoke)
