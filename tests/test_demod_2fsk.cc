/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/mod_2fsk.h>
#include <gnuradio/qradiolink/demod_2fsk.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <vector>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_demod_2fsk_instantiation)
{
    auto demod = demod_2fsk::make(125, 250000, 1700, 8000, false);
    BOOST_REQUIRE(demod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_demod_2fsk_flowgraph)
{
    std::vector<gr_complex> iq(1000, gr_complex(0.5f, 0.0f));
    auto tb = gr::make_top_block("test");
    auto demod = demod_2fsk::make(125, 250000, 1700, 8000, false);
    auto source = gr::blocks::vector_source<gr_complex>::make(iq, true);
    auto head = gr::blocks::head::make(sizeof(gr_complex), 500);
    auto sink0 = gr::blocks::vector_sink<gr_complex>::make();
    auto sink1 = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto sink2 = gr::blocks::null_sink::make(sizeof(char));
    auto sink3 = gr::blocks::null_sink::make(sizeof(char));

    tb->connect(source, 0, demod, 0);
    tb->connect(demod, 0, head, 0);
    tb->connect(head, 0, sink0, 0);
    tb->connect(demod, 1, sink1, 0);
    tb->connect(demod, 2, sink2, 0);
    tb->connect(demod, 3, sink3, 0);
    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();

    BOOST_REQUIRE(sink0->data().size() > 0);
}

BOOST_AUTO_TEST_CASE(test_demod_2fsk_fm_mode)
{
    auto demod = demod_2fsk::make(125, 250000, 1700, 8000, true);
    BOOST_REQUIRE(demod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_demod_2fsk_edge_zero_input)
{
    std::vector<gr_complex> zeros(1000, gr_complex(0.0f, 0.0f));
    auto tb = gr::make_top_block("test");
    auto demod = demod_2fsk::make(125, 250000, 1700, 8000, false);
    auto source = gr::blocks::vector_source<gr_complex>::make(zeros, false);
    auto head = gr::blocks::head::make(sizeof(gr_complex), 500);
    auto sink0 = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto sink1 = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto sink2 = gr::blocks::vector_sink<unsigned char>::make();
    auto sink3 = gr::blocks::null_sink::make(sizeof(char));

    tb->connect(source, 0, demod, 0);
    tb->connect(demod, 0, head, 0);
    tb->connect(head, 0, sink0, 0);
    tb->connect(demod, 1, sink1, 0);
    tb->connect(demod, 2, sink2, 0);
    tb->connect(demod, 3, sink3, 0);
    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();

    std::vector<unsigned char> decoded = sink2->data();
    BOOST_REQUIRE(decoded.size() <= 500 && "demod with 500 complex inputs produces at most 500 bytes");
}

BOOST_AUTO_TEST_CASE(test_mod_2fsk_demod_2fsk_roundtrip)
{
    const int sps = 125;
    const int samp_rate = 250000;
    const int carrier_freq = 1700;
    const int filter_width = 8000;
    const bool fm = false;

    std::vector<unsigned char> input_bytes = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    };
    for (int i = 0; i < 48; ++i)
        input_bytes.push_back(static_cast<unsigned char>(i & 0xFF));

    auto tb = gr::make_top_block("roundtrip");
    auto src = gr::blocks::vector_source<unsigned char>::make(input_bytes, false);
    auto mod = mod_2fsk::make(sps, samp_rate, carrier_freq, filter_width, fm);
    auto demod = demod_2fsk::make(sps, samp_rate, carrier_freq, filter_width, fm);
    auto sink_decoded = gr::blocks::vector_sink<unsigned char>::make();
    auto sink0 = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto sink1 = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto sink3 = gr::blocks::null_sink::make(sizeof(char));

    tb->connect(src, 0, mod, 0);
    tb->connect(mod, 0, demod, 0);
    tb->connect(demod, 0, sink0, 0);
    tb->connect(demod, 1, sink1, 0);
    tb->connect(demod, 2, sink_decoded, 0);
    tb->connect(demod, 3, sink3, 0);

    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();

    std::vector<unsigned char> output = sink_decoded->data();
    BOOST_REQUIRE_MESSAGE(
        output.size() > 0,
        "Roundtrip must produce decoded output (got " << output.size() << " bytes)");
    BOOST_REQUIRE_MESSAGE(
        output.size() >= static_cast<size_t>(input_bytes.size()),
        "Roundtrip produced " << output.size() << " bytes, need at least "
                             << input_bytes.size());
}

} // namespace qradiolink
} // namespace gr
