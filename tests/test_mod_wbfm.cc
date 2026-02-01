/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/mod_wbfm.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <boost/test/unit_test.hpp>
#include <vector>

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_mod_wbfm_instantiation)
{
    auto mod = mod_wbfm::make(125, 250000, 1700, 8000);
    BOOST_REQUIRE(mod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_mod_wbfm_flowgraph)
{
    auto tb = gr::make_top_block("test");
    auto mod = mod_wbfm::make(125, 250000, 1700, 8000);
    auto source = gr::blocks::null_source::make(sizeof(float));
    auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));

    tb->connect(source, 0, mod, 0);
    tb->connect(mod, 0, sink, 0);
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_mod_wbfm_setters)
{
    auto mod = mod_wbfm::make(125, 250000, 1700, 8000);
    mod->set_filter_width(10000);
    mod->set_bb_gain(0.5f);
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_mod_wbfm_edge_zero_input)
{
    std::vector<float> zeros(1000, 0.0f);
    auto tb = gr::make_top_block("test");
    auto mod = mod_wbfm::make(125, 250000, 1700, 8000);
    auto source = gr::blocks::vector_source<float>::make(zeros, false);
    auto head = gr::blocks::head::make(sizeof(gr_complex), 500);
    auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));

    tb->connect(source, 0, mod, 0);
    tb->connect(mod, 0, head, 0);
    tb->connect(head, 0, sink, 0);
    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_mod_wbfm_edge_extreme_amplitude)
{
    std::vector<float> extreme(1000, 1e6f);
    auto tb = gr::make_top_block("test");
    auto mod = mod_wbfm::make(125, 250000, 1700, 8000);
    auto source = gr::blocks::vector_source<float>::make(extreme, false);
    auto head = gr::blocks::head::make(sizeof(gr_complex), 500);
    auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));

    tb->connect(source, 0, mod, 0);
    tb->connect(mod, 0, head, 0);
    tb->connect(head, 0, sink, 0);
    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();
    BOOST_REQUIRE(true);
}

} // namespace qradiolink
} // namespace gr

