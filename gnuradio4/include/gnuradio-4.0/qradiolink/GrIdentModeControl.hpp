// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_GRIDENTMODECONTROL_HPP
#define GNURADIO4_QRAD_GRIDENTMODECONTROL_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/Message.hpp>
#include <gnuradio-4.0/Port.hpp>
#include <gnuradio-4.0/Value.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/GrIdentModeMap.hpp>
#include <gnuradio-4.0/qradiolink/detail/GrIdentZmqProtocol.hpp>

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::GrIdentModeControl)

struct GrIdentModeControl : gr::Block<GrIdentModeControl, gr::NoTagPropagation> {
    using Description = gr::Doc<"Apply gr-ident preamble JSON (ZMQ :5560 wire format) to mod/demod route selection.">;

    gr::MsgPortIn  msg_preamble_in{};
    gr::MsgPortOut msg_route_out{};

    gr::Annotated<std::uint16_t, "mode_id", gr::Doc<"Last gr-ident mode_id (0-511).">> mode_id{0U};
    gr::Annotated<bool, "digital", gr::Doc<"gr-ident bit 11 (digital flag).">> digital{false};
    gr::Annotated<bool, "encrypted", gr::Doc<"gr-ident bit 10.">> encrypted{false};
    gr::Annotated<bool, "metadata_present", gr::Doc<"gr-ident bit 9.">> metadata_present{false};
    gr::Annotated<std::uint8_t, "active_demod_route", gr::Doc<"detail::QrModemRoute as uint8 for demod bank.">>
        active_demod_route{static_cast<std::uint8_t>(detail::QrModemRoute::Unknown)};
    gr::Annotated<std::uint8_t, "active_mod_route", gr::Doc<"detail::QrModemRoute as uint8 for mod bank.">>
        active_mod_route{static_cast<std::uint8_t>(detail::QrModemRoute::Unknown)};
    gr::Annotated<bool, "route_valid", gr::Doc<"True when mode_id maps to a qradiolink block family.">>
        route_valid{false};

    GR_MAKE_REFLECTABLE(GrIdentModeControl,
        msg_preamble_in,
        msg_route_out,
        mode_id,
        digital,
        encrypted,
        metadata_present,
        active_demod_route,
        active_mod_route,
        route_valid);

private:
    [[nodiscard]] static std::string_view jsonFromPropertyMap(const gr::property_map& body) noexcept
    {
        const auto key_json = gr::convert_string_domain(std::string_view("json"));
        const auto it       = body.find(key_json);
        if (it != body.end()) {
            return it->second.value_or(std::string_view{});
        }
        const auto key_alt = gr::convert_string_domain(std::string_view("preamble_json"));
        const auto it_alt  = body.find(key_alt);
        if (it_alt != body.end()) {
            return it_alt->second.value_or(std::string_view{});
        }
        return {};
    }

    void applySelection(const detail::GrIdentModeSelection& sel) noexcept
    {
        mode_id            = sel.preamble.mode_id;
        digital            = sel.preamble.digital;
        encrypted          = sel.preamble.encrypted;
        metadata_present   = sel.preamble.metadata_present;
        active_demod_route = static_cast<std::uint8_t>(sel.demod_route);
        active_mod_route   = static_cast<std::uint8_t>(sel.mod_route);
        route_valid        = sel.route_valid;
    }

    void publishRouteNotify()
    {
        const auto demod_route = static_cast<detail::QrModemRoute>(active_demod_route.value);
        const auto mod_route   = static_cast<detail::QrModemRoute>(active_mod_route.value);

        gr::property_map body;
        body[gr::convert_string_domain(std::string_view("mode_id"))] =
            gr::pmt::Value(static_cast<std::uint32_t>(mode_id.value));
        body[gr::convert_string_domain(std::string_view("digital"))]           = gr::pmt::Value(digital.value);
        body[gr::convert_string_domain(std::string_view("encrypted"))]         = gr::pmt::Value(encrypted.value);
        body[gr::convert_string_domain(std::string_view("metadata_present"))]  = gr::pmt::Value(metadata_present.value);
        body[gr::convert_string_domain(std::string_view("active_demod_route"))] =
            gr::pmt::Value(static_cast<std::uint32_t>(active_demod_route.value));
        body[gr::convert_string_domain(std::string_view("active_mod_route"))] =
            gr::pmt::Value(static_cast<std::uint32_t>(active_mod_route.value));
        body[gr::convert_string_domain(std::string_view("route_valid"))] = gr::pmt::Value(route_valid.value);
        body[gr::convert_string_domain(std::string_view("demod_block"))] =
            gr::pmt::Value(std::pmr::string{detail::qrRouteName(demod_route)});
        body[gr::convert_string_domain(std::string_view("mod_block"))] =
            gr::pmt::Value(std::pmr::string{detail::qrRouteName(mod_route)});

        gr::Message outgoing;
        outgoing.cmd  = gr::message::Command::Notify;
        outgoing.data = std::move(body);
        auto w        = msg_route_out.streamWriter().template reserve<gr::SpanReleasePolicy::ProcessAll>(1UZ);
        w[0]          = std::move(outgoing);
        w.publish(1UZ);
    }

public:
    [[nodiscard]] gr::work::Status processBulk() noexcept { return gr::work::Status::OK; }

    void processMessages(gr::MsgPortIn& port, std::span<const gr::Message> messages)
    {
        if (std::addressof(port) != std::addressof(msg_preamble_in)) {
            return;
        }
        for (const auto& msg : messages) {
            if (!msg.data.has_value()) {
                continue;
            }
            const std::string_view json_body = jsonFromPropertyMap(msg.data.value());
            if (json_body.empty()) {
                continue;
            }
            const auto sel = detail::selectionFromPreambleJson(json_body);
            if (!sel) {
                continue;
            }
            applySelection(*sel);
            publishRouteNotify();
        }
    }

    void applyPreambleZmqFrames(std::string_view topic, std::string_view json_body)
    {
        const auto parsed = detail::parsePreambleResultMultipart(topic, json_body);
        if (!parsed) {
            return;
        }
        applySelection(detail::selectionFromPreamble(*parsed));
        publishRouteNotify();
    }
};

} // namespace gnuradio4::qradiolink

#endif
