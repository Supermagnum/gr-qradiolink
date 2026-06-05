/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/qradiolink/grident_mode_map.h>
#include <gnuradio/qradiolink/grident_zmq_protocol.h>
#include <iostream>
#include <string>

int main()
{
    using namespace gr::qradiolink::grident;

    const std::string sample = R"({"mode_id":20,"digital":false,"encrypted":false,"metadata_present":false})";
    const auto parsed = parse_preamble_json(sample);
    if (!parsed || parsed->mode_id != 20 || parsed->digital) {
        return 1;
    }

    const auto sel = selection_from_json(R"({"mode_id":104,"digital":true,"encrypted":false,"metadata_present":false})");
    if (!sel || sel->demod_route != modem_route::demod_ysf
        || std::string(route_block_name(sel->demod_route)) != "ysf_decoder") {
        return 1;
    }

    std::cout << "test_grident_zmq: OK" << std::endl;
    return 0;
}
