// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DSSSCDMARECEIVERCC_HPP
#define GNURADIO4_QRAD_DSSSCDMARECEIVERCC_HPP

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

GR_REGISTER_BLOCK(gnuradio4::qradiolink::DsssCdmaReceiverCc)

struct DsssCdmaReceiverCc : gr::Block<DsssCdmaReceiverCc, gr::Resampling<1UZ, 1UZ, false>> {
    using Description =
        gr::Doc<"Single-flow CDMA receiver surrogate: correlate chip burst onto local PN replicas (standalone deterministic).">;

    gr::PortIn<std::complex<float>>                    in{};
    gr::PortOut<std::complex<float>>                out{};
    gr::Annotated<std::string, "spread_bits_ascii_binary_cd"> spread_bits_ascii_binary_cd{std::string("1100")};
    gr::Annotated<gr::Size_t, "chips_scaled_per_symbol_uc"> chips_scaled_per_symbol_uc{4U};
    GR_MAKE_REFLECTABLE(DsssCdmaReceiverCc, in, out, spread_bits_ascii_binary_cd, chips_scaled_per_symbol_uc);

private:
    std::vector<std::complex<float>> _pn_train{};
    std::size_t                      _walker{0UZ};

    void regen_wave() noexcept
    {
        std::vector<int> chips_loc{};
        detail::parse_pn_ascii_binary(spread_bits_ascii_binary_cd, chips_loc);
        if (chips_loc.empty()) {
            chips_loc.push_back(1);
        }
        _pn_train.resize(chips_loc.size());
        for (std::size_t v = 0; v < chips_loc.size(); ++v) {
            _pn_train[v] = detail::chip01_to_complex(chips_loc[v]);
        }
        const gr::Size_t in_burst = chips_scaled_per_symbol_uc < gr::Size_t{1U} ? gr::Size_t{1U}
                                                                                         : chips_scaled_per_symbol_uc;
        this->input_chunk_size  = in_burst;
        this->output_chunk_size = gr::Size_t{1U};
        _walker                 = 0UZ;
    }

public:
    void start() noexcept { regen_wave(); }

    void settingsChanged(const gr::property_map&, const gr::property_map& neo) noexcept
    {
        if (neo.contains("spread_bits_ascii_binary_cd") || neo.contains("chips_scaled_per_symbol_uc")) {
            regen_wave();
        }
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& burst_in, std::span<std::complex<float>>& symbol_out_pack) noexcept
    {
        const std::size_t cps_here = chips_scaled_per_symbol_uc < gr::Size_t{1U}
                                         ? 1UZ
                                         : static_cast<std::size_t>(chips_scaled_per_symbol_uc);
        if (burst_in.size() < cps_here || symbol_out_pack.empty() || _pn_train.empty()) {
            return gr::work::Status::ERROR;
        }
        std::complex<float> sym_acc{};
        for (std::size_t ww = 0; ww < cps_here; ++ww) {
            const std::size_t idx_circ = (_walker + ww) % _pn_train.size();
            auto              pn_here    = _pn_train[idx_circ];
            const float denom_re = pn_here.real() <= 1.0e-8F ? 1.0e-8F : pn_here.real();
            const float denom_im = pn_here.imag() <= 1.0e-8F ? 1.0e-8F : pn_here.imag();
            sym_acc +=
                std::complex<float>{burst_in[ww].real() / denom_re, burst_in[ww].imag() / denom_im};
        }
        sym_acc /= static_cast<float>(cps_here);
        symbol_out_pack[0]           = sym_acc;
        _walker = (_walker + cps_here) % _pn_train.size();
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
