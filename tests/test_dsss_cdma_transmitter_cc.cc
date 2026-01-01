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

#include <gnuradio/qradiolink/dsss_cdma_transmitter_cc.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <iostream>
#include <vector>
#include <complex>

int main(int argc, char** argv)
{
    std::cout << "Testing DSSS-CDMA Transmitter" << std::endl;
    std::cout << "=============================" << std::endl;
    
    try {
        std::cout << "Testing DSSS-CDMA Transmitter creation... ";
        
        // Generate spreading codes for 2 users
        std::vector<int> code1(128, 1);
        std::vector<int> code2(128, -1);
        std::vector<std::vector<int>> spreading_codes = {code1, code2};
        
        auto transmitter = gr::qradiolink::dsss_cdma_transmitter_cc::make(
            spreading_codes, 128, 2, true);
        if (transmitter == nullptr) {
            std::cout << "FAILED: nullptr returned" << std::endl;
            return 1;
        }
        std::cout << "PASSED" << std::endl;
        
        std::cout << "Testing DSSS-CDMA Transmitter flowgraph (single user)... ";
        // Test with single user first to avoid multi-user synchronization issues
        std::vector<int> code_single(128, 1);
        std::vector<std::vector<int>> spreading_codes_single = {code_single};
        auto transmitter_single = gr::qradiolink::dsss_cdma_transmitter_cc::make(
            spreading_codes_single, 128, 1, true);
        
        auto tb1 = gr::make_top_block("test1");
        std::vector<gr_complex> test_data_single(10, gr_complex(0.5f, 0.3f));
        auto source_single = gr::blocks::vector_source<gr_complex>::make(test_data_single, false);
        auto head_single = gr::blocks::head::make(sizeof(gr_complex), 1280); // 10 symbols * 128
        auto sink_single = gr::blocks::null_sink::make(sizeof(gr_complex));
        
        tb1->connect(source_single, 0, transmitter_single, 0);
        tb1->connect(transmitter_single, 0, head_single, 0);
        tb1->connect(head_single, 0, sink_single, 0);
        
        tb1->start();
        tb1->wait();
        tb1->stop();
        tb1->wait();
        std::cout << "PASSED" << std::endl;
        
        std::cout << "Testing DSSS-CDMA Transmitter flowgraph (multi-user)... ";
        auto tb = gr::make_top_block("test");
        
        // Use exactly same amount of data for both users (5 symbols each)
        // With spreading factor 128, this will produce 5*128 = 640 output chips
        std::vector<gr_complex> test_data1(5, gr_complex(0.5f, 0.3f));
        std::vector<gr_complex> test_data2(5, gr_complex(0.3f, 0.5f));
        auto source1 = gr::blocks::vector_source<gr_complex>::make(test_data1, false);
        auto source2 = gr::blocks::vector_source<gr_complex>::make(test_data2, false);
        // Limit output to expected amount (5 symbols * 128 chips = 640)
        auto head_out = gr::blocks::head::make(sizeof(gr_complex), 640);
        auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));
        
        tb->connect(source1, 0, transmitter, 0);
        tb->connect(source2, 0, transmitter, 1);
        tb->connect(transmitter, 0, head_out, 0);
        tb->connect(head_out, 0, sink, 0);
        
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

