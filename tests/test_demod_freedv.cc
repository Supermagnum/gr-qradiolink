/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/demod_freedv.h>
#include <gnuradio/vocoder/freedv_api.h>
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

BOOST_AUTO_TEST_CASE(test_demod_freedv_instantiation)
{
    auto demod = demod_freedv::make(
        125, 8000, 1700, 2000, 200,
        gr::vocoder::freedv_api::MODE_1600, 0);
    BOOST_REQUIRE(demod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_demod_freedv_flowgraph)
{
    std::vector<gr_complex> iq(1000, gr_complex(0.5f, 0.0f));
    auto tb = gr::make_top_block("test");
    auto demod = demod_freedv::make(
        125, 8000, 1700, 2000, 200,
        gr::vocoder::freedv_api::MODE_1600, 0);
    auto source = gr::blocks::vector_source<gr_complex>::make(iq, true);
    auto head = gr::blocks::head::make(sizeof(gr_complex), 500);
    auto sink0 = gr::blocks::vector_sink<gr_complex>::make();
    auto sink1 = gr::blocks::null_sink::make(sizeof(float));

    tb->connect(source, 0, demod, 0);
    tb->connect(demod, 0, head, 0);
    tb->connect(head, 0, sink0, 0);
    tb->connect(demod, 1, sink1, 0);
    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();

    BOOST_REQUIRE(sink0->data().size() > 0);
}

} // namespace qradiolink
} // namespace gr

