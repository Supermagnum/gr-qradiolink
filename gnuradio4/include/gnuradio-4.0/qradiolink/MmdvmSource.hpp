// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_MMDVMSOURCE_HPP
#define GNURADIO4_QRAD_MMDVMSOURCE_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::MmdvmSource)

struct MmdvmSource : gr::Block<MmdvmSource> {
    using Description = gr::Doc<"Placeholder MMDVM IQ source (zero-stuffed carrier for GR4 graph-free tests).">;

    gr::PortOut<std::complex<float>> out{};
    GR_MAKE_REFLECTABLE(MmdvmSource, out);

    [[nodiscard]] gr::work::Status processBulk(gr::OutputSpanLike auto& output_span_chunk) noexcept
    {
        const std::size_t publish_len = output_span_chunk.size();
        for (std::size_t n = 0; n < publish_len; ++n) {
            output_span_chunk[static_cast<std::ptrdiff_t>(n)] = {0.001F, 0.F};
        }
        output_span_chunk.publish(publish_len);
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
