// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DETAIL_DSSSGDSSTOOLS_HPP
#define GNURADIO4_QRAD_DETAIL_DSSSGDSSTOOLS_HPP

#include <algorithm>
#include <cctype>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <string>
#include <vector>

namespace gnuradio4::qradiolink::detail {

inline bool parse_pn_ascii_binary(const std::string& ascii, std::vector<int>& chips01)
{
    chips01.clear();
    for (char ch : ascii) {
        if (ch == '0' || ch == ' ') {
            if (ch == ' ') {
                continue;
            }
            chips01.push_back(0);
        } else if (ch == '1') {
            chips01.push_back(1);
        } else if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
            continue;
        } else {
            chips01.clear();
            return false;
        }
    }
    return !chips01.empty();
}

inline std::complex<float> chip01_to_complex(int chip01)
{
    return chip01 != 0 ? std::complex<float>{1.0F, 0.0F} : std::complex<float>{-1.0F, 0.0F};
}

inline std::complex<float> corr_desymbolize(std::span<const std::complex<float>> observation,
    std::span<const int>                                                                pn01)
{
    if (pn01.empty()) {
        return {0.F, 0.F};
    }
    std::complex<float> correlation{};
    const std::size_t   usable = std::min(observation.size(), pn01.size());
    if (usable == 0UZ) {
        return {0.F, 0.F};
    }
    for (std::size_t ci = 0; ci < usable; ++ci) {
        correlation += observation[ci] * std::conj(chip01_to_complex(pn01[ci]));
    }
    correlation /= static_cast<float>(pn01.size());
    return correlation;
}

inline void gdssGaussianSequence(std::mt19937_64& rng, float variance_hint, std::vector<std::complex<float>>& chips)
{
    std::normal_distribution<float> rnd{0.0F, std::sqrt(variance_hint > 1e-12F ? variance_hint : 1.0F)};
    for (auto& ck : chips) {
        ck = {rnd(rng), rnd(rng)};
    }
}

} // namespace gnuradio4::qradiolink::detail

#endif
