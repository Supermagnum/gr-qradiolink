// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_STRETCHERCC_HPP
#define GNURADIO4_QRAD_STRETCHERCC_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::StretcherCc)

struct StretcherCc : gr::Block<StretcherCc> {
    using Description =
        gr::Doc<"Heuristic envelope-aware stretch scalar (standalone portable substitute for the VOLK heavy general_work block).">;

    gr::PortIn<std::complex<float>>            in{};
    gr::PortOut<std::complex<float>>         out{};

    GR_MAKE_REFLECTABLE(StretcherCc, in, out);

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& ins, std::span<std::complex<float>>& outs) noexcept {
        constexpr float kEmaxCoeff = 1.0F / ((0.70710677F / 2.0F));
        const auto      lim        = outs.size();
        for (std::size_t idx = 0; idx + 1UZ < std::min(ins.size(), outs.size()); ++idx) {
            float       envelope = std::max({std::hypot(ins[idx].real(), ins[idx].imag()),
                std::hypot(ins[idx].real(), ins[idx].imag()),
                idx + 1UZ < lim ? std::hypot(ins[idx + 1UZ].real(), ins[idx + 1UZ].imag()) : 0.F});
            envelope              = envelope * kEmaxCoeff;
            envelope              = std::max(envelope, 1.F);
            envelope              = ((envelope - 1.F) * 2.F) + 1.F;
            const float denominator = envelope < 1.0e-6F ? 1.0F : envelope;
            const float rr          = ins[idx].real() / denominator;
            const float ij          = ins[idx].imag() / denominator;
            outs[idx]               = std::complex<float>{rr, ij};
        }
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
