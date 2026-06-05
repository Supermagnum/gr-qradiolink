// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DSSSDESPREADERCC_HPP
#define GNURADIO4_QRAD_DSSSDESPREADERCC_HPP

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

GR_REGISTER_BLOCK(gnuradio4::qradiolink::DsssDespreaderCc)

struct DsssDespreaderCc : gr::Block<DsssDespreaderCc, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = gr::Doc<"DSSS despreader: collapse chip train back to one IQ decision per symbol dwell.">;

    gr::PortIn<std::complex<float>>                     in{};
    gr::PortOut<std::complex<float>>                  out{};
    gr::Annotated<std::string, "pn_bits_ascii_binary"> pn_bits_ascii_binary{std::string("1010")};
    gr::Annotated<gr::Size_t, "chips_for_each_symbol"> chips_for_each_symbol{4U};

    GR_MAKE_REFLECTABLE(DsssDespreaderCc, in, out, pn_bits_ascii_binary, chips_for_each_symbol);

private:
    std::vector<std::complex<float>> _pn_waveform{};
    std::size_t                       _chip_cursor{0UZ};

    void rebuild_waveform() noexcept
    {
        std::vector<int> bits_loc{};
        detail::parse_pn_ascii_binary(pn_bits_ascii_binary, bits_loc);
        if (bits_loc.empty()) {
            bits_loc.push_back(1);
        }
        _pn_waveform.resize(bits_loc.size());
        for (std::size_t j = 0; j < bits_loc.size(); ++j) {
            _pn_waveform[j] = detail::chip01_to_complex(bits_loc[j]);
        }
        const gr::Size_t in_sz = chips_for_each_symbol < gr::Size_t{1U} ? gr::Size_t{1U} : chips_for_each_symbol;
        this->input_chunk_size  = in_sz;
        this->output_chunk_size = gr::Size_t{1U};
        _chip_cursor              = 0UZ;
    }

public:
    void start() noexcept { rebuild_waveform(); }

    void settingsChanged(const gr::property_map&, const gr::property_map& neo) noexcept
    {
        if (neo.contains("pn_bits_ascii_binary") || neo.contains("chips_for_each_symbol")) {
            rebuild_waveform();
        }
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& chip_in, std::span<std::complex<float>>& symbol_out) noexcept
    {
        const std::size_t cps_here = chips_for_each_symbol < gr::Size_t{1U} ? 1UZ : static_cast<std::size_t>(chips_for_each_symbol);
        if (chip_in.size() < cps_here || symbol_out.size() < 1UZ || _pn_waveform.empty()) {
            return gr::work::Status::INSUFFICIENT_INPUT_ITEMS;
        }
        std::complex<float> acc{};
        for (std::size_t cx = 0; cx < cps_here; ++cx) {
            const std::size_t idx_here = (_chip_cursor + cx) % _pn_waveform.size();
            acc += chip_in[cx] * std::conj(_pn_waveform[idx_here]);
        }
        acc /= static_cast<float>(cps_here);
        symbol_out[0]     = acc;
        _chip_cursor = (_chip_cursor + cps_here) % _pn_waveform.size();
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
