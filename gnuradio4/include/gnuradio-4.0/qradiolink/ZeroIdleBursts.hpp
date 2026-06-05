// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_ZEROIDLEBURSTS_HPP
#define GNURADIO4_QRAD_ZEROIDLEBURSTS_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/annotated.hpp>

#include <complex>
#include <span>

namespace gnuradio4::qradiolink {

GR_REGISTER_BLOCK(gnuradio4::qradiolink::ZeroIdleBursts)

struct ZeroIdleBursts : gr::Block<ZeroIdleBursts> {
    using Description = gr::Doc<"Zeroes bursts when qa requests idle countdown via annotated counter.">;

    gr::PortIn<std::complex<float>>            in{};
    gr::PortOut<std::complex<float>>         out{};
    gr::Annotated<gr::Size_t, "requested_idle_zeros"> requested_idle_zeros{0U};

    GR_MAKE_REFLECTABLE(ZeroIdleBursts, in, out, requested_idle_zeros);

private:
    gr::Size_t _samples_remaining_idle{0U};

public:
    void settingsChanged(const gr::property_map& /*old_map*/, const gr::property_map& new_map) noexcept {
        if (!new_map.contains("requested_idle_zeros")) {
            return;
        }
        _samples_remaining_idle += requested_idle_zeros;
        requested_idle_zeros               = static_cast<gr::Size_t>(0U); // consumes request
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const std::complex<float>>& ins, std::span<std::complex<float>>& outs) noexcept {
        const std::size_t nmin = std::min(ins.size(), outs.size());
        for (std::size_t ix = 0; ix < nmin; ++ix) {
            if (_samples_remaining_idle > static_cast<gr::Size_t>(0U)) {
                outs[ix]                           = {};
                _samples_remaining_idle -= static_cast<gr::Size_t>(1U);
            } else {
                outs[ix] = ins[ix];
            }
        }
        return gr::work::Status::OK;
    }
};

} // namespace gnuradio4::qradiolink

#endif
