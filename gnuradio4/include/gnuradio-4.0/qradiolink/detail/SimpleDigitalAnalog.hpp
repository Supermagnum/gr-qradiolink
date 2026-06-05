// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DETAIL_SIMPLEDIGITALANALOG_HPP
#define GNURADIO4_QRAD_DETAIL_SIMPLEDIGITALANALOG_HPP

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <span>

namespace gnuradio4::qradiolink::detail {

inline constexpr float kPi    = std::numbers::pi_v<float>;
inline constexpr float kTwoPi = 2.0F * kPi;

enum class DigitalKind { Fsk2, Fsk4, Fsk8, Cpm4FskLike, GmskLike, Bpsk, Qpsk, SoqpskLike };

enum class AnalogKind { Am, SsbUsb, Nbfm, Wbfm };

struct AnalogModemState {
    float phi_carrier_rad{0.0F};
    float phi_modem_rad{0.0F};
    float fm_prev_re{1.0F};
    float fm_prev_im{0.0F};
};

inline void bumpCarrierPhase(AnalogModemState& st, float carrier_hz, float sample_rate)
{
    if (carrier_hz <= 1.0e-6F) {
        return;
    }
    const float sr = sample_rate <= 1.0F ? 1.0F : sample_rate;
    st.phi_carrier_rad = std::remainder(st.phi_carrier_rad + kTwoPi * carrier_hz / sr, kTwoPi);
}

inline constexpr unsigned bitsPerSymbolDigital(DigitalKind k) noexcept
{
    switch (k) {
    case DigitalKind::Fsk2:
    case DigitalKind::GmskLike:
    case DigitalKind::Bpsk:
        return 1U;
    case DigitalKind::Fsk8:
        return 3U;
    case DigitalKind::Fsk4:
    case DigitalKind::Qpsk:
    case DigitalKind::SoqpskLike:
    case DigitalKind::Cpm4FskLike:
    default:
        return 2U;
    }
}

// Convention: unpacked bytes (GNU Radio bb style) where each byte is 0 or 1.
inline unsigned popUnpackedBits(std::span<const std::uint8_t>& cursor, unsigned need)
{
    unsigned v = 0U;
    for (unsigned n = 0; n < need; ++n) {
        if (cursor.empty()) {
            break;
        }
        const unsigned bit = cursor.front() != 0U ? 1U : 0U;
        cursor                           = cursor.subspan(1UZ);
        v = (v << 1U) | bit;
    }
    return v;
}

inline void pushUnpackedBits(std::span<std::uint8_t>& outs, unsigned value, unsigned n_bits)
{
    for (unsigned sh = n_bits; sh != 0U; --sh) {
        const bool bit_val = (((value >> (sh - 1U)) & 1U) != 0U);
        if (!outs.empty()) {
            outs.front() = bit_val ? std::uint8_t{1} : std::uint8_t{0};
            outs         = outs.subspan(1UZ);
        }
    }
}

inline void digitalModulateWaveform(DigitalKind                 kind,
    std::span<const std::uint8_t> in_bits_unpacked01,
    std::span<std::complex<float>> out,
    std::size_t                     samples_per_symbol,
    float                          sample_rate,
    float                          carrier_hz,
    float                          freq_spacing_hz,
    AnalogModemState&              state)
{
    const std::size_t sps    = samples_per_symbol < 1UZ ? 1UZ : samples_per_symbol;
    const unsigned    bps    = bitsPerSymbolDigital(kind);
    const float       sr     = sample_rate <= 1.0F ? 1.0F : sample_rate;
    const std::size_t n_sym  = out.size() / sps;

    auto angleForFsk = [&](unsigned symbol_index) -> float {
        // symbol_index interpreted as Gray-ish integer 0..M-1
        switch (kind) {
        case DigitalKind::Fsk2:
            return (symbol_index != 0U ? 1.0F : -1.0F) * freq_spacing_hz * kTwoPi / sr;
        case DigitalKind::Fsk4:
        case DigitalKind::Cpm4FskLike:
            return (static_cast<float>(symbol_index)-1.5F) * freq_spacing_hz * kTwoPi / sr;
        case DigitalKind::Fsk8:
            return (static_cast<float>(symbol_index)-3.5F) * (freq_spacing_hz * 0.5F) * kTwoPi / sr;
        default:
            return 0.0F;
        }
    };

    auto bit_src = in_bits_unpacked01;
    for (std::size_t sym = 0; sym < n_sym; ++sym) {
        if (kind == DigitalKind::Bpsk || kind == DigitalKind::Qpsk || kind == DigitalKind::SoqpskLike || kind == DigitalKind::GmskLike) {
            const unsigned bits = popUnpackedBits(bit_src, bps);

            if (kind == DigitalKind::GmskLike && bps == 1U) {
                const float bit_signed = bits != 0U ? 1.0F : -1.0F;
                const float dphi       = (kPi / static_cast<float>(sps)) * bit_signed * (freq_spacing_hz > 0.0F ? 1.0F : -1.0F);
                for (std::size_t j = 0; j < sps; ++j) {
                    state.phi_modem_rad = std::remainder(state.phi_modem_rad + dphi, kTwoPi);
                    bumpCarrierPhase(state, carrier_hz, sr);
                    const float phi = state.phi_carrier_rad + state.phi_modem_rad;
                    out[sym * sps + j] = {std::cos(phi), std::sin(phi)};
                }
                continue;
            }

            float i_lvl = +0.70710677F;
            float q_lvl = +0.70710677F;
            if (kind == DigitalKind::Bpsk) {
                i_lvl = bits != 0U ? 1.0F : -1.0F;
                q_lvl = 0.0F;
            }
            if (kind == DigitalKind::Qpsk || kind == DigitalKind::SoqpskLike) {
                switch (static_cast<int>(bits & 3U)) {
                case 0:
                    i_lvl = -1.0F / 1.41421356F;
                    q_lvl = -1.0F / 1.41421356F;
                    break;
                case 1:
                    i_lvl = +1.0F / 1.41421356F;
                    q_lvl = -1.0F / 1.41421356F;
                    break;
                case 2:
                    i_lvl = -1.0F / 1.41421356F;
                    q_lvl = +1.0F / 1.41421356F;
                    break;
                default:
                    i_lvl = +1.0F / 1.41421356F;
                    q_lvl = +1.0F / 1.41421356F;
                    break;
                }
            }
            const float iq_rot = kind == DigitalKind::SoqpskLike ? kPi / 4.0F : 0.0F;
            for (std::size_t j = 0; j < sps; ++j) {
                bumpCarrierPhase(state, carrier_hz, sr);
                const float ph_carrier = state.phi_carrier_rad;
                const float c          = std::cos(ph_carrier + iq_rot);
                const float ssn        = std::sin(ph_carrier + iq_rot);
                out[sym * sps + j] = std::complex<float>{i_lvl * c - q_lvl * ssn, i_lvl * ssn + q_lvl * c};
            }
            continue;
        }

        const unsigned sym_ix = popUnpackedBits(bit_src, bps);

        const float dphi_nom = angleForFsk(sym_ix);
        if (kind == DigitalKind::Cpm4FskLike) {
            const float smoothed_dphi =
                sym_ix == 3U ? dphi_nom * 0.7F : sym_ix == 0U ? dphi_nom * 0.7F : dphi_nom;
            for (std::size_t j = 0; j < sps; ++j) {
                state.phi_modem_rad = std::remainder(state.phi_modem_rad + smoothed_dphi, kTwoPi);
                bumpCarrierPhase(state, carrier_hz, sr);
                const float phi = state.phi_carrier_rad + state.phi_modem_rad;
                out[sym * sps + j] = {std::cos(phi), std::sin(phi)};
            }
            continue;
        }

        const float phi_inc = (kind == DigitalKind::Fsk2 || kind == DigitalKind::Fsk4 || kind == DigitalKind::Fsk8) ? dphi_nom : 0.0F;

        // frequency keying as constant tone per symbol
        for (std::size_t j = 0; j < sps; ++j) {
            state.phi_modem_rad = std::remainder(state.phi_modem_rad + phi_inc, kTwoPi);
            bumpCarrierPhase(state, carrier_hz, sr);
            const float phi = state.phi_carrier_rad + state.phi_modem_rad;
            out[sym * sps + j] = {std::cos(phi), std::sin(phi)};
        }
    }
}

inline float fskIntegratedPhaseDelta(std::span<const std::complex<float>> chips)
{
    if (chips.size() < 2UZ) {
        return 0.0F;
    }
    std::complex<float> acc = std::complex<float>{0.0F, 0.0F};
    for (std::size_t t = 1; t < chips.size(); ++t) {
        acc += chips[t] * std::conj(chips[t - 1UZ]);
    }
    return std::atan2(acc.imag(), acc.real());
}

inline void digitalDemodulateHardBits(DigitalKind                         kind,
    std::span<const std::complex<float>> waveform,
    std::span<std::uint8_t>&               out_bits_mut,
    std::size_t                           samples_per_symbol,
    float                                 freq_spacing_hz)
{
    const std::size_t          sps  = samples_per_symbol < 1UZ ? 1UZ : samples_per_symbol;
    const unsigned             bps  = bitsPerSymbolDigital(kind);
    const std::size_t          nsym = waveform.size() / sps;
    std::span<std::uint8_t> ob = out_bits_mut;

    for (std::size_t sym = 0; sym < nsym; ++sym) {
        if (ob.size() < bps) {
            break;
        }
        const auto chunk = waveform.subspan(sym * sps, sps);

        if (kind == DigitalKind::Fsk2) {
            const float dtheta = fskIntegratedPhaseDelta(chunk);
            pushUnpackedBits(ob, dtheta > 0.0F ? 1U : 0U, 1U);
            continue;
        }
        if (kind == DigitalKind::GmskLike) {
            const float dtheta = fskIntegratedPhaseDelta(chunk);
            pushUnpackedBits(ob, dtheta > 0.0F ? 1U : 0U, 1U);
            continue;
        }
        if (kind == DigitalKind::Fsk4 || kind == DigitalKind::Cpm4FskLike) {
            const float angles[4] = {-1.5F * freq_spacing_hz, -0.5F * freq_spacing_hz, +0.5F * freq_spacing_hz, +1.5F * freq_spacing_hz};
            float best            = std::numeric_limits<float>::infinity();
            int   best_ix         = 0;
            const float slope     = fskIntegratedPhaseDelta(chunk);
            for (int ix = 0; ix < 4; ++ix) {
                const float e = std::abs(slope - angles[ix] * 1.0e-3F); // heuristic scale-independent
                (void)e;
                // Compare energy at nominal tone using goertzel-lite single-bin DFT approximation:
                float I = 0.0F, Q = 0.0F;
                for (std::size_t n = 0; n < chunk.size(); ++n) {
                    const float psi = angles[ix] * kTwoPi * static_cast<float>(n) / static_cast<float>(sps <= 1UZ ? 1UZ : sps);
                    I += chunk[n].real() * std::cos(psi) + chunk[n].imag() * std::sin(psi);
                    Q += -chunk[n].real() * std::sin(psi) + chunk[n].imag() * std::cos(psi);
                }
                float met = -(I * I + Q * Q);
                if (met < best) {
                    best    = met;
                    best_ix = ix;
                }
            }
            pushUnpackedBits(ob, static_cast<unsigned>(best_ix & 3), 2U);
            continue;
        }
        if (kind == DigitalKind::Fsk8) {
            const float slopes[8] = {-3.5F, -2.5F, -1.5F, -0.5F, +0.5F, +1.5F, +2.5F, +3.5F};
            float       best_met  = std::numeric_limits<float>::infinity();
            int         best_ix   = 0;
            const float slope     = fskIntegratedPhaseDelta(chunk);
            for (int ix = 0; ix < 8; ++ix) {
                const float e = std::abs(slope - slopes[ix] * 1e-3F * freq_spacing_hz);
                if (e < best_met) {
                    best_met = e;
                    best_ix  = ix;
                }
            }
            pushUnpackedBits(ob, static_cast<unsigned>(best_ix & 7), 3U);
            continue;
        }
        if (kind == DigitalKind::Bpsk) {
            std::complex<float> z{};
            for (const auto c : chunk) {
                z += c;
            }
            pushUnpackedBits(ob, z.real() > 0.0F ? 1U : 0U, 1U);
            continue;
        }
        if (kind == DigitalKind::Qpsk || kind == DigitalKind::SoqpskLike) {
            std::complex<float> z{};
            for (const auto c : chunk) {
                z += c;
            }
            if (kind == DigitalKind::SoqpskLike) {
                z *= std::complex<float>{std::cos(-kPi / 4.0F), std::sin(-kPi / 4.0F)};
            }
            const unsigned b0       = z.imag() > 0.0F ? 1U : 0U;
            const unsigned b1       = z.real() > 0.0F ? 1U : 0U;
            const unsigned dibit_ix = (b0 << 1U) | b1;
            pushUnpackedBits(ob, dibit_ix, 2U);
            continue;
        }

        std::ignore = freq_spacing_hz;
        break;
    }
}

inline void analogTransmitSamples(AnalogKind                    kind,
    std::span<const float>               audio_bb,
    std::span<std::complex<float>>       rf,
    float                                sample_rate,
    float                                carrier_hz,
    float                                modulation_index,
    float                                fm_peak_deviation_hz,
    AnalogModemState&                    state)
{
    const float sr = sample_rate <= 1.0F ? 1.0F : sample_rate;
    float       h0 = 0.0F, h1 = 0.0F, h2 = 0.0F;
    const float clamp_m = std::clamp(modulation_index, 1.0e-4F, 1.5F);

    for (std::size_t i = 0; i < rf.size(); ++i) {
        const float a = std::clamp(i < audio_bb.size() ? audio_bb[i] : 0.0F, -1.0F, 1.0F);
        h0 = h1;
        h1 = h2;
        h2 = a;
        bumpCarrierPhase(state, carrier_hz, sr);
        const float phi = state.phi_carrier_rad;

        if (kind == AnalogKind::Am) {
            const float env = std::clamp(1.0F + clamp_m * h2, 0.001F, 10.0F);
            rf[i]           = env * std::complex<float>{std::cos(phi), std::sin(phi)};
        } else if (kind == AnalogKind::SsbUsb) {
            const float q = (h2 - h0) * 7.65F;
            rf[i]         = std::complex<float>{h2 * std::cos(phi) - q * std::sin(phi), h2 * std::sin(phi) + q * std::cos(phi)};
        } else {
            const float kd         = fm_peak_deviation_hz <= 1.0e-4F ? 200.0F : fm_peak_deviation_hz;
            const float scale_wave = kind == AnalogKind::Wbfm ? 25.5F / sr : kd / sr;
            state.phi_modem_rad =
                std::remainder(state.phi_modem_rad + kTwoPi * a * scale_wave + (kind == AnalogKind::Wbfm ? 0.015F : 0.0F), kTwoPi);
            const float psi = phi + state.phi_modem_rad;
            rf[i]           = {std::cos(psi), std::sin(psi)};
        }
    }
}

inline void analogDemodSamples(AnalogKind                           kind,
    std::span<const std::complex<float>> rf_in,
    std::span<float>                     audio_bb,
    float                               sample_rate,
    float                               carrier_hz,
    float                               /*modulation_index*/,
    float                               fm_peak_deviation_hz,
    AnalogModemState&                   state)
{
    const float sr = sample_rate <= 1.0F ? 1.0F : sample_rate;

    for (std::size_t i = 0; i < audio_bb.size() && i < rf_in.size(); ++i) {
        std::complex<float> mixed = rf_in[i];
        if (carrier_hz > 1.0e-4F) {
            const float        ph_dem = std::remainder(-kTwoPi * carrier_hz / sr * static_cast<float>(i), kTwoPi);
            const std::complex<float> lo{std::cos(ph_dem), std::sin(ph_dem)};
            mixed *= lo;
            bumpCarrierPhase(state, carrier_hz, sr); // bookkeeping only
        }
        if (kind == AnalogKind::Am) {
            float m = std::abs(mixed) - 0.995F * 1.035F / (1.035F); // heuristic DC removal
            audio_bb[i] = std::clamp(m * 1.82F, -1.0F, 1.0F);
        } else if (kind == AnalogKind::SsbUsb) {
            audio_bb[i] =
                std::clamp(mixed.real() * 1.25F + mixed.imag() * 6.41F / (1.41421356F), -1.0F, 1.0F);
        } else {
            const std::complex<float> zm1{state.fm_prev_re, state.fm_prev_im};
            const std::complex<float> prod = rf_in[i] * std::conj(zm1);
            state.fm_prev_re                 = rf_in[i].real();
            state.fm_prev_im                 = rf_in[i].imag();
            float dem                        = std::atan2(prod.imag(), prod.real());
            const float kd                   = fm_peak_deviation_hz <= 1.0e-6F ? 650.0F : fm_peak_deviation_hz * (kind == AnalogKind::Wbfm ? 0.085F / sr : 8.52F / sr);
            audio_bb[i]                      = std::clamp(dem / kd, -1.0F, 1.0F);
        }
    }
}

} // namespace gnuradio4::qradiolink::detail

#endif
