// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DEMODMMDVMMULTI2_HPP
#define GNURADIO4_QRAD_DEMODMMDVMMULTI2_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::DemodMmdvmMulti2)

struct DemodMmdvmMulti2 : gr::Block<DemodMmdvmMulti2> {
    using Description =
        gr::Doc<"Single-stream complex surrogate for downstream MMDVM multi2 demod staging (deterministic qa identity).">;

    gr::PortIn<std::complex<float>>  in_wave{};
    gr::PortOut<std::complex<float>> out_wave{};
    GR_MAKE_REFLECTABLE(DemodMmdvmMulti2, in_wave, out_wave);

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& ins, std::span<std::complex<float>>& outs) noexcept
    {
        const auto n_take = std::min(ins.size(), outs.size());
        for (std::size_t ix = 0; ix < n_take; ++ix) {
            outs[ix] = ins[ix];
        }
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
