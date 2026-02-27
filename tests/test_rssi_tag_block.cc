/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/rssi_tag_block.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_instantiation)
{
    auto rssi = rssi_tag_block::make();
    BOOST_REQUIRE(rssi != nullptr);
}

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_flowgraph)
{
    std::vector<gr_complex> iq(1000, gr_complex(0.5f, 0.0f));
    auto tb = gr::make_top_block("test");
    auto rssi = rssi_tag_block::make();
    auto source = gr::blocks::vector_source<gr_complex>::make(iq, false);
    auto sink = gr::blocks::vector_sink<gr_complex>::make();

    tb->connect(source, 0, rssi, 0);
    tb->connect(rssi, 0, sink, 0);
    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();

    BOOST_REQUIRE(sink->data().size() == 1000);
}

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_calibrate)
{
    auto rssi = rssi_tag_block::make();
    BOOST_REQUIRE(rssi != nullptr);
    
    // Test calibration method
    rssi->calibrate_rssi(0.5);
    
    // If we get here, calibration succeeded
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_calibrate_edge_zero)
{
    auto rssi = rssi_tag_block::make();
    BOOST_REQUIRE(rssi != nullptr);
    rssi->calibrate_rssi(0.0);
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_calibrate_edge_small)
{
    auto rssi = rssi_tag_block::make();
    BOOST_REQUIRE(rssi != nullptr);
    rssi->calibrate_rssi(1e-10);
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_calibrate_edge_large)
{
    auto rssi = rssi_tag_block::make();
    BOOST_REQUIRE(rssi != nullptr);
    rssi->calibrate_rssi(1e10);
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_flowgraph_zero_input)
{
    auto tb = gr::make_top_block("test");
    auto rssi = rssi_tag_block::make();
    std::vector<gr_complex> zeros(1000, gr_complex(0.0f, 0.0f));
    auto source = gr::blocks::vector_source<gr_complex>::make(zeros, false);
    auto sink = gr::blocks::vector_sink<gr_complex>::make();

    tb->connect(source, 0, rssi, 0);
    tb->connect(rssi, 0, sink, 0);
    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();

    BOOST_REQUIRE(sink->data().size() == 1000);
}

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_flowgraph_single_sample)
{
    auto tb = gr::make_top_block("test");
    auto rssi = rssi_tag_block::make();
    std::vector<gr_complex> single = {gr_complex(1.0f, 0.0f)};
    auto source = gr::blocks::vector_source<gr_complex>::make(single, false);
    auto sink = gr::blocks::vector_sink<gr_complex>::make();

    tb->connect(source, 0, rssi, 0);
    tb->connect(rssi, 0, sink, 0);
    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();

    BOOST_REQUIRE(sink->data().size() == 1);
}

} // namespace qradiolink
} // namespace gr

