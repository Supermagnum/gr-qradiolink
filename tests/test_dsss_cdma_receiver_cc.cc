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

#include <gnuradio/qradiolink/dsss_cdma_receiver_cc.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <iostream>
#include <vector>
#include <complex>
#include <cassert>

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    std::cout << "Testing DSSS-CDMA Receiver" << std::endl;
    std::cout << "===========================" << std::endl;
    
    try {
        std::cout << "Testing DSSS-CDMA Receiver creation... ";
        
        // Use same spreading code as transmitter
        std::vector<int> spreading_code(128, 1);
        
        auto receiver = gr::qradiolink::dsss_cdma_receiver_cc::make(
            spreading_code, 128, 0.7f, 2);
        if (receiver == nullptr) {
            std::cout << "FAILED: nullptr returned" << std::endl;
            return 1;
        }
        std::cout << "PASSED" << std::endl;
        
        std::cout << "Testing DSSS-CDMA Receiver flowgraph... ";
        auto tb = gr::make_top_block("test");
        
        std::vector<gr_complex> test_data(6400, gr_complex(0.1f, 0.1f));
        auto source = gr::blocks::vector_source<gr_complex>::make(test_data);
        auto head = gr::blocks::head::make(sizeof(gr_complex), 1000);
        auto sink1 = gr::blocks::vector_sink<gr_complex>::make();
        auto sink2 = gr::blocks::vector_sink<float>::make();
        auto sink3 = gr::blocks::vector_sink<float>::make();
        auto sink4 = gr::blocks::vector_sink<float>::make();

        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, receiver, 0);
        tb->connect(receiver, 0, sink1, 0);
        tb->connect(receiver, 1, sink2, 0);
        tb->connect(receiver, 2, sink3, 0);
        tb->connect(receiver, 3, sink4, 0);

        tb->start();
        tb->wait();
        tb->stop();
        tb->wait();

        assert(sink1->data().size() > 0 && "dsss_cdma_receiver must produce complex output");
        assert(sink2->data().size() > 0 && "dsss_cdma_receiver must produce output on port 1");
        assert(sink3->data().size() > 0 && "dsss_cdma_receiver must produce output on port 2");
        assert(sink4->data().size() > 0 && "dsss_cdma_receiver must produce output on port 3");
        std::cout << "PASSED" << std::endl;
        
        std::cout << "All tests PASSED" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        return 1;
    }
}

