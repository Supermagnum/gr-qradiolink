// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_MODWBFM_HPP
#define GNURADIO4_QRAD_MODWBFM_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/SimpleDigitalAnalog.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::ModWbfm)

struct ModWbfm : gr::Block<ModWbfm> {
    using Description = gr::Doc<"WBFM discriminator chain heuristic.">;

    gr::PortIn<float>                  in{};
    gr::PortOut<std::complex<float>>   out{};
    gr::Annotated<float, "sample_rate_hz"> sample_rate_hz{96000.F};
    gr::Annotated<float, "carrier_hz"> carrier_hz{0.F};
    gr::Annotated<float, "peak_deviation_hz"> peak_deviation_hz{75000.F};

    GR_MAKE_REFLECTABLE(ModWbfm, in, out, sample_rate_hz, carrier_hz, peak_deviation_hz);

private:
    detail::AnalogModemState _modem{};

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const float>& in_span, std::span<std::complex<float>>& out_span) noexcept {
        if (out_span.size() < in_span.size()) {
            return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
        }
        const auto wav = out_span.subspan(0UZ, in_span.size());
        detail::analogTransmitSamples(detail::AnalogKind::Wbfm, in_span, wav, sample_rate_hz, carrier_hz, 1.F, peak_deviation_hz,
            _modem);
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
