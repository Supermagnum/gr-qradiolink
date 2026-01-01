/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_GDSS_SPREADER_CC_H
#define INCLUDED_QRADIOLINK_GDSS_SPREADER_CC_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_interpolator.h>
#include <vector>

namespace gr {
namespace qradiolink {

/*!
 * \brief GDSS Spreader - spreads complex symbols using Gaussian-distributed sequence
 * \ingroup qradiolink
 *
 * This block implements Gaussian-Distributed Spread-Spectrum (GDSS) spreading.
 * Unlike traditional DSSS which uses binary PN sequences (+1/-1), GDSS uses
 * Gaussian-distributed spreading sequences, providing better spectral properties
 * and improved interference rejection.
 *
 * Features:
 * - Gaussian-distributed spreading sequence (not binary)
 * - Configurable sequence length
 * - Configurable chips per symbol
 * - Better spectral shaping than binary PN sequences
 *
 * Input: Complex symbols at symbol rate
 * Output: Spread spectrum signal at chip rate
 *
 * The spreading sequence is generated from a Gaussian distribution with
 * zero mean and configurable variance, providing smoother spectral characteristics.
 */
class QRADIOLINK_API gdss_spreader_cc : public sync_interpolator
{
public:
    typedef std::shared_ptr<gdss_spreader_cc> sptr;

    /*!
     * \brief Make a GDSS spreader block
     *
     * \param sequence_length Length of the Gaussian spreading sequence
     * \param chips_per_symbol Number of chips per symbol (default: 42)
     * \param variance Variance of the Gaussian distribution (default: 1.0)
     * \param seed Random seed for sequence generation (default: 0, uses time)
     */
    static sptr make(int sequence_length,
                     int chips_per_symbol = 42,
                     float variance = 1.0f,
                     unsigned int seed = 0);

    /*!
     * \brief Set spreading sequence (allows runtime updates)
     *
     * \param sequence New Gaussian spreading sequence
     */
    virtual void set_spreading_sequence(const std::vector<float>& sequence);

    /*!
     * \brief Set chips per symbol
     *
     * \param chips_per_symbol New chips per symbol value
     */
    virtual void set_chips_per_symbol(int chips_per_symbol);

    /*!
     * \brief Regenerate spreading sequence with new parameters
     *
     * \param variance New variance for Gaussian distribution
     * \param seed New random seed
     */
    virtual void regenerate_sequence(float variance, unsigned int seed);

    virtual ~gdss_spreader_cc();

protected:
    gdss_spreader_cc(const std::string& name,
                     gr::io_signature::sptr input_signature,
                     gr::io_signature::sptr output_signature,
                     int sequence_length,
                     int chips_per_symbol,
                     float variance,
                     unsigned int seed)
        : sync_interpolator(name, input_signature, output_signature, chips_per_symbol)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_GDSS_SPREADER_CC_H */

