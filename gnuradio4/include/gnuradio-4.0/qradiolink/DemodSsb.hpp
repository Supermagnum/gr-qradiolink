// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DEMODSSB_HPP
#define GNURADIO4_QRAD_DEMODSSB_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/SimpleDigitalAnalog.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::DemodSsb)

struct DemodSsb : gr::Block<DemodSsb> {
    using Description = gr::Doc<"USB SSB heuristic demod sample-for-sample.">;

    gr::PortIn<std::complex<float>>  in{};
    gr::PortOut<float>               out{};
    gr::Annotated<float, "sample_rate_hz"> sample_rate_hz{48000.F};
    gr::Annotated<float, "carrier_hz"> carrier_hz{1500.F};

    GR_MAKE_REFLECTABLE(DemodSsb, in, out, sample_rate_hz, carrier_hz);

private:
    detail::AnalogModemState _modem{};

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& in_span, std::span<float>& out_span) noexcept {
        if (out_span.size() < in_span.size()) {
            return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
        }
        auto audio = out_span.subspan(0UZ, in_span.size());
        detail::analogDemodSamples(detail::AnalogKind::SsbUsb, in_span, audio, sample_rate_hz, carrier_hz, 1.F, 100.F, _modem);
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
