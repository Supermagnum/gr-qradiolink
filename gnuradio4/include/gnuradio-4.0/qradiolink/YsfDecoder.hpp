// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_YSFDECODER_HPP
#define GNURADIO4_QRAD_YSFDECODER_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/Message.hpp>
#include <gnuradio-4.0/Port.hpp>
#include <gnuradio-4.0/Tensor.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/Value.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/PduHelpers.hpp>
#include <gnuradio-4.0/qradiolink/detail/VoicePduEngine.hpp>

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::YsfDecoder)

struct YsfDecoder : gr::Block<YsfDecoder, gr::NoTagPropagation> {
    using Description = gr::Doc<"PDU shim (deterministic transforms for qa).">;
    gr::MsgPortIn  msg_pdu_in{};
    gr::MsgPortOut msg_pdu_out{};

    GR_MAKE_REFLECTABLE(YsfDecoder, msg_pdu_in, msg_pdu_out);

public:
    [[nodiscard]] gr::work::Status processBulk() noexcept { return gr::work::Status::OK; }

    void processMessages(gr::MsgPortIn& port, std::span<const gr::Message> messages) {
        if (std::addressof(port) != std::addressof(msg_pdu_in)) {
            return;
        }
        for (const auto& msg : messages) {
            if (!msg.data.has_value()) {
                continue;
            }
            gr::property_map body_in = msg.data.value();
            const auto*     pdu_pts  = gnuradio4::qradiolink::detail::tensorBytesFromMap(body_in, std::string_view("pdu_data"));
            if (pdu_pts == nullptr || pdu_pts->empty()) {
                continue;
            }
            std::vector<std::uint8_t> pay(pdu_pts->begin(), pdu_pts->end());
            (void)detail::pduApplyYsfDecode(pay);
            gr::property_map reply(body_in.get_allocator());
            for (const auto& kv : body_in) {
                reply.emplace(kv.first, kv.second);
            }
            reply[gr::convert_string_domain(std::string_view("pdu_data"))] = gr::pmt::Value(gr::Tensor<std::uint8_t>(pay));
            gr::Message outgoing;
            outgoing.cmd  = gr::message::Command::Notify;
            outgoing.data = std::move(reply);
            auto w = msg_pdu_out.streamWriter().template reserve<gr::SpanReleasePolicy::ProcessAll>(1UZ);
            w[0]   = std::move(outgoing);
            w.publish(1UZ);
        }
    }
};

} // namespace gnuradio4::qradiolink

#endif
