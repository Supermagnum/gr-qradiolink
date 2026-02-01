/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/interleaver_bb.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <vector>
#include <numeric>
#include <stdexcept>

namespace gr {
namespace qradiolink {

static void run_roundtrip(const std::vector<unsigned char>& test_data,
                          unsigned int n_rows,
                          unsigned int n_cols)
{
    auto tb = gr::make_top_block("test");
    auto src = gr::blocks::vector_source<unsigned char>::make(test_data, false);
    auto inter = interleaver_bb::make(n_rows, n_cols, true);
    auto deinter = interleaver_bb::make(n_rows, n_cols, false);
    auto sink = gr::blocks::vector_sink<unsigned char>::make();

    tb->connect(src, 0, inter, 0);
    tb->connect(inter, 0, deinter, 0);
    tb->connect(deinter, 0, sink, 0);

    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();

    auto out = sink->data();
    BOOST_REQUIRE_EQUAL(test_data.size(), out.size());
    for (size_t i = 0; i < test_data.size(); ++i) {
        BOOST_REQUIRE_EQUAL(test_data[i], out[i]);
    }
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_instantiation)
{
    auto inter = interleaver_bb::make(8, 46, true);
    BOOST_REQUIRE(inter != nullptr);

    auto deinter = interleaver_bb::make(8, 46, false);
    BOOST_REQUIRE(deinter != nullptr);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_invalid_args)
{
    BOOST_REQUIRE_THROW(interleaver_bb::make(0, 46, true), std::invalid_argument);
    BOOST_REQUIRE_THROW(interleaver_bb::make(8, 0, true), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_flowgraph)
{
    const unsigned int n_rows = 8;
    const unsigned int n_cols = 46;
    const unsigned int block_size = n_rows * n_cols;

    std::vector<unsigned char> test_data(block_size * 2);
    std::iota(test_data.begin(), test_data.end(), 0);

    run_roundtrip(test_data, n_rows, n_cols);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_roundtrip_single_block)
{
    const unsigned int n_rows = 8;
    const unsigned int n_cols = 46;
    const unsigned int block_size = n_rows * n_cols;

    std::vector<unsigned char> test_data(block_size);
    for (unsigned int i = 0; i < block_size; ++i) {
        test_data[i] = static_cast<unsigned char>(i & 0xFF);
    }

    run_roundtrip(test_data, n_rows, n_cols);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_edge_minimum_1x1)
{
    std::vector<unsigned char> test_data = {0xAB};
    run_roundtrip(test_data, 1, 1);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_edge_single_row)
{
    const unsigned int n_cols = 46;
    std::vector<unsigned char> test_data(n_cols);
    std::iota(test_data.begin(), test_data.end(), 0);
    run_roundtrip(test_data, 1, n_cols);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_edge_single_column)
{
    const unsigned int n_rows = 8;
    std::vector<unsigned char> test_data(n_rows);
    for (unsigned int i = 0; i < n_rows; ++i) {
        test_data[i] = static_cast<unsigned char>(i * 17);
    }
    run_roundtrip(test_data, n_rows, 1);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_edge_asymmetric_2x3)
{
    std::vector<unsigned char> test_data = {0, 1, 2, 3, 4, 5};
    run_roundtrip(test_data, 2, 3);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_edge_asymmetric_3x2)
{
    std::vector<unsigned char> test_data = {0, 1, 2, 3, 4, 5};
    run_roundtrip(test_data, 3, 2);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_edge_all_zeros)
{
    const unsigned int block_size = 368;
    std::vector<unsigned char> test_data(block_size, 0);
    run_roundtrip(test_data, 8, 46);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_edge_all_0xff)
{
    const unsigned int block_size = 368;
    std::vector<unsigned char> test_data(block_size, 0xFF);
    run_roundtrip(test_data, 8, 46);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_edge_many_blocks)
{
    const unsigned int n_rows = 8;
    const unsigned int n_cols = 46;
    const unsigned int n_blocks = 100;
    const unsigned int block_size = n_rows * n_cols;
    std::vector<unsigned char> test_data(n_blocks * block_size);
    for (size_t i = 0; i < test_data.size(); ++i) {
        test_data[i] = static_cast<unsigned char>(i & 0xFF);
    }
    run_roundtrip(test_data, n_rows, n_cols);
}

BOOST_AUTO_TEST_CASE(test_interleaver_bb_edge_interleave_only_permutes)
{
    const unsigned int n_rows = 2;
    const unsigned int n_cols = 3;
    const unsigned int block_size = 6;
    std::vector<unsigned char> in_data = {0, 1, 2, 3, 4, 5};

    auto tb = gr::make_top_block("test");
    auto src = gr::blocks::vector_source<unsigned char>::make(in_data, false);
    auto inter = interleaver_bb::make(n_rows, n_cols, true);
    auto sink = gr::blocks::vector_sink<unsigned char>::make();

    tb->connect(src, 0, inter, 0);
    tb->connect(inter, 0, sink, 0);
    tb->start();
    tb->wait();
    tb->stop();
    tb->wait();

    auto out = sink->data();
    BOOST_REQUIRE_EQUAL(block_size, out.size());
    BOOST_REQUIRE_EQUAL(out[0], 0);
    BOOST_REQUIRE_EQUAL(out[1], 3);
    BOOST_REQUIRE_EQUAL(out[2], 1);
    BOOST_REQUIRE_EQUAL(out[3], 4);
    BOOST_REQUIRE_EQUAL(out[4], 2);
    BOOST_REQUIRE_EQUAL(out[5], 5);
}

} // namespace qradiolink
} // namespace gr
