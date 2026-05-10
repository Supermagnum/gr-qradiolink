// SPDX-License-Identifier: GPL-3.0-or-later
// GR4 header-only block — DSP to be matched to GR3 lib/*_impl.cc (see README_gr4.md).
#pragma once

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/meta/reflection.hpp>

#include <complex>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace gr::qradiolink {
struct P25Decoder : gr::Block<P25Decoder> {
    using Description = gr::Doc<"GR4 port placeholder — implement from GR3 p25_decoder_impl.cc.">;

    gr::PortIn<std::uint8_t>   in{};
    gr::PortOut<std::uint8_t>  out{};

    GR_MAKE_REFLECTABLE(P25Decoder, in, out);

    [[nodiscard]] gr::work::Status processBulk(InputSpanLike auto& inSpan, OutputSpanLike auto& outSpan) noexcept {
        std::ignore = inSpan.consume(0UZ);
        outSpan.publish(0UZ);
        return gr::work::Status::INSUFFICIENT_INPUT_ITEMS;
    }
};

} // namespace gr::qradiolink
