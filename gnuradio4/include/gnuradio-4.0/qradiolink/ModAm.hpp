// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_MODAM_HPP
#define GNURADIO4_QRAD_MODAM_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/SimpleDigitalAnalog.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::ModAm)

struct ModAm : gr::Block<ModAm> {
    using Description = gr::Doc<"AM envelope modulator sample-for-sample heuristic (GR4 qa).">;

    gr::PortIn<float>                  in{};
    gr::PortOut<std::complex<float>>   out{};
    gr::Annotated<float, "sample_rate_hz"> sample_rate_hz{48000.F};
    gr::Annotated<float, "carrier_hz"> carrier_hz{1000.F};
    gr::Annotated<float, "modulation_index"> modulation_index{0.5F};

    GR_MAKE_REFLECTABLE(ModAm, in, out, sample_rate_hz, carrier_hz, modulation_index);

private:
    detail::AnalogModemState _modem{};

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const float>& in_span, std::span<std::complex<float>>& out_span) noexcept {
        if (out_span.size() < in_span.size()) {
            return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
        }
        const auto wav = out_span.subspan(0UZ, in_span.size());
        detail::analogTransmitSamples(detail::AnalogKind::Am, in_span, wav, sample_rate_hz, carrier_hz, modulation_index, 100.F /*unused*/,
            _modem);
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
