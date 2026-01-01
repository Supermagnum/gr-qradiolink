/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_GDSS_DESPREDER_CC_H
#define INCLUDED_QRADIOLINK_GDSS_DESPREDER_CC_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/block.h>
#include <vector>

namespace gr {
namespace qradiolink {

/*!
 * \brief GDSS Despreader/Correlator - despreads GDSS signal and recovers symbols
 * \ingroup qradiolink
 *
 * This block accepts spread spectrum signal at chip rate and despreads it
 * to recover the original symbols at symbol rate using correlation with the
 * Gaussian-distributed spreading sequence.
 *
 * Features:
 * - Gaussian sequence correlation and despreading
 * - Timing synchronization with early-late gate
 * - Code phase acquisition
 * - Lock detection
 * - SNR estimation
 *
 * Input: Spread spectrum signal at chip rate (e.g., 500 ksps)
 * Outputs:
 *  - 0: Despread complex symbols at symbol rate (e.g., 12 ksps)
 *  - 1: Lock status (float, 0.0 = unlocked, 1.0 = locked)
 *  - 2: SNR estimate (float, dB)
 */
class QRADIOLINK_API gdss_despreader_cc : public block
{
public:
    typedef std::shared_ptr<gdss_despreader_cc> sptr;

    enum sync_state {
        STATE_ACQUISITION,  // Searching for code alignment
        STATE_TRACKING,     // Tracking code phase
        STATE_LOCKED        // Locked and despreading
    };

    /*!
     * \brief Make a GDSS despreader block
     *
     * \param spreading_sequence Gaussian spreading sequence (must match transmitter)
     * \param chips_per_symbol Number of chips per symbol (default: 42)
     * \param correlation_threshold Threshold for valid correlation (default: 0.7)
     * \param timing_error_tolerance Samples tolerance for timing errors (default: 2)
     */
    static sptr make(const std::vector<float>& spreading_sequence,
                     int chips_per_symbol = 42,
                     float correlation_threshold = 0.7f,
                     int timing_error_tolerance = 2);

    /*!
     * \brief Set spreading sequence
     *
     * \param spreading_sequence New Gaussian spreading sequence
     */
    virtual void set_spreading_sequence(const std::vector<float>& spreading_sequence);

    /*!
     * \brief Set chips per symbol
     *
     * \param chips_per_symbol New chips per symbol value
     */
    virtual void set_chips_per_symbol(int chips_per_symbol);

    /*!
     * \brief Get current sync state
     *
     * \return Current synchronization state
     */
    virtual sync_state get_sync_state() const;

    /*!
     * \brief Get lock status
     *
     * \return True if locked, false otherwise
     */
    virtual bool is_locked() const;

    /*!
     * \brief Get SNR estimate
     *
     * \return SNR estimate in dB
     */
    virtual float get_snr_estimate() const;

    virtual ~gdss_despreader_cc();

protected:
    gdss_despreader_cc(const std::string& name,
                       gr::io_signature::sptr input_signature,
                       gr::io_signature::sptr output_signature,
                       const std::vector<float>& spreading_sequence,
                       int chips_per_symbol,
                       float correlation_threshold,
                       int timing_error_tolerance)
        : block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_GDSS_DESPREDER_CC_H */

