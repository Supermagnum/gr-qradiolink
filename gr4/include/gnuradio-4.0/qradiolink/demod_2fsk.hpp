// SPDX-License-Identifier: GPL-3.0-or-later
// Minimal non-coherent 2FSK slicer: integrate phase derivative over samples_per_symbol.
// GR3 demod_2fsk is far richer (FLL, dual path, FEC); replace processBulk with logic from lib/demod_2fsk_impl.cc when porting.
#pragma once

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/meta/reflection.hpp>

#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <utility>

namespace gr::qradiolink {

template<typename T>
struct Demod2FSK : gr::Block<Demod2FSK<T>> {
    using Description = gr::Doc<"Minimal 2FSK demod placeholder — port DSP from GR3 demod_2fsk_impl.cc.">;

    gr::PortIn<std::complex<T>> in{};
    gr::PortOut<std::uint8_t>    out{};

    T             sample_rate{T{48000}};
    T             deviation_hz{T{2400}};
    std::uint32_t samples_per_symbol{10U};

    GR_MAKE_REFLECTABLE(Demod2FSK, in, out, sample_rate, deviation_hz, samples_per_symbol);

private:
    std::complex<T> _prev{T{1}, T{0}};
    std::size_t     _idx{0UZ};
    T               _acc{T{0}};

public:
    void start() noexcept {
        _prev = {T{1}, T{0}};
        _idx  = 0UZ;
        _acc  = T{0};
    }

    [[nodiscard]] gr::work::Status processBulk(InputSpanLike auto& inSpan, OutputSpanLike auto& outSpan) noexcept {
        const std::size_t sps = samples_per_symbol == 0U ? 1UZ : static_cast<std::size_t>(samples_per_symbol);
        if (inSpan.size() == 0UZ || outSpan.size() == 0UZ) {
            std::ignore = inSpan.consume(0UZ);
            outSpan.publish(0UZ);
            return gr::work::Status::INSUFFICIENT_INPUT_ITEMS;
        }

        std::size_t in_i = 0UZ;
        std::size_t out_i = 0UZ;

        while (in_i < inSpan.size() && out_i < outSpan.size()) {
            const std::complex<T> z = inSpan[in_i];
            const T               p = std::arg(z * std::conj(_prev));
            _prev                   = z;
            _acc += p;
            ++in_i;
            ++_idx;
            if (_idx >= sps) {
                outSpan[out_i++] = (_acc > T{0}) ? std::uint8_t{1} : std::uint8_t{0};
                _acc = T{0};
                _idx = 0UZ;
            }
        }

        std::ignore = inSpan.consume(in_i);
        outSpan.publish(out_i);
        return gr::work::Status::OK;
    }
};

} // namespace gr::qradiolink
