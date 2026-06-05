// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_MODSSB_HPP
#define GNURADIO4_QRAD_MODSSB_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/SimpleDigitalAnalog.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::ModSsb)

struct ModSsb : gr::Block<ModSsb> {
    using Description = gr::Doc<"USB SSB heuristic modulator sample-for-sample.">;

    gr::PortIn<float>                  in{};
    gr::PortOut<std::complex<float>>   out{};
    gr::Annotated<float, "sample_rate_hz"> sample_rate_hz{48000.F};
    gr::Annotated<float, "carrier_hz"> carrier_hz{1500.F};

    GR_MAKE_REFLECTABLE(ModSsb, in, out, sample_rate_hz, carrier_hz);

private:
    detail::AnalogModemState _modem{};

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const float>& in_span, std::span<std::complex<float>>& out_span) noexcept {
        if (out_span.size() < in_span.size()) {
            return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
        }
        const auto wav = out_span.subspan(0UZ, in_span.size());
        detail::analogTransmitSamples(detail::AnalogKind::SsbUsb, in_span, wav, sample_rate_hz, carrier_hz, 1.0F, 100.F, _modem);
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
