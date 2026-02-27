/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/mod_dsss.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <vector>

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_mod_dsss_instantiation)
{
    // filter_width used with if_samp_rate = 5200, so filter_width must be <= 2600
    // Use 2000 Hz
    auto mod = mod_dsss::make(25, 250000, 1700, 2000);
    BOOST_REQUIRE(mod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_mod_dsss_flowgraph)
{
    std::vector<unsigned char> data(100, 0xAA);
    auto tb = gr::make_top_block("test");
    auto mod = mod_dsss::make(25, 250000, 1700, 2000);
    auto source = gr::blocks::vector_source<unsigned char>::make(data, true);
    auto head = gr::blocks::head::make(sizeof(gr_complex), 500);
    auto sink = gr::blocks::vector_sink<gr_complex>::make();

    tb->connect(source, 0, mod, 0);
    tb->connect(mod, 0, head, 0);
    tb->connect(head, 0, sink, 0);
    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();

    BOOST_REQUIRE(sink->data().size() == 500);
}

BOOST_AUTO_TEST_CASE(test_mod_dsss_set_bb_gain)
{
    auto mod = mod_dsss::make(25, 250000, 1700, 2000);
    mod->set_bb_gain(0.5f);
    // If no exception is thrown, the call succeeded
    BOOST_REQUIRE(true);
}

} // namespace qradiolink
} // namespace gr

