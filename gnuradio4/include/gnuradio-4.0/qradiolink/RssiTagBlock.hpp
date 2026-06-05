// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_RSSITAGBLOCK_HPP
#define GNURADIO4_QRAD_RSSITAGBLOCK_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/annotated.hpp>

#include <cmath>
#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::RssiTagBlock)

struct RssiTagBlock : gr::Block<RssiTagBlock> {
    using Description = gr::Doc<"Attaches intermittent RSSI side-channel tags proportional to logarithmic smoothed amplitude.">;

    gr::PortIn<std::complex<float>>           in{};
    gr::PortOut<std::complex<float>>        out{};
    gr::Annotated<float, "sample_window"> sample_window_f{300.F};
    gr::Annotated<float, "calibration_db"> calibration_db{0.F};

    GR_MAKE_REFLECTABLE(RssiTagBlock, in, out, sample_window_f, calibration_db);

private:
    gr::Size_t _counter{0UZ};
    float      _pwr_sq_accum{0.F};

public:
    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& ins, std::span<std::complex<float>>& outs) noexcept {
        const std::size_t win_sz = sample_window_f < 1.F ? 1UZ : static_cast<std::size_t>(sample_window_f);
        const auto        n_lim  = std::min(ins.size(), outs.size());
        for (std::size_t k = 0; k < n_lim; ++k) {
            outs[k]                                          = ins[k];
            const float re                                    = ins[k].real();
            const float jd                                    = ins[k].imag();
            const float pwr_sq                                = (re * re + jd * jd) * (re * re + jd * jd);
            _pwr_sq_accum += pwr_sq;
            _counter++;
            if (_counter >= win_sz) {
                const float lvl = std::sqrt(_pwr_sq_accum / static_cast<float>(_counter));
                const float db  = 10.0F * std::log10(lvl + 1.0e-20F) + calibration_db;
                gr::property_map rss{{gr::convert_string_domain(std::string_view("RSSI")), static_cast<float>(db)}};
                publishTag(std::move(rss), k);
                _pwr_sq_accum = 0.F;
                _counter      = static_cast<gr::Size_t>(0U);
            }
        }
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
