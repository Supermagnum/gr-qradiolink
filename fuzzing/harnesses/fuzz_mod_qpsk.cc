/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * libFuzzer harness for mod_qpsk
 */

#include <cstdint>
#include <cstddef>
#include <gnuradio/qradiolink/mod_qpsk.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0 || size > 1024) {
        return 0;
    }

    try {
        auto tb = gr::make_top_block("fuzz");
        auto mod = gr::qradiolink::mod_qpsk::make(125, 250000, 1700, 8000);
        auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));
        
        std::vector<unsigned char> input_data(data, data + size);
        auto source = gr::blocks::vector_source<unsigned char>::make(input_data, false);
        auto head = gr::blocks::head::make(sizeof(unsigned char), size);
        
        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, mod, 0);
        tb->connect(mod, 0, sink, 0);
        
        tb->start();
        tb->wait();
    } catch (...) {
    }
    
    return 0;
}

