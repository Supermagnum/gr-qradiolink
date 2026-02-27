/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/qradiolink/gdss_spreader_cc.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <iostream>
#include <vector>
#include <complex>
#include <cassert>

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    std::cout << "Testing GDSS Spreader" << std::endl;
    std::cout << "====================" << std::endl;

    try {
        std::cout << "Testing GDSS Spreader creation... ";
        const int chips_per_symbol = 42;
        auto spreader = gr::qradiolink::gdss_spreader_cc::make(127, chips_per_symbol, 1.0f, 12345);
        if (spreader == nullptr) {
            std::cout << "FAILED: nullptr returned" << std::endl;
            return 1;
        }
        std::cout << "PASSED" << std::endl;

        std::cout << "Testing GDSS Spreader flowgraph... ";
        auto tb = gr::make_top_block("test");

        const int num_symbols = 100;
        std::vector<gr_complex> test_data(num_symbols, gr_complex(0.5f, 0.3f));
        auto source = gr::blocks::vector_source<gr_complex>::make(test_data);
        auto sink = gr::blocks::vector_sink<gr_complex>::make();

        tb->connect(source, 0, spreader, 0);
        tb->connect(spreader, 0, sink, 0);

        tb->start();
        tb->wait();
        tb->stop();
        tb->wait();

        std::vector<gr_complex> out = sink->data();
        assert(out.size() == static_cast<size_t>(num_symbols * chips_per_symbol)
               && "gdss_spreader: 100 symbols * 42 chips/symbol = 4200 samples");

        std::cout << "PASSED" << std::endl;

        std::cout << "All tests PASSED" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        return 1;
    }
}

