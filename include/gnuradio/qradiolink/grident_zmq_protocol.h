/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * gr-ident ZMQ preamble JSON (tcp://127.0.0.1:5560) aligned with
 * https://github.com/Supermagnum/gr-ident docs/zeromq-protocol.md
 */

#ifndef INCLUDED_QRADIOLINK_GRIDENT_ZMQ_PROTOCOL_H
#define INCLUDED_QRADIOLINK_GRIDENT_ZMQ_PROTOCOL_H

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace gr {
namespace qradiolink {
namespace grident {

inline constexpr const char* k_preamble_pub_default = "tcp://127.0.0.1:5560";
inline constexpr const char* k_tx_control_default   = "tcp://127.0.0.1:5561";
inline constexpr const char* k_preamble_topic       = "grident";
inline constexpr const char* k_preamble_topic_prefix = "grident.";
inline constexpr const char* k_tx_topic             = "grident.tx";

struct preamble_result {
    std::uint16_t mode_id{0};
    bool          digital{false};
    bool          encrypted{false};
    bool          metadata_present{false};
};

inline bool topic_matches_preamble(std::string_view topic) noexcept
{
    return topic == k_preamble_topic || topic.rfind(k_preamble_topic_prefix, 0) == 0;
}

inline std::string format_preamble_json(const preamble_result& r)
{
    std::string out;
    out.reserve(96);
    out += R"({"mode_id":)";
    out += std::to_string(r.mode_id);
    out += R"(,"digital":)";
    out += r.digital ? "true" : "false";
    out += R"(,"encrypted":)";
    out += r.encrypted ? "true" : "false";
    out += R"(,"metadata_present":)";
    out += r.metadata_present ? "true" : "false";
    out += '}';
    return out;
}

namespace detail {

inline std::optional<std::string_view> extract_field(std::string_view json, std::string_view key)
{
    const std::string needle = std::string("\"") + std::string(key) + "\":";
    const std::size_t pos    = json.find(needle);
    if (pos == std::string_view::npos) {
        return std::nullopt;
    }
    std::string_view tail = json.substr(pos + needle.size());
    while (!tail.empty() && (tail.front() == ' ' || tail.front() == '\t')) {
        tail.remove_prefix(1);
    }
    return tail;
}

inline std::optional<int> parse_int_field(std::string_view tail)
{
    bool negative = false;
    if (!tail.empty() && tail.front() == '-') {
        negative = true;
        tail.remove_prefix(1);
    }
    int  value = 0;
    bool any   = false;
    while (!tail.empty() && tail.front() >= '0' && tail.front() <= '9') {
        value = value * 10 + (tail.front() - '0');
        tail.remove_prefix(1);
        any = true;
    }
    if (!any) {
        return std::nullopt;
    }
    return negative ? -value : value;
}

inline std::optional<bool> parse_bool_field(std::string_view tail)
{
    if (tail.rfind("true", 0) == 0) {
        return true;
    }
    if (tail.rfind("false", 0) == 0) {
        return false;
    }
    return std::nullopt;
}

} // namespace detail

inline std::optional<preamble_result> parse_preamble_json(std::string_view json)
{
    using namespace detail;
    const auto mode_tail = extract_field(json, "mode_id");
    const auto dig_tail  = extract_field(json, "digital");
    const auto enc_tail  = extract_field(json, "encrypted");
    const auto meta_tail = extract_field(json, "metadata_present");
    if (!mode_tail || !dig_tail || !enc_tail || !meta_tail) {
        return std::nullopt;
    }
    const auto mode_id = parse_int_field(*mode_tail);
    const auto digital = parse_bool_field(*dig_tail);
    const auto enc     = parse_bool_field(*enc_tail);
    const auto meta    = parse_bool_field(*meta_tail);
    if (!mode_id || !digital || !enc || !meta) {
        return std::nullopt;
    }
    if (*mode_id < 0 || *mode_id > 511) {
        return std::nullopt;
    }
    return preamble_result{ static_cast<std::uint16_t>(*mode_id), *digital, *enc, *meta };
}

inline std::optional<preamble_result> parse_preamble_multipart(std::string_view topic, std::string_view json_body)
{
    if (!topic_matches_preamble(topic)) {
        return std::nullopt;
    }
    return parse_preamble_json(json_body);
}

} // namespace grident
} // namespace qradiolink
} // namespace gr

#endif
