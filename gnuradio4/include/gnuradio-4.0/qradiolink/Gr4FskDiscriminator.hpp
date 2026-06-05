// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_GR4FSKDISCRIMINATOR_HPP
#define GNURADIO4_QRAD_GR4FSKDISCRIMINATOR_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::Gr4FskDiscriminator)

struct Gr4FskDiscriminator : gr::Block<Gr4FskDiscriminator> {
    using Description = gr::Doc<"Four-branch energy contest maps to quadrant constellation (parity with legacy float interface).">;

    gr::PortIn<float>                      in_branch0{};
    gr::PortIn<float>                      in_branch1{};
    gr::PortIn<float>                      in_branch2{};
    gr::PortIn<float>                      in_branch3{};
    gr::PortOut<std::complex<float>>       out{};
    GR_MAKE_REFLECTABLE(Gr4FskDiscriminator, in_branch0, in_branch1, in_branch2, in_branch3, out);

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const float>& b0,
        std::span<const float>& b1,
        std::span<const float>& b2,
        std::span<const float>& b3,
        std::span<std::complex<float>>& o) noexcept {
        const std::size_t ntake = std::min({b0.size(), b1.size(), b2.size(), b3.size(), o.size()});
        constexpr float   s     = +0.70710677F;
        for (std::size_t n = 0; n < ntake; ++n) {
            if ((b0[n] > b1[n]) && (b0[n] > b2[n]) && (b0[n] > b3[n])) {
                o[n] = {-s, -s};
            } else if ((b1[n] > b0[n]) && (b1[n] > b2[n]) && (b1[n] > b3[n])) {
                o[n] = {-s, +s};
            } else if ((b2[n] > b1[n]) && (b2[n] > b0[n]) && (b2[n] > b3[n])) {
                o[n] = {+s, +s};
            } else if ((b3[n] > b2[n]) && (b3[n] > b0[n]) && (b3[n] > b1[n])) {
                o[n] = {+s, -s};
            } else {
                o[n] = {0.F, 0.F};
            }
        }
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
