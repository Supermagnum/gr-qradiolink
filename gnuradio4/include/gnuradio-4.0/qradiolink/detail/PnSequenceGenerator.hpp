// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DETAIL_PNSEQUENCEGENERATOR_HPP
#define GNURADIO4_QRAD_DETAIL_PNSEQUENCEGENERATOR_HPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace gnuradio4::qradiolink::detail {

inline std::vector<int> pnLfsrSequence(const std::vector<int>& taps, int length, std::uint32_t initial_state)
{
    if (taps.empty() || length <= 0) {
        return {};
    }

    const int        n_regs = taps[0];
    std::uint32_t    state  = initial_state;
    std::vector<int> sequence;
    sequence.reserve(static_cast<std::size_t>(length));

    for (int i = 0; i < length; ++i) {
        sequence.push_back(static_cast<int>(state & 1U));
        int feedback = 0;
        for (size_t j = 0; j < taps.size(); ++j) {
            const int tp = taps[j];
            if (tp > 0 && tp <= n_regs) {
                feedback ^= static_cast<int>((state >> (n_regs - tp)) & 1U);
            }
        }
        state = (state >> 1U) | (static_cast<std::uint32_t>(feedback) << (n_regs - 1));
    }
    return sequence;
}

inline std::vector<int> pnMsequenceBipolar(const std::vector<int>& taps, int length)
{
    std::vector<int> binary_seq = pnLfsrSequence(taps, length, 1U);
    std::vector<int> bipolar;
    bipolar.reserve(binary_seq.size());
    for (int bit : binary_seq) {
        bipolar.push_back(bit == 1 ? 1 : -1);
    }
    return bipolar;
}

inline std::vector<int> pnGoldCodeBipolar(const std::vector<int>& poly1_taps, const std::vector<int>& poly2_taps, int code_number, int length)
{
    std::vector<int> seq1 = pnMsequenceBipolar(poly1_taps, length);
    std::vector<int> seq2 = pnMsequenceBipolar(poly2_taps, length);
    if (seq1.size() != static_cast<std::size_t>(length) || seq2.size() != static_cast<std::size_t>(length)) {
        return {};
    }
    std::vector<int> seq2_shifted(static_cast<std::size_t>(length));
    for (int i = 0; i < length; ++i) {
        seq2_shifted[static_cast<std::size_t>(i)] = seq2[(static_cast<std::size_t>(i) + static_cast<std::size_t>(code_number)) % static_cast<std::size_t>(length)];
    }
    std::vector<int> gold;
    gold.reserve(static_cast<std::size_t>(length));
    for (int i = 0; i < length; ++i) {
        gold.push_back(seq1[static_cast<std::size_t>(i)] * seq2_shifted[static_cast<std::size_t>(i)]);
    }
    return gold;
}

inline std::vector<float> pnSequenceAutocorrelation(const std::vector<int>& sequence, int max_shift)
{
    std::vector<float> autocorr;
    const int          length_i = static_cast<int>(sequence.size());
    autocorr.reserve(static_cast<std::size_t>(max_shift + 1));
    for (int shift = 0; shift <= max_shift; ++shift) {
        float sum = 0.0F;
        for (int i = 0; i < length_i; ++i) {
            const int j = (i + shift) % length_i;
            sum += static_cast<float>(sequence[static_cast<std::size_t>(i)] * sequence[static_cast<std::size_t>(j)]);
        }
        autocorr.push_back(sum);
    }
    return autocorr;
}

inline bool pnValidateSequenceRoughly(const std::vector<int>& sequence)
{
    if (sequence.empty()) {
        return false;
    }
    const int balance = std::abs(std::accumulate(sequence.begin(), sequence.end(), 0));
    if (balance > static_cast<int>(sequence.size()) / 2) {
        return false;
    }
    const std::vector<float> autocorr = pnSequenceAutocorrelation(sequence, std::min(10, static_cast<int>(sequence.size())));
    if (autocorr.empty() || autocorr[0] < static_cast<float>(sequence.size()) * 0.9F) {
        return false;
    }
    return true;
}

inline std::vector<int> pnMsequence127()
{
    return pnMsequenceBipolar({7, 1}, 127);
}

inline std::vector<int> pnMsequence511()
{
    return pnMsequenceBipolar({9, 4}, 511);
}

inline std::vector<int> pnMsequence1023()
{
    return pnMsequenceBipolar({10, 3}, 1023);
}

} // namespace gnuradio4::qradiolink::detail

#endif // GNURADIO4_QRAD_DETAIL_PNSEQUENCEGENERATOR_HPP
