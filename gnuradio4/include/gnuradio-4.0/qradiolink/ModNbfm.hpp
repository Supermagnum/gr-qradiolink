// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_MODNBFM_HPP
#define GNURADIO4_QRAD_MODNBFM_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/SimpleDigitalAnalog.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::ModNbfm)

struct ModNbfm : gr::Block<ModNbfm> {
    using Description = gr::Doc<"NBFM discriminator chain sample-for-sample.">;

    gr::PortIn<float>                  in{};
    gr::PortOut<std::complex<float>>   out{};
    gr::Annotated<float, "sample_rate_hz"> sample_rate_hz{48000.F};
    gr::Annotated<float, "carrier_hz"> carrier_hz{1200.F};
    gr::Annotated<float, "deviation_hz"> deviation_hz{2500.F};

    GR_MAKE_REFLECTABLE(ModNbfm, in, out, sample_rate_hz, carrier_hz, deviation_hz);

private:
    detail::AnalogModemState _modem{};

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const float>& in_span, std::span<std::complex<float>>& out_span) noexcept {
        if (out_span.size() < in_span.size()) {
            return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
        }
        const auto wav = out_span.subspan(0UZ, in_span.size());
        detail::analogTransmitSamples(detail::AnalogKind::Nbfm, in_span, wav, sample_rate_hz, carrier_hz, 1.F, deviation_hz,
            _modem);
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
