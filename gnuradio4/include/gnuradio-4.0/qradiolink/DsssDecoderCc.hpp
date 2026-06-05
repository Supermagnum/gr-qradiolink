// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DSSSDECODERCC_HPP
#define GNURADIO4_QRAD_DSSSDECODERCC_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>
#include <gnuradio-4.0/qradiolink/detail/DsssGdssTools.hpp>

#include <complex>
#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::DsssDecoderCc)

struct DsssDecoderCc : gr::Block<DsssDecoderCc, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = gr::Doc<"DSSS despreader per symbol: correlate window with PN template to produce one IQ sample.">;

    gr::PortIn<std::complex<float>>                   in{};
    gr::PortOut<std::complex<float>>                out{};
    gr::Annotated<std::string, "spread_bits_ascii_binary"> spread_bits_ascii_binary{std::string("1010")};
    gr::Annotated<gr::Size_t, "samples_per_chip_counts"> samples_per_chip_counts{4U};

    GR_MAKE_REFLECTABLE(DsssDecoderCc, in, out, spread_bits_ascii_binary, samples_per_chip_counts);

private:
    std::vector<std::complex<float>> _code_template{};

    void rebuild_codebook() noexcept
    {
        std::vector<int> chips01{};
        detail::parse_pn_ascii_binary(spread_bits_ascii_binary, chips01);
        if (chips01.empty()) {
            chips01.push_back(1);
        }
        _code_template.resize(chips01.size());
        for (std::size_t j = 0; j < chips01.size(); ++j) {
            _code_template[j] = detail::chip01_to_complex(chips01[j]);
        }
        const gr::Size_t in_len = static_cast<gr::Size_t>(_code_template.size())
            * (samples_per_chip_counts < gr::Size_t{1U} ? gr::Size_t{1U} : samples_per_chip_counts);
        this->input_chunk_size  = in_len < gr::Size_t{1U} ? gr::Size_t{1U} : in_len;
        this->output_chunk_size = gr::Size_t{1U};
    }

public:
    void start() noexcept { rebuild_codebook(); }

    void settingsChanged(const gr::property_map&, const gr::property_map& neo) noexcept
    {
        if (neo.contains("spread_bits_ascii_binary") || neo.contains("samples_per_chip_counts")) {
            rebuild_codebook();
        }
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& in_window, std::span<std::complex<float>>& out_sym) noexcept
    {
        const std::size_t sps_here = samples_per_chip_counts < gr::Size_t{1U}
                                         ? 1UZ
                                         : static_cast<std::size_t>(samples_per_chip_counts);
        const std::size_t need_in  = _code_template.size() * sps_here;
        if (in_window.size() < need_in || out_sym.size() < 1UZ) {
            return gr::work::Status::INSUFFICIENT_INPUT_ITEMS;
        }
        std::complex<float> corr_acc{};
        std::size_t          input_idx = 0UZ;
        for (std::size_t ch = 0; ch < _code_template.size(); ++ch) {
            std::complex<float> chip_average{};
            for (std::size_t s = 0; s < sps_here; ++s) {
                chip_average += in_window[input_idx + s];
            }
            chip_average /= static_cast<float>(sps_here);
            corr_acc += chip_average * std::conj(_code_template[ch]);
            input_idx += sps_here;
        }
        corr_acc /= static_cast<float>(_code_template.size());
        out_sym[0] = corr_acc;
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
