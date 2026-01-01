/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "pn_sequence_generator.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace gr {
namespace qradiolink {

std::vector<int> pn_sequence_generator::lfsr_sequence(const std::vector<int>& taps, int length, uint32_t initial_state)
{
    if (taps.empty() || length <= 0) {
        return std::vector<int>();
    }

    int n = taps[0]; // First tap is the register length
    uint32_t state = initial_state;
    std::vector<int> sequence;
    sequence.reserve(length);

    for (int i = 0; i < length; i++) {
        // Output the LSB
        sequence.push_back(state & 1);

        // Compute feedback (XOR of tap positions)
        int feedback = 0;
        for (size_t j = 0; j < taps.size(); j++) {
            if (taps[j] > 0 && taps[j] <= n) {
                feedback ^= (state >> (n - taps[j])) & 1;
            }
        }

        // Shift and insert feedback
        state = (state >> 1) | (feedback << (n - 1));
    }

    return sequence;
}

std::vector<int> pn_sequence_generator::generate_msequence(const std::vector<int>& taps, int length)
{
    std::vector<int> binary_seq = lfsr_sequence(taps, length);
    
    // Convert 0/1 to +1/-1
    std::vector<int> result;
    result.reserve(binary_seq.size());
    for (int bit : binary_seq) {
        result.push_back(bit == 1 ? 1 : -1);
    }

    return result;
}

std::vector<int> pn_sequence_generator::generate_gold_code(const std::vector<int>& poly1_taps,
                                                           const std::vector<int>& poly2_taps,
                                                           int code_number,
                                                           int length)
{
    // Generate the two m-sequences
    std::vector<int> seq1 = generate_msequence(poly1_taps, length);
    std::vector<int> seq2 = generate_msequence(poly2_taps, length);

    // Circularly shift seq2 by code_number positions
    std::vector<int> seq2_shifted(length);
    for (int i = 0; i < length; i++) {
        seq2_shifted[i] = seq2[(i + code_number) % length];
    }

    // XOR the two sequences (multiply in +1/-1 representation)
    std::vector<int> gold_code;
    gold_code.reserve(length);
    for (int i = 0; i < length; i++) {
        gold_code.push_back(seq1[i] * seq2_shifted[i]);
    }

    return gold_code;
}

bool pn_sequence_generator::validate_sequence(const std::vector<int>& sequence)
{
    if (sequence.empty()) {
        return false;
    }

    // Check balance (should have roughly equal +1 and -1)
    int sum = std::accumulate(sequence.begin(), sequence.end(), 0);
    int balance = std::abs(sum);
    
    // For m-sequences, balance should be 1 (one more of one value)
    // For Gold codes, balance can vary but should be reasonable
    if (balance > static_cast<int>(sequence.size()) / 2) {
        return false;
    }

    // Check autocorrelation peak
    std::vector<float> autocorr = sequence_autocorrelation(sequence, std::min(10, (int)sequence.size()));
    if (autocorr.empty() || autocorr[0] < sequence.size() * 0.9f) {
        return false;
    }

    return true;
}

std::vector<float> pn_sequence_generator::sequence_autocorrelation(const std::vector<int>& sequence, int max_shift)
{
    std::vector<float> autocorr;
    autocorr.reserve(max_shift + 1);

    int length = sequence.size();
    for (int shift = 0; shift <= max_shift; shift++) {
        float sum = 0.0f;
        for (int i = 0; i < length; i++) {
            int j = (i + shift) % length;
            sum += sequence[i] * sequence[j];
        }
        autocorr.push_back(sum);
    }

    return autocorr;
}

std::vector<int> pn_sequence_generator::generate_msequence_127()
{
    // Polynomial: x^7 + x + 1 (taps: [7, 1])
    std::vector<int> taps = {7, 1};
    return generate_msequence(taps, 127);
}

std::vector<int> pn_sequence_generator::generate_msequence_511()
{
    // Polynomial: x^9 + x^4 + 1 (taps: [9, 4])
    std::vector<int> taps = {9, 4};
    return generate_msequence(taps, 511);
}

std::vector<int> pn_sequence_generator::generate_msequence_1023()
{
    // Polynomial: x^10 + x^3 + 1 (taps: [10, 3])
    std::vector<int> taps = {10, 3};
    return generate_msequence(taps, 1023);
}

std::vector<int> pn_sequence_generator::generate_msequence_255()
{
    // Polynomial: x^8 + x^4 + x^3 + x^2 + 1 (taps: [8, 4, 3, 2])
    std::vector<int> taps = {8, 4, 3, 2};
    return generate_msequence(taps, 255);
}

std::vector<int> pn_sequence_generator::generate_msequence_2047()
{
    // Polynomial: x^11 + x^2 + 1 (taps: [11, 2])
    std::vector<int> taps = {11, 2};
    return generate_msequence(taps, 2047);
}

std::vector<std::vector<int>> pn_sequence_generator::generate_gold_code_family(int length, int num_codes)
{
    std::vector<std::vector<int>> family;
    family.reserve(num_codes);

    // Select preferred pairs based on length
    std::vector<int> poly1_taps, poly2_taps;

    if (length == 127) {
        // Preferred pair for 127: [7,1] and [7,3]
        poly1_taps = {7, 1};
        poly2_taps = {7, 3};
    } else if (length == 255) {
        // Preferred pair for 255: [8,4,3,2] and [8,6,5,3]
        poly1_taps = {8, 4, 3, 2};
        poly2_taps = {8, 6, 5, 3};
    } else if (length == 511) {
        // Preferred pair for 511: [9,4] and [9,6]
        poly1_taps = {9, 4};
        poly2_taps = {9, 6};
    } else if (length == 1023) {
        // Preferred pair for 1023: [10,3] and [10,8]
        poly1_taps = {10, 3};
        poly2_taps = {10, 8};
    } else if (length == 2047) {
        // Preferred pair for 2047: [11,2] and [11,8]
        poly1_taps = {11, 2};
        poly2_taps = {11, 8};
    } else {
        // Default to m-sequence if no preferred pair defined
        if (length == 127) {
            for (int i = 0; i < num_codes; i++) {
                family.push_back(generate_msequence_127());
            }
        } else if (length == 255) {
            for (int i = 0; i < num_codes; i++) {
                family.push_back(generate_msequence_255());
            }
        } else if (length == 511) {
            for (int i = 0; i < num_codes; i++) {
                family.push_back(generate_msequence_511());
            }
        } else if (length == 1023) {
            for (int i = 0; i < num_codes; i++) {
                family.push_back(generate_msequence_1023());
            }
        } else if (length == 2047) {
            for (int i = 0; i < num_codes; i++) {
                family.push_back(generate_msequence_2047());
            }
        }
        return family;
    }

    // Generate Gold codes
    for (int i = 0; i < num_codes; i++) {
        family.push_back(generate_gold_code(poly1_taps, poly2_taps, i, length));
    }

    return family;
}

} // namespace qradiolink
} // namespace gr

