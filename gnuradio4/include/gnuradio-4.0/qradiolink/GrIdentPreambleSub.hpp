// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_GRIDENTPREAMBLESUB_HPP
#define GNURADIO4_QRAD_GRIDENTPREAMBLESUB_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/Message.hpp>
#include <gnuradio-4.0/Port.hpp>
#include <gnuradio-4.0/Value.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/GrIdentZmqProtocol.hpp>

#include <atomic>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <thread>

#if defined(GR_QRAD_GR4_HAVE_ZMQ)
#include <zmq.h>
#endif

namespace gnuradio4::qradiolink {

#if defined(GR_QRAD_GR4_HAVE_ZMQ)

GR_REGISTER_BLOCK(gnuradio4::qradiolink::GrIdentPreambleSub)

struct GrIdentPreambleSub : gr::Block<GrIdentPreambleSub, gr::NoTagPropagation> {
    using Description = gr::Doc<"SUBscribe to gr-ident PreambleResultZmqPub (tcp :5560) and forward JSON to msg_route_out.">;

    gr::MsgPortOut msg_route_out{};

    gr::Annotated<std::string, "endpoint", gr::Doc<"ZMQ SUB connect target (gr-ident PUB binds here).">>
        endpoint{std::string{detail::kGrIdentPreamblePubDefault}};
    gr::Annotated<std::string, "topic_filter", gr::Doc<"ZMQ SUB prefix; use grident for all grident.* topics.">>
        topic_filter{std::string{detail::kGrIdentPreambleTopic}};
    gr::Annotated<bool, "bind_socket", gr::Doc<"False: connect SUB (typical). True: bind SUB (lab only).">>
        bind_socket{false};

    GR_MAKE_REFLECTABLE(GrIdentPreambleSub, msg_route_out, endpoint, topic_filter, bind_socket);

private:
    void*                      _zmq_ctx{nullptr};
    void*                      _zmq_sub{nullptr};
    std::thread                _io_thread{};
    std::atomic<bool>          _stop{false};
    std::atomic<bool>          _thread_running{false};

    void closeZmq() noexcept
    {
        _stop.store(true, std::memory_order_release);
        if (_io_thread.joinable()) {
            _io_thread.join();
        }
        if (_zmq_sub != nullptr) {
            zmq_close(_zmq_sub);
            _zmq_sub = nullptr;
        }
        if (_zmq_ctx != nullptr) {
            zmq_ctx_term(_zmq_ctx);
            _zmq_ctx = nullptr;
        }
        _thread_running.store(false, std::memory_order_release);
    }

    void forwardFrame(std::string_view topic, std::string_view json_body)
    {
        gr::property_map body;
        body[gr::convert_string_domain(std::string_view("topic"))] = gr::pmt::Value(std::string{topic});
        body[gr::convert_string_domain(std::string_view("json"))]  = gr::pmt::Value(std::string{json_body});
        gr::Message outgoing;
        outgoing.cmd  = gr::message::Command::Notify;
        outgoing.data = std::move(body);
        auto w        = msg_route_out.streamWriter().template reserve<gr::SpanReleasePolicy::ProcessAll>(1UZ);
        w[0]          = std::move(outgoing);
        w.publish(1UZ);
    }

    void ioLoop()
    {
        _thread_running.store(true, std::memory_order_release);
        zmq_pollitem_t item{};
        item.socket  = _zmq_sub;
        item.events  = ZMQ_POLLIN;
        while (!_stop.load(std::memory_order_acquire)) {
            const int rc = zmq_poll(&item, 1, 200);
            if (rc <= 0) {
                continue;
            }
            zmq_msg_t topic_msg;
            zmq_msg_t body_msg;
            zmq_msg_init(&topic_msg);
            zmq_msg_init(&body_msg);
            if (zmq_msg_recv(&topic_msg, _zmq_sub, 0) < 0) {
                zmq_msg_close(&topic_msg);
                zmq_msg_close(&body_msg);
                continue;
            }
            if (zmq_msg_recv(&body_msg, _zmq_sub, 0) < 0) {
                zmq_msg_close(&topic_msg);
                zmq_msg_close(&body_msg);
                continue;
            }
            const std::string_view topic{static_cast<const char*>(zmq_msg_data(&topic_msg)),
                static_cast<std::size_t>(zmq_msg_size(&topic_msg))};
            const std::string_view body{static_cast<const char*>(zmq_msg_data(&body_msg)),
                static_cast<std::size_t>(zmq_msg_size(&body_msg))};
            if (detail::grIdentTopicMatchesPreamble(topic)) {
                forwardFrame(topic, body);
            }
            zmq_msg_close(&topic_msg);
            zmq_msg_close(&body_msg);
        }
    }

public:
    void start()
    {
        closeZmq();
        _stop.store(false, std::memory_order_release);
        _zmq_ctx = zmq_ctx_new();
        if (_zmq_ctx == nullptr) {
            return;
        }
        _zmq_sub = zmq_socket(_zmq_ctx, ZMQ_SUB);
        if (_zmq_sub == nullptr) {
            closeZmq();
            return;
        }
        const std::string filter = topic_filter.empty() ? std::string{detail::kGrIdentPreambleTopic} : topic_filter;
        zmq_setsockopt(_zmq_sub, ZMQ_SUBSCRIBE, filter.data(), filter.size());
        const int ep_rc = bind_socket ? zmq_bind(_zmq_sub, endpoint.c_str()) : zmq_connect(_zmq_sub, endpoint.c_str());
        if (ep_rc != 0) {
            closeZmq();
            return;
        }
        _io_thread = std::thread([this] { ioLoop(); });
    }

    void stop() { closeZmq(); }

    [[nodiscard]] gr::work::Status processBulk() noexcept { return gr::work::Status::OK; }
};

#endif // GR_QRAD_GR4_HAVE_ZMQ

} // namespace gnuradio4::qradiolink

#endif
