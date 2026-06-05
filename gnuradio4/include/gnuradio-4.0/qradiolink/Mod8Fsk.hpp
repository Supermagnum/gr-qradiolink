// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_MOD8FSK_HPP
#define GNURADIO4_QRAD_MOD8FSK_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>

#include <gnuradio-4.0/qradiolink/detail/SimpleDigitalAnalog.hpp>

#include <complex>
#include <cstddef>
#include <cstdint>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::Mod8Fsk)

struct Mod8Fsk : gr::Block<Mod8Fsk> {
    using Description = gr::Doc<"Portable GR4 shim: unpacked bit stream modulator (simplified coherence chain).">;
    gr::PortIn<std::uint8_t>                in{};
    gr::PortOut<std::complex<float>>       out{};
    gr::Annotated<gr::Size_t, "samples_per_symbol", gr::Doc<"Samples emitted per unpacked input bit/group">> samples_per_symbol{16U};
    gr::Annotated<float, "sample_rate_hz", gr::Doc<"Passband sample rate in Hz">> sample_rate_hz{48000.F};
    gr::Annotated<float, "carrier_offset_hz"> carrier_offset_hz{0.F};
    gr::Annotated<float, "frequency_separation_hz", gr::Doc<"FSK tone spacing">> frequency_separation_hz{2400.F};

    GR_MAKE_REFLECTABLE(Mod8Fsk, in, out, samples_per_symbol, sample_rate_hz, carrier_offset_hz, frequency_separation_hz);

private:
    detail::AnalogModemState _modem{};

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const std::uint8_t>& in_span, std::span<std::complex<float>>& out_span) noexcept {
        const std::size_t sps_local = samples_per_symbol < gr::Size_t{1U} ? 1UZ : static_cast<std::size_t>(samples_per_symbol);
        const std::size_t need_out  = in_span.size() * sps_local;
        if (out_span.size() < need_out) {
            return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
        }
        detail::digitalModulateWaveform(detail::DigitalKind::Fsk8,
            in_span,
            out_span.subspan(0UZ, need_out),
            sps_local,
            sample_rate_hz,
            carrier_offset_hz,
            frequency_separation_hz,
            _modem);
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif // GNURADIO4_QRAD_MOD8FSK_HPP
