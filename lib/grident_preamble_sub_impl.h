/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_QRADIOLINK_GRIDENT_PREAMBLE_SUB_IMPL_H
#define INCLUDED_QRADIOLINK_GRIDENT_PREAMBLE_SUB_IMPL_H

#include <gnuradio/qradiolink/grident_preamble_sub.h>
#include <atomic>
#include <string>
#include <thread>

namespace gr {
namespace qradiolink {

class grident_preamble_sub_impl : public grident_preamble_sub
{
private:
    std::string       d_endpoint;
    std::string       d_topic_filter;
    bool              d_bind_socket;
    void*             d_zmq_ctx{nullptr};
    void*             d_zmq_sub{nullptr};
    std::thread       d_thread;
    std::atomic<bool> d_stop{false};

    void close_zmq();
    void io_loop();

public:
    grident_preamble_sub_impl(const std::string& endpoint, const std::string& topic_filter, bool bind_socket);
    ~grident_preamble_sub_impl() override;
};

} // namespace qradiolink
} // namespace gr

#endif
