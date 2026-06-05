/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "grident_preamble_sub_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/pmt_fmt.h>
#include <zmq.h>
#include <cstring>

namespace gr {
namespace qradiolink {

static const pmt::pmt_t PORT_PREAMBLE_OUT = pmt::mp("preamble_out");

grident_preamble_sub::sptr grident_preamble_sub::make(const std::string& endpoint,
    const std::string& topic_filter,
    bool               bind_socket)
{
    return gnuradio::get_initial_sptr(new grident_preamble_sub_impl(endpoint, topic_filter, bind_socket));
}

grident_preamble_sub_impl::grident_preamble_sub_impl(const std::string& endpoint,
    const std::string& topic_filter,
    bool               bind_socket)
    : grident_preamble_sub("grident_preamble_sub", gr::io_signature::make(0, 0, 0), gr::io_signature::make(0, 0, 0))
    , d_endpoint(endpoint)
    , d_topic_filter(topic_filter.empty() ? std::string(grident::k_preamble_topic) : topic_filter)
    , d_bind_socket(bind_socket)
{
    message_port_register_out(PORT_PREAMBLE_OUT);

    d_zmq_ctx = zmq_ctx_new();
    if (d_zmq_ctx == nullptr) {
        return;
    }
    d_zmq_sub = zmq_socket(d_zmq_ctx, ZMQ_SUB);
    if (d_zmq_sub == nullptr) {
        close_zmq();
        return;
    }
    zmq_setsockopt(d_zmq_sub, ZMQ_SUBSCRIBE, d_topic_filter.data(), d_topic_filter.size());
    const int ep_rc =
        d_bind_socket ? zmq_bind(d_zmq_sub, d_endpoint.c_str()) : zmq_connect(d_zmq_sub, d_endpoint.c_str());
    if (ep_rc != 0) {
        close_zmq();
        return;
    }
    d_stop.store(false);
    d_thread = std::thread([this]() { io_loop(); });
}

grident_preamble_sub_impl::~grident_preamble_sub_impl() { close_zmq(); }

void grident_preamble_sub_impl::close_zmq()
{
    d_stop.store(true);
    if (d_thread.joinable()) {
        d_thread.join();
    }
    if (d_zmq_sub != nullptr) {
        zmq_close(d_zmq_sub);
        d_zmq_sub = nullptr;
    }
    if (d_zmq_ctx != nullptr) {
        zmq_ctx_term(d_zmq_ctx);
        d_zmq_ctx = nullptr;
    }
}

void grident_preamble_sub_impl::io_loop()
{
    zmq_pollitem_t item{};
    item.socket = d_zmq_sub;
    item.events = ZMQ_POLLIN;

    while (!d_stop.load()) {
        if (zmq_poll(&item, 1, 200) <= 0) {
            continue;
        }
        zmq_msg_t topic_msg;
        zmq_msg_t body_msg;
        zmq_msg_init(&topic_msg);
        zmq_msg_init(&body_msg);
        if (zmq_msg_recv(&topic_msg, d_zmq_sub, 0) < 0) {
            zmq_msg_close(&topic_msg);
            zmq_msg_close(&body_msg);
            continue;
        }
        if (zmq_msg_recv(&body_msg, d_zmq_sub, 0) < 0) {
            zmq_msg_close(&topic_msg);
            zmq_msg_close(&body_msg);
            continue;
        }

        const char* topic_ptr = static_cast<const char*>(zmq_msg_data(&topic_msg));
        const char* body_ptr  = static_cast<const char*>(zmq_msg_data(&body_msg));
        const std::string topic(topic_ptr, topic_ptr + zmq_msg_size(&topic_msg));
        const std::string json(body_ptr, body_ptr + zmq_msg_size(&body_msg));

        if (grident::topic_matches_preamble(topic)) {
            pmt::pmt_t d = pmt::make_dict();
            d            = pmt::dict_add(d, pmt::mp("topic"), pmt::mp(topic));
            d            = pmt::dict_add(d, pmt::mp("json"), pmt::mp(json));
            message_port_pub(PORT_PREAMBLE_OUT, d);
        }

        zmq_msg_close(&topic_msg);
        zmq_msg_close(&body_msg);
    }
}

} // namespace qradiolink
} // namespace gr
