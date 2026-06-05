// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>

#include <gnuradio-4.0/Message.hpp>
#include <gnuradio-4.0/Port.hpp>
#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/Tensor.hpp>
#include <gnuradio-4.0/Value.hpp>
#include <gnuradio-4.0/qradiolink/ModFreeDv.hpp>
#include <gnuradio-4.0/qradiolink/ModM17.hpp>
#include <gnuradio-4.0/qradiolink/detail/PduHelpers.hpp>
#include <gnuradio-4.0/qradiolink/detail/VoicePduEngine.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] std::uint8_t tensor_flat_at(const gr::Tensor<std::uint8_t>& tens, std::size_t ix) noexcept { return tens.data()[static_cast<std::ptrdiff_t>(ix)]; }

const boost::ut::suite qr_digital_voice_pdu = [] {
    using namespace boost::ut;
    using namespace gr;

    "tagged voice helpers round-trip PDU tags"_test = [] {
        std::vector<std::uint8_t> pay{42U};
        gnuradio4::qradiolink::detail::pduApplyM17Mod(pay);
        bool ok = gnuradio4::qradiolink::detail::pduApplyM17Demod(pay);
        expect(ok);
        expect(eq(pay.size(), 1UZ));
        expect(eq(pay[0], 42U));

        gnuradio4::qradiolink::detail::pduApplyDmEncode(pay);
        ok = gnuradio4::qradiolink::detail::pduApplyDmDecode(pay);
        expect(ok);

        gnuradio4::qradiolink::detail::pduApplyNxEncode(pay);
        ok = gnuradio4::qradiolink::detail::pduApplyNxDecode(pay);
        expect(ok);
    };

#ifndef GR_QRAD_GR4_HAVE_CODEC2
    "codec2-absent FreeDv leaves tagged stub payloads"_test = [] {
        std::vector<std::uint8_t> pay{};
        gnuradio4::qradiolink::detail::pduFdMod(pay);
        expect(pay.size() >= 4UZ);
        const bool unr = gnuradio4::qradiolink::detail::pduFdDem(pay);
        expect(unr);
        expect(eq(pay.size(), 0UZ));
    };
#endif

    "ModM17 message path prepends discriminator tag"_test = [] {
        using namespace gnuradio4::qradiolink::detail;

        gnuradio4::qradiolink::ModM17 blk{};
        blk.init(std::make_shared<gr::Sequence>());

        gr::MsgPortIn capture_in{};
        expect(blk.msg_pdu_out.connect(capture_in).has_value());

        gr::Tensor<std::uint8_t> tens(gr::data_from, std::initializer_list<std::uint8_t>{9U, 10U, 15U});
        gr::property_map         body{};
        body.insert_or_assign(gr::convert_string_domain(std::string_view("extra_note")), gr::pmt::Value(std::string_view("ping")));
        body.insert_or_assign(gr::convert_string_domain(std::string_view("pdu_data")), gr::pmt::Value(std::move(tens)));

        gr::Message pdu_msg{};
        pdu_msg.cmd  = gr::message::Command::Notify;
        pdu_msg.data.emplace(std::move(body));

        std::vector<gr::Message> inbox;
        inbox.emplace_back(std::move(pdu_msg));
        blk.processMessages(blk.msg_pdu_in, std::span<const gr::Message>(inbox.data(), inbox.size()));

        expect(gt(capture_in.streamReader().available(), 0UZ));
        ReaderSpanLike auto rsp = capture_in.streamReader().get<gr::SpanReleasePolicy::ProcessAll>(1UZ);
        expect(eq(rsp.size(), 1UZ));

        const gr::Message routed = rsp[0];
        expect(eq(rsp.consume(rsp.size()), true));

        expect(routed.data.has_value());
        const auto* mapped = gnuradio4::qradiolink::detail::tensorBytesFromMap(*routed.data, std::string_view("pdu_data"));
        expect(mapped != nullptr);
        expect(ge(mapped->size(), 7UZ)); // prefix + preserved payload octets

        expect(eq(tensor_flat_at(*mapped, 0UZ), kTagM17[0]));
        expect(eq(tensor_flat_at(*mapped, 1UZ), kTagM17[1]));
        expect(eq(tensor_flat_at(*mapped, 2UZ), kTagM17[2]));
        expect(eq(tensor_flat_at(*mapped, 3UZ), kTagM17[3]));
        expect(eq(tensor_flat_at(*mapped, 4UZ), std::uint8_t{9U}));
        expect(eq(tensor_flat_at(*mapped, 5UZ), std::uint8_t{10U}));
        expect(eq(tensor_flat_at(*mapped, 6UZ), std::uint8_t{15U}));

        expect(eq(routed.data.value().contains(gr::convert_string_domain(std::string_view("extra_note"))), true));
    };

#ifdef GR_QRAD_GR4_HAVE_CODEC2
    "ModFreeDv accepts codec-linked PDU shim"_test = [] {
        gnuradio4::qradiolink::ModFreeDv blk{};
        blk.init(std::make_shared<gr::Sequence>());
        expect(blk.processBulk() == gr::work::Status::OK);

        gr::MsgPortIn cap{};
        expect(blk.msg_pdu_out.connect(cap).has_value());
        gr::Tensor<std::uint8_t> ten(gr::data_from, std::initializer_list<std::uint8_t>{3U});
        gr::property_map         body{};
        body.insert_or_assign(gr::convert_string_domain(std::string_view("pdu_data")), gr::pmt::Value(std::move(ten)));
        gr::Message pdu_msg{};
        pdu_msg.cmd  = gr::message::Command::Notify;
        pdu_msg.data.emplace(std::move(body));

        std::vector<gr::Message> msgs;
        msgs.emplace_back(std::move(pdu_msg));
        blk.processMessages(blk.msg_pdu_in, std::span<const gr::Message>(msgs.data(), msgs.size()));
        expect(ge(cap.streamReader().available(), 1UZ));
    };
#endif
};
} // namespace

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
