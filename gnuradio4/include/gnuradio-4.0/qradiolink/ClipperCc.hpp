// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_CLIPPERCC_HPP
#define GNURADIO4_QRAD_CLIPPERCC_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::ClipperCc)

struct ClipperCc : gr::Block<ClipperCc> {
    using Description = gr::Doc<"Complex amplitude clipper akin to VOLK clip path (ported scalar qa form).">;

    gr::PortIn<std::complex<float>>           in{};
    gr::PortOut<std::complex<float>>        out{};
    gr::Annotated<float, "threshold_level_amplitude"> threshold_level_amplitude{0.99F};

    GR_MAKE_REFLECTABLE(ClipperCc, in, out, threshold_level_amplitude);

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& ins, std::span<std::complex<float>>& outs) noexcept {
        const std::size_t n = std::min(ins.size(), outs.size());
        const float       t = static_cast<float>(threshold_level_amplitude) <= 1.0e-6F ? 1.0e-6F : static_cast<float>(threshold_level_amplitude);
        for (std::size_t i = 0; i < n; ++i) {
            float       ph = std::atan2(ins[i].imag(), ins[i].real());
            float       mg = std::hypot(ins[i].real(), ins[i].imag());
            const float cl = std::min(mg, t);
            outs[i]        = std::complex<float>{cl * std::cos(ph), cl * std::sin(ph)};
        }
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
