/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * libFuzzer harness for mod_2fsk
 * Tests: mod_2fsk_impl processing of byte input data
 */

#include <cstdint>
#include <cstddef>
#include <gnuradio/qradiolink/mod_2fsk.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Limit input size to prevent excessive memory usage
    if (size == 0 || size > 1024) {
        return 0;
    }

    try {
        // Create a top block and modulator
        auto tb = gr::make_top_block("fuzz");
        
        // Standard parameters for 2FSK
        int sps = 125;
        int samp_rate = 250000;
        int carrier_freq = 1700;
        int filter_width = 8000;
        bool fm = false;
        
        auto mod = gr::qradiolink::mod_2fsk::make(sps, samp_rate, carrier_freq, filter_width, fm);
        auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));
        
        // Create vector source from fuzzer input
        std::vector<unsigned char> input_data(data, data + size);
        auto source = gr::blocks::vector_source<unsigned char>::make(input_data, false);
        
        // Use head block to limit processing to prevent infinite loops
        // Limit to input size to ensure we process all fuzzer data
        auto head = gr::blocks::head::make(sizeof(unsigned char), size);
        
        // Connect blocks
        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, mod, 0);
        tb->connect(mod, 0, sink, 0);
        
        // Run the flowgraph - it will stop when head block is done
        tb->start();
        tb->wait();
        
    } catch (...) {
        // Catch all exceptions to prevent crashes from stopping fuzzing
    }
    
    return 0;
}

