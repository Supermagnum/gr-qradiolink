// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_MMDVMSINK_HPP
#define GNURADIO4_QRAD_MMDVMSINK_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::MmdvmSink)

struct MmdvmSink : gr::Block<MmdvmSink> {
    using Description = gr::Doc<"Placeholder MMDVM IQ sink (discards samples for GR4 graph-free tests).">;

    gr::PortIn<std::complex<float>> in{};
    GR_MAKE_REFLECTABLE(MmdvmSink, in);

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& ins) noexcept
    {
        (void)ins;
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
