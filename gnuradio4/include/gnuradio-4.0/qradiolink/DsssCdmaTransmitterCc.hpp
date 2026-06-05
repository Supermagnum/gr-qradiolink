// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DSSSCDMATRANSMITTERCC_HPP
#define GNURADIO4_QRAD_DSSSCDMATRANSMITTERCC_HPP

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

GR_REGISTER_BLOCK(gnuradio4::qradiolink::DsssCdmaTransmitterCc)

struct DsssCdmaTransmitterCc : gr::Block<DsssCdmaTransmitterCc, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = gr::Doc<"Single-flow CDMA surrogate: multiplies IQ symbols against one PN train (deterministic qa path).">;

    gr::PortIn<std::complex<float>>                    in{};
    gr::PortOut<std::complex<float>>                out{};
    gr::Annotated<std::string, "spread_bits_ascii_binary_cd"> spread_bits_ascii_binary_cd{std::string("1100")};
    gr::Annotated<gr::Size_t, "chips_scaled_per_symbol_uc"> chips_scaled_per_symbol_uc{4U};
    GR_MAKE_REFLECTABLE(DsssCdmaTransmitterCc, in, out, spread_bits_ascii_binary_cd, chips_scaled_per_symbol_uc);

private:
    std::vector<std::complex<float>> _pn_train{};
    std::size_t                      _walker{0UZ};

    void regen_wave() noexcept
    {
        std::vector<int> ch{};
        detail::parse_pn_ascii_binary(spread_bits_ascii_binary_cd, ch);
        if (ch.empty()) {
            ch.push_back(1);
        }
        _pn_train.resize(ch.size());
        for (std::size_t u = 0; u < ch.size(); ++u) {
            _pn_train[u] = detail::chip01_to_complex(ch[u]);
        }
        const gr::Size_t out_len_chip = chips_scaled_per_symbol_uc < gr::Size_t{1U} ? gr::Size_t{1U}
                                                                                       : chips_scaled_per_symbol_uc;
        this->input_chunk_size           = gr::Size_t{1U};
        this->output_chunk_size           = out_len_chip;
        _walker                          = 0UZ;
    }

public:
    void start() noexcept { regen_wave(); }

    void settingsChanged(const gr::property_map&, const gr::property_map& neo) noexcept
    {
        if (neo.contains("spread_bits_ascii_binary_cd") || neo.contains("chips_scaled_per_symbol_uc")) {
            regen_wave();
        }
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& sym_in_span, std::span<std::complex<float>>& chip_burst_out) noexcept
    {
        const std::size_t cps_here = chips_scaled_per_symbol_uc < gr::Size_t{1U} ? 1UZ : static_cast<std::size_t>(chips_scaled_per_symbol_uc);
        if (sym_in_span.empty() || chip_burst_out.size() < cps_here || _pn_train.empty()) {
            return gr::work::Status::ERROR;
        }
        const auto sym_here = sym_in_span[0];
        for (std::size_t w = 0; w < cps_here; ++w) {
            const std::size_t idx_circ = (_walker + w) % _pn_train.size();
            const auto        pn       = _pn_train[idx_circ];
            chip_burst_out[w] =
                std::complex<float>{sym_here.real() * pn.real(), sym_here.imag() * pn.imag()};
        }
        _walker = (_walker + cps_here) % _pn_train.size();
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
