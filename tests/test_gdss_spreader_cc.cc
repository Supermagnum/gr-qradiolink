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
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <iostream>
#include <vector>
#include <complex>

int main(int argc, char** argv)
{
    std::cout << "Testing GDSS Spreader" << std::endl;
    std::cout << "====================" << std::endl;
    
    try {
        std::cout << "Testing GDSS Spreader creation... ";
        auto spreader = gr::qradiolink::gdss_spreader_cc::make(127, 42, 1.0f, 12345);
        if (spreader == nullptr) {
            std::cout << "FAILED: nullptr returned" << std::endl;
            return 1;
        }
        std::cout << "PASSED" << std::endl;
        
        std::cout << "Testing GDSS Spreader flowgraph... ";
        auto tb = gr::make_top_block("test");
        
        std::vector<gr_complex> test_data(100, gr_complex(0.5f, 0.3f));
        auto source = gr::blocks::vector_source<gr_complex>::make(test_data);
        auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));
        
        tb->connect(source, 0, spreader, 0);
        tb->connect(spreader, 0, sink, 0);
        
        tb->start();
        tb->wait();
        tb->stop();
        tb->wait();
        
        std::cout << "PASSED" << std::endl;
        
        std::cout << "All tests PASSED" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        return 1;
    }
}

