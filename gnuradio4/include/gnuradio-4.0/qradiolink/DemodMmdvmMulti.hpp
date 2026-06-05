// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DEMODMMDVMMULTI_HPP
#define GNURADIO4_QRAD_DEMODMMDVMMULTI_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::DemodMmdvmMulti)

struct DemodMmdvmMulti : gr::Block<DemodMmdvmMulti> {
    using Description = gr::Doc<"Deterministic float quartet to complex stub (GR4 graph-free replacement for multi-channel MMDVM demod).">;

    gr::PortIn<float>                  branch_0_energy{};
    gr::PortIn<float>                  branch_1_energy{};
    gr::PortIn<float>                  branch_2_energy{};
    gr::PortIn<float>                  branch_3_energy{};
    gr::PortOut<std::complex<float>> iq_result{};
    GR_MAKE_REFLECTABLE(DemodMmdvmMulti, branch_0_energy, branch_1_energy, branch_2_energy, branch_3_energy, iq_result);

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const float>& b0,
        std::span<const float>& b1,
        std::span<const float>& b2,
        std::span<const float>& b3,
        std::span<std::complex<float>>& oq) noexcept
    {
        const std::size_t ncopy = std::min({b0.size(), b1.size(), b2.size(), b3.size(), oq.size()});
        constexpr float s = 0.5F;
        for (std::size_t j = 0; j < ncopy; ++j) {
            oq[j] =
                {(b0[j] - b2[j]) * s, (b1[j] - b3[j]) * s}; // quadrant proxy from four channel powers
        }
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
