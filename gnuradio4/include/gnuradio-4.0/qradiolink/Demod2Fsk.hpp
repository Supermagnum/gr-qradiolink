// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DEMOD2FSK_HPP
#define GNURADIO4_QRAD_DEMOD2FSK_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/SimpleDigitalAnalog.hpp>

#include <complex>
#include <cstddef>
#include <cstdint>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::Demod2Fsk)

struct Demod2Fsk : gr::Block<Demod2Fsk> {
    using Description = gr::Doc<"Unpacked-symbol demapper (deterministic coherence helper for GR4 qa).">;
    gr::PortIn<std::complex<float>>         in{};
    gr::PortOut<std::uint8_t>              out{};
    gr::Annotated<gr::Size_t, "samples_per_symbol"> samples_per_symbol{16U};
    gr::Annotated<float, "frequency_separation_hz"> frequency_separation_hz{2400.F};

    GR_MAKE_REFLECTABLE(Demod2Fsk, in, out, samples_per_symbol, frequency_separation_hz);

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& in_span, std::span<std::uint8_t>& out_span) noexcept {
        const std::size_t sps_local   = samples_per_symbol < gr::Size_t{1U} ? 1UZ : static_cast<std::size_t>(samples_per_symbol);
        const unsigned    bps         = detail::bitsPerSymbolDigital(detail::DigitalKind::Fsk2);
        if (in_span.size() % sps_local != 0UZ) {
            return gr::work::Status::ERROR;
        }
        const std::size_t nsymbols = in_span.size() / sps_local;
        const std::size_t need_out   = nsymbols * static_cast<std::size_t>(bps);
        if (out_span.size() < need_out) {
            return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
        }
        std::span<std::uint8_t> bit_view = out_span.subspan(0UZ, need_out);
        detail::digitalDemodulateHardBits(detail::DigitalKind::Fsk2, in_span, bit_view, sps_local, frequency_separation_hz);
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif // GNURADIO4_QRAD_DEMOD2FSK_HPP
