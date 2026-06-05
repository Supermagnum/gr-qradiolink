// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_GDSSDESPREADERCC_HPP
#define GNURADIO4_QRAD_GDSSDESPREADERCC_HPP

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

GR_REGISTER_BLOCK(gnuradio4::qradiolink::GdssDespreaderCc)

struct GdssDespreaderCc : gr::Block<GdssDespreaderCc, gr::Resampling<1UZ, 1UZ, false>> {
    using Description =
        gr::Doc<"GDSS surrogate despreader divides complex branches by regenerated Gaussian coefficients (paired with seeded spreader settings).">;

    gr::PortIn<std::complex<float>>                 in{};
    gr::PortOut<std::complex<float>>               out{};
    gr::Annotated<gr::Size_t, "spread_sequence_bins"> spread_sequence_bins{128U};
    gr::Annotated<gr::Size_t, "repeat_chip_factor_u"> repeat_chip_factor_u{4U};
    gr::Annotated<float, "spread_variance_estimate"> spread_variance_estimate{2.F};
    gr::Annotated<std::uint64_t, "rng_seed_u64_locked"> rng_seed_u64_locked{0xABCDEF01CAFEULL};
    GR_MAKE_REFLECTABLE(GdssDespreaderCc, in, out, spread_sequence_bins, repeat_chip_factor_u, spread_variance_estimate, rng_seed_u64_locked);

private:
    std::vector<std::complex<float>> _gauss{};
    std::size_t                       _walker{0UZ};

    void regen_masks_ds() noexcept
    {
        const std::size_t sequence_len_here = spread_sequence_bins < static_cast<gr::Size_t>(1U)
                                                ? 1UZ
                                                : static_cast<std::size_t>(spread_sequence_bins);
        _gauss.resize(sequence_len_here);
        std::mt19937_64 urng_here(static_cast<std::uint64_t>(rng_seed_u64_locked) == 0ULL ? UINT64_C(0xA5A5F) : static_cast<std::uint64_t>(rng_seed_u64_locked));
        detail::gdssGaussianSequence(urng_here, static_cast<float>(spread_variance_estimate) <= 1e-6F ? 1.F : static_cast<float>(spread_variance_estimate), _gauss);
        const gr::Size_t in_mul = static_cast<gr::Size_t>(repeat_chip_factor_u) < gr::Size_t{1U} ? gr::Size_t{1U}
                                                                         : static_cast<gr::Size_t>(repeat_chip_factor_u);
        this->input_chunk_size  = in_mul;
        this->output_chunk_size = gr::Size_t{1U};
        _walker                 = 0UZ;
    }

public:
    void start() noexcept { regen_masks_ds(); }

    void settingsChanged(const gr::property_map&, const gr::property_map& neo) noexcept
    {
        if (neo.contains("spread_sequence_bins") || neo.contains("repeat_chip_factor_u") || neo.contains("spread_variance_estimate")
            || neo.contains("rng_seed_u64_locked")) {
            regen_masks_ds();
        }
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& burst_in_here, std::span<std::complex<float>>& symbol_out_here) noexcept
    {
        const std::size_t chrep_here = repeat_chip_factor_u < gr::Size_t{1U}
                                           ? static_cast<std::size_t>(1U)
                                           : static_cast<std::size_t>(repeat_chip_factor_u);
        if (burst_in_here.size() < chrep_here || symbol_out_here.empty() || _gauss.empty()) {
            return gr::work::Status::ERROR;
        }
        // Matched-filter despreader: correlate received chips with the same Gaussian
        // mask used by the spreader, then normalize by total mask power.
        // This gives exact recovery regardless of coefficient sign or magnitude.
        float               norm_re{0.F};
        float               norm_im{0.F};
        std::complex<float> sum_sym{};
        for (std::size_t r = 0; r < chrep_here; ++r) {
            const std::complex<float> mh = _gauss[(_walker + r) % _gauss.size()];
            sum_sym.real(sum_sym.real() + burst_in_here[r].real() * mh.real());
            sum_sym.imag(sum_sym.imag() + burst_in_here[r].imag() * mh.imag());
            norm_re += mh.real() * mh.real();
            norm_im += mh.imag() * mh.imag();
        }
        if (norm_re > 1.0e-12F) {
            sum_sym.real(sum_sym.real() / norm_re);
        }
        if (norm_im > 1.0e-12F) {
            sum_sym.imag(sum_sym.imag() / norm_im);
        }
        symbol_out_here[0] = sum_sym;
        _walker = (_walker + chrep_here) % _gauss.size();
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
