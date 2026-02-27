/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * libFuzzer harness for gdss_spreader_cc
 * Tests: gdss_spreader_cc_impl processing of complex symbol input
 */

#include <cstdint>
#include <cstddef>
#include <gnuradio/qradiolink/gdss_spreader_cc.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 64 || size > 2048) {
        return 0;
    }

    try {
        static const int seq_lens[] = {31, 63, 127};
        static const int chips_vals[] = {7, 21, 42};
        int sequence_length = seq_lens[data[0] % 3];
        int chips_per_symbol = chips_vals[data[1] % 3];
        unsigned int seed = size > 4 ? (data[2] | (data[3] << 8) | (data[4] << 16)) : 12345u;
        auto tb = gr::make_top_block("fuzz");
        auto spreader =
            gr::qradiolink::gdss_spreader_cc::make(
                sequence_length, chips_per_symbol, 1.0f, seed);
        auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));

        std::vector<gr_complex> complex_data;
        complex_data.reserve(size / 2);
        for (size_t i = 0; i + 1 < size; i += 2) {
            float re = (float)((int8_t)data[i]) / 127.0f;
            float im = (float)((int8_t)data[i + 1]) / 127.0f;
            complex_data.push_back(gr_complex(re, im));
        }
        if (complex_data.empty()) {
            return 0;
        }

        auto source =
            gr::blocks::vector_source<gr_complex>::make(complex_data, false);
        auto head =
            gr::blocks::head::make(sizeof(gr_complex), complex_data.size());

        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, spreader, 0);
        tb->connect(spreader, 0, sink, 0);

        tb->start();
        tb->wait();
    } catch (...) {
        throw;  // Report exception-based crashes to libFuzzer
    }

    return 0;
}
