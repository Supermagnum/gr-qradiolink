// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_GDSSSPREADERCC_HPP
#define GNURADIO4_QRAD_GDSSSPREADERCC_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/DsssGdssTools.hpp>

#include <complex>
#include <cstddef>
#include <random>
#include <span>
#include <vector>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::GdssSpreaderCc)

struct GdssSpreaderCc : gr::Block<GdssSpreaderCc, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = gr::Doc<"GDSS surrogate: IID Gaussian-ish mask replicated per-chip (standalone variance-limited qa form).">;

    gr::PortIn<std::complex<float>>                 in{};
    gr::PortOut<std::complex<float>>               out{};
    gr::Annotated<gr::Size_t, "spread_sequence_bins"> spread_sequence_bins{128U};
    gr::Annotated<gr::Size_t, "repeat_chip_factor_u"> repeat_chip_factor_u{4U};
    gr::Annotated<float, "spread_variance_estimate"> spread_variance_estimate{2.F};
    gr::Annotated<std::uint64_t, "rng_seed_u64_locked"> rng_seed_u64_locked{0xABCDEF01CAFEULL};
    GR_MAKE_REFLECTABLE(GdssSpreaderCc, in, out, spread_sequence_bins, repeat_chip_factor_u, spread_variance_estimate, rng_seed_u64_locked);

private:
    std::vector<std::complex<float>> _gauss_mask_lut{};
    std::size_t                       _walker{0UZ};

    void regen_masks() noexcept
    {
        const std::size_t sequence_len_here = spread_sequence_bins < static_cast<gr::Size_t>(1U)
                                                ? 1UZ
                                                : static_cast<std::size_t>(spread_sequence_bins);
        _gauss_mask_lut.resize(sequence_len_here);
        std::mt19937_64 urng(static_cast<std::uint64_t>(rng_seed_u64_locked) == 0ULL ? UINT64_C(0xA5A5F) : static_cast<std::uint64_t>(rng_seed_u64_locked));
        detail::gdssGaussianSequence(urng, static_cast<float>(spread_variance_estimate) <= 1.0e-6F ? 1.F : static_cast<float>(spread_variance_estimate), _gauss_mask_lut);

        const gr::Size_t out_mul = static_cast<gr::Size_t>(repeat_chip_factor_u) < gr::Size_t{1U} ? gr::Size_t{1U}
                                                                            : static_cast<gr::Size_t>(repeat_chip_factor_u);
        this->input_chunk_size  = gr::Size_t{1U};
        this->output_chunk_size = out_mul;
        _walker                 = 0UZ;
    }

public:
    void start() noexcept { regen_masks(); }

    void settingsChanged(const gr::property_map&, const gr::property_map& neo) noexcept
    {
        if (neo.contains("spread_sequence_bins") || neo.contains("repeat_chip_factor_u") || neo.contains("spread_variance_estimate")
            || neo.contains("rng_seed_u64_locked")) {
            regen_masks();
        }
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& in_sym_c, std::span<std::complex<float>>& out_chip_seq) noexcept
    {
        const std::size_t chrep = repeat_chip_factor_u < gr::Size_t{1U}
                                      ? static_cast<std::size_t>(1U)
                                      : static_cast<std::size_t>(repeat_chip_factor_u);
        if (in_sym_c.empty() || out_chip_seq.size() < chrep || _gauss_mask_lut.empty()) {
            return gr::work::Status::ERROR;
        }
        const auto sym_here = in_sym_c[0];
        for (std::size_t h = 0; h < chrep; ++h) {
            const std::complex<float> gm = _gauss_mask_lut[(_walker + h) % _gauss_mask_lut.size()];
            out_chip_seq[h]               = std::complex<float>{
                sym_here.real() * gm.real(), sym_here.imag() * gm.imag()};
        }
        _walker = (_walker + chrep) % _gauss_mask_lut.size();
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
