/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/qradiolink/mod_8fsk.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <iostream>
#include <vector>
#include <cassert>

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    std::cout << "Testing mod_8fsk..." << std::endl;

    std::vector<unsigned char> test_data = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};

    auto src = gr::blocks::vector_source<unsigned char>::make(test_data, true);
    auto mod = gr::qradiolink::mod_8fsk::make(125, 250000, 1700, 8000, true);
    auto head = gr::blocks::head::make(sizeof(gr_complex), 1000);
    auto sink = gr::blocks::vector_sink<gr_complex>::make();

    auto tb = gr::make_top_block("test");
    tb->connect(src, 0, mod, 0);
    tb->connect(mod, 0, head, 0);
    tb->connect(head, 0, sink, 0);

    tb->start();
    tb->wait();

    std::vector<gr_complex> out = sink->data();
    assert(out.size() == 1000 && "mod_8fsk must produce 1000 complex samples (head limit)");

    std::cout << "mod_8fsk test passed!" << std::endl;
    return 0;
}

