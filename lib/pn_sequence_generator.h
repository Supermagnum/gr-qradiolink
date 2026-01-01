/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_PN_SEQUENCE_GENERATOR_H
#define INCLUDED_QRADIOLINK_PN_SEQUENCE_GENERATOR_H

#include <vector>
#include <cstdint>

namespace gr {
namespace qradiolink {

/*!
 * \brief PN Sequence Generator - generates m-sequences, Gold codes, and Kasami sequences
 * \ingroup qradiolink
 *
 * This utility class provides functions for generating standard PN sequences
 * used in DSSS communication systems.
 */
class pn_sequence_generator
{
public:
    /*!
     * \brief Generate an m-sequence (maximal length sequence)
     *
     * \param taps Polynomial tap positions (e.g., [7,1] for 7-bit LFSR)
     * \param length Sequence length (2^n - 1, e.g., 127, 511, 1023)
     * \return Vector of +1/-1 values representing the sequence
     */
    static std::vector<int> generate_msequence(const std::vector<int>& taps, int length);

    /*!
     * \brief Generate a Gold code from two preferred m-sequences
     *
     * \param poly1_taps First polynomial tap positions
     * \param poly2_taps Second polynomial tap positions
     * \param code_number Which code from the Gold code family (0 to length-1)
     * \param length Sequence length
     * \return Vector of +1/-1 values representing the Gold code
     */
    static std::vector<int> generate_gold_code(const std::vector<int>& poly1_taps,
                                                const std::vector<int>& poly2_taps,
                                                int code_number,
                                                int length);

    /*!
     * \brief Validate sequence properties (autocorrelation, balance)
     *
     * \param sequence Sequence to validate
     * \return True if sequence has good properties
     */
    static bool validate_sequence(const std::vector<int>& sequence);

    /*!
     * \brief Compute autocorrelation of a sequence
     *
     * \param sequence Input sequence
     * \param max_shift Maximum shift to compute
     * \return Autocorrelation values for shifts 0 to max_shift
     */
    static std::vector<float> sequence_autocorrelation(const std::vector<int>& sequence, int max_shift);

    /*!
     * \brief Generate standard 127-chip m-sequence (polynomial [7,1])
     */
    static std::vector<int> generate_msequence_127();

    /*!
     * \brief Generate standard 511-chip m-sequence (polynomial [9,4])
     */
    static std::vector<int> generate_msequence_511();

    /*!
     * \brief Generate standard 1023-chip m-sequence (polynomial [10,3])
     */
    static std::vector<int> generate_msequence_1023();

    /*!
     * \brief Generate standard 255-chip m-sequence (polynomial [8,4,3,2])
     */
    static std::vector<int> generate_msequence_255();

    /*!
     * \brief Generate standard 2047-chip m-sequence (polynomial [11,2])
     */
    static std::vector<int> generate_msequence_2047();

    /*!
     * \brief Generate Gold code family for multi-user operation
     *
     * \param length Sequence length (127, 255, 511, 1023, or 2047)
     * \param num_codes Number of codes to generate
     * \return Vector of code sequences
     */
    static std::vector<std::vector<int>> generate_gold_code_family(int length, int num_codes);

private:
    /*!
     * \brief Internal LFSR implementation for m-sequence generation
     */
    static std::vector<int> lfsr_sequence(const std::vector<int>& taps, int length, uint32_t initial_state = 1);
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_PN_SEQUENCE_GENERATOR_H */

