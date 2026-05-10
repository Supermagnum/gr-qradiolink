// SPDX-License-Identifier: GPL-3.0-or-later
// GR4 continuous-phase binary FSK modulator (reference implementation).
// GR3 mod_2fsk uses a larger hier_block (FEC, scrambler, RRC, etc.); match that behaviour when porting DSP from lib/mod_2fsk_impl.cc.
#pragma once

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/meta/reflection.hpp>

#include <complex>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <utility>

namespace gr::qradiolink {

template<typename T>
struct Mod2FSK : gr::Block<Mod2FSK<T>> {
    using Description = gr::Doc<R""(
@brief Binary FSK modulator (complex baseband).

One unpacked bit per input byte (LSB). Emits samples_per_symbol complex outputs per bit at the given sample_rate.
deviation_hz is the peak frequency offset used for the mark tone; space uses the opposite offset.
)"";

    gr::PortIn<std::uint8_t>         in{};
    gr::PortOut<std::complex<T>>     out{};

    T        sample_rate{T{48000}};
    T        deviation_hz{T{2400}};
    std::uint32_t samples_per_symbol{10U};

    GR_MAKE_REFLECTABLE(Mod2FSK, in, out, sample_rate, deviation_hz, samples_per_symbol);

private:
    T _phase{T{0}};

public:
    void start() noexcept { _phase = T{0}; }

    [[nodiscard]] gr::work::Status processBulk(InputSpanLike auto& inSpan, OutputSpanLike auto& outSpan) noexcept {
        const std::size_t n_bits       = inSpan.size();
        const std::size_t sps          = samples_per_symbol == 0U ? 1UZ : static_cast<std::size_t>(samples_per_symbol);
        const std::size_t n_out_needed = n_bits * sps;

        if (n_bits == 0UZ) {
            std::ignore = inSpan.consume(0UZ);
            outSpan.publish(0UZ);
            return gr::work::Status::INSUFFICIENT_INPUT_ITEMS;
        }
        if (outSpan.size() < n_out_needed) {
            std::ignore = inSpan.consume(0UZ);
            outSpan.publish(0UZ);
            return gr::work::Status::INSUFFICIENT_OUTPUT_ITEMS;
        }

        const T        two_pi    = T{2} * std::numbers::pi_v<T>;
        const T        phase_inc = two_pi * deviation_hz / sample_rate;
        std::size_t    out_idx   = 0UZ;

        for (std::size_t i = 0UZ; i < n_bits; ++i) {
            const T inc = (inSpan[i] & 1U) != 0 ? phase_inc : -phase_inc;
            for (std::size_t s = 0UZ; s < sps; ++s) {
                outSpan[out_idx++] = std::polar(T{1}, _phase);
                _phase += inc;
                if (_phase > std::numbers::pi_v<T>) {
                    _phase -= two_pi;
                }
                if (_phase < -std::numbers::pi_v<T>) {
                    _phase += two_pi;
                }
            }
        }

        std::ignore = inSpan.consume(n_bits);
        outSpan.publish(n_out_needed);
        return gr::work::Status::OK;
    }
};

} // namespace gr::qradiolink
