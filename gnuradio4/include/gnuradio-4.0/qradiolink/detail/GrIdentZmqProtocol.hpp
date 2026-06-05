// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DETAIL_GRIDENTZMQPROTOCOL_HPP
#define GNURADIO4_QRAD_DETAIL_GRIDENTZMQPROTOCOL_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace gnuradio4::qradiolink::detail {

// Wire format aligned with gr-ident python/grident/zmq_protocol.py and docs/zeromq-protocol.md.

inline constexpr std::string_view kGrIdentPreamblePubDefault   = "tcp://127.0.0.1:5560";
inline constexpr std::string_view kGrIdentTxControlDefault       = "tcp://127.0.0.1:5561";
inline constexpr std::string_view kGrIdentIqPushPullDefault      = "tcp://127.0.0.1:5555";
inline constexpr std::string_view kGrIdentPreambleTopic          = "grident";
inline constexpr std::string_view kGrIdentPreambleTopicPrefix     = "grident.";
inline constexpr std::string_view kGrIdentTxTopic                = "grident.tx";

struct GrIdentPreambleResult {
    std::uint16_t mode_id{0U};
    bool          digital{false};
    bool          encrypted{false};
    bool          metadata_present{false};
};

[[nodiscard]] inline bool grIdentTopicMatchesPreamble(std::string_view topic) noexcept
{
    return topic == kGrIdentPreambleTopic || topic.starts_with(kGrIdentPreambleTopicPrefix);
}

[[nodiscard]] inline std::string formatPreambleResultJson(const GrIdentPreambleResult& r)
{
    std::string out;
    out.reserve(96UZ);
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

namespace grident_json_detail {

[[nodiscard]] inline std::optional<std::string_view> extractField(std::string_view json, std::string_view key)
{
    const std::string needle = std::string{'"'} + std::string{key} + "\":";
    const std::size_t pos    = json.find(needle);
    if (pos == std::string_view::npos) {
        return std::nullopt;
    }
    std::string_view tail = json.substr(pos + needle.size());
    while (!tail.empty() && (tail.front() == ' ' || tail.front() == '\t')) {
        tail.remove_prefix(1UZ);
    }
    return tail;
}

[[nodiscard]] inline std::optional<int> parseIntField(std::string_view tail)
{
    bool negative = false;
    if (!tail.empty() && tail.front() == '-') {
        negative = true;
        tail.remove_prefix(1UZ);
    }
    int value = 0;
    bool any  = false;
    while (!tail.empty() && tail.front() >= '0' && tail.front() <= '9') {
        value = value * 10 + static_cast<int>(tail.front() - '0');
        tail.remove_prefix(1UZ);
        any = true;
    }
    if (!any) {
        return std::nullopt;
    }
    return negative ? -value : value;
}

[[nodiscard]] inline std::optional<bool> parseBoolField(std::string_view tail)
{
    if (tail.starts_with("true")) {
        return true;
    }
    if (tail.starts_with("false")) {
        return false;
    }
    return std::nullopt;
}

} // namespace grident_json_detail

[[nodiscard]] inline std::optional<GrIdentPreambleResult> parsePreambleResultJson(std::string_view json)
{
    using namespace grident_json_detail;
    const auto mode_tail = extractField(json, "mode_id");
    const auto dig_tail  = extractField(json, "digital");
    const auto enc_tail  = extractField(json, "encrypted");
    const auto meta_tail = extractField(json, "metadata_present");
    if (!mode_tail || !dig_tail || !enc_tail || !meta_tail) {
        return std::nullopt;
    }
    const auto mode_id = parseIntField(*mode_tail);
    const auto digital = parseBoolField(*dig_tail);
    const auto enc     = parseBoolField(*enc_tail);
    const auto meta    = parseBoolField(*meta_tail);
    if (!mode_id || !digital || !enc || !meta) {
        return std::nullopt;
    }
    if (*mode_id < 0 || *mode_id > 511) {
        return std::nullopt;
    }
    return GrIdentPreambleResult{
        static_cast<std::uint16_t>(*mode_id),
        *digital,
        *enc,
        *meta,
    };
}

[[nodiscard]] inline std::optional<GrIdentPreambleResult> parsePreambleResultMultipart(std::string_view topic,
    std::string_view json_body)
{
    if (!grIdentTopicMatchesPreamble(topic)) {
        return std::nullopt;
    }
    return parsePreambleResultJson(json_body);
}

} // namespace gnuradio4::qradiolink::detail

#endif
