/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * libFuzzer harness for gdss_despreader_cc
 * Tests: gdss_despreader_cc_impl processing of complex chip-rate input
 */

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <gnuradio/qradiolink/gdss_despreader_cc.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <vector>

namespace {

std::vector<float> make_deterministic_sequence(int num_chips) {
    std::vector<float> seq(2 * num_chips);
    for (int i = 0; i < num_chips; ++i) {
        float u = 0.5f * std::sin(0.1f * (float)i) + 0.3f * std::cos(0.07f * (float)i);
        float v = 0.4f * std::cos(0.13f * (float)i) + 0.2f * std::sin(0.11f * (float)i);
        seq[2 * i] = std::abs(u);
        seq[2 * i + 1] = std::abs(v);
    }
    return seq;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 128 || size > 2048) {
        return 0;
    }

    try {
        int num_chips = 127;
        int chips_per_symbol = 42;
        std::vector<float> spreading_sequence = make_deterministic_sequence(num_chips);

        auto despreader = gr::qradiolink::gdss_despreader_cc::make(
            spreading_sequence, chips_per_symbol, 0.7f, 2);

        auto tb = gr::make_top_block("fuzz");
        auto sink_symbols = gr::blocks::null_sink::make(sizeof(gr_complex));
        auto sink_lock = gr::blocks::null_sink::make(sizeof(float));
        auto sink_snr = gr::blocks::null_sink::make(sizeof(float));

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
        tb->connect(head, 0, despreader, 0);
        tb->connect(despreader, 0, sink_symbols, 0);
        tb->connect(despreader, 1, sink_lock, 0);
        tb->connect(despreader, 2, sink_snr, 0);

        tb->start();
        tb->wait();
    } catch (...) {
        throw;  // Report exception-based crashes to libFuzzer
    }

    return 0;
}
