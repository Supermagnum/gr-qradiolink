/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DSSS_CDMA_RECEIVER_CC_H
#define INCLUDED_QRADIOLINK_DSSS_CDMA_RECEIVER_CC_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/block.h>
#include <vector>

namespace gr {
namespace qradiolink {

/*!
 * \brief DSSS-CDMA Receiver - despreads CDMA signal for a specific user
 * \ingroup qradiolink
 *
 * This block implements a Direct Sequence Spread Spectrum Code Division Multiple Access
 * (DSSS-CDMA) receiver that accepts a combined CDMA signal and despreads it to recover
 * a specific user's signal.
 *
 * Features:
 * - Despreading for a specific user using their spreading code
 * - Configurable spreading factors: 32, 64, 128, 256, 512 chips per symbol
 * - Timing synchronization with early-late gate
 * - Code phase acquisition
 * - Lock detection
 * - SNR estimation
 * - Multi-user interference rejection
 *
 * Input: Combined CDMA signal at chip rate
 * Outputs:
 *  - 0: Despread complex symbols for the selected user at symbol rate
 *  - 1: Lock status (float, 0.0 = unlocked, 1.0 = locked)
 *  - 2: SNR estimate (float, dB)
 *  - 3: Multi-user interference estimate (float, dB)
 *
 * The block uses correlation to separate the desired user's signal from other users
 * and noise, assuming orthogonal or quasi-orthogonal spreading codes.
 */
class QRADIOLINK_API dsss_cdma_receiver_cc : public block
{
public:
    typedef std::shared_ptr<dsss_cdma_receiver_cc> sptr;

    enum sync_state {
        STATE_ACQUISITION,  // Searching for code alignment
        STATE_TRACKING,     // Tracking code phase
        STATE_LOCKED        // Locked and despreading
    };

    /*!
     * \brief Make a DSSS-CDMA receiver block
     *
     * \param spreading_code Spreading code for the desired user (+1/-1 values)
     * \param spreading_factor Number of chips per symbol (32, 64, 128, 256, or 512)
     * \param correlation_threshold Threshold for valid correlation (default: 0.7)
     * \param timing_error_tolerance Samples tolerance for timing errors (default: 2)
     */
    static sptr make(const std::vector<int>& spreading_code,
                     int spreading_factor,
                     float correlation_threshold = 0.7f,
                     int timing_error_tolerance = 2);

    /*!
     * \brief Set spreading code for the desired user
     *
     * \param spreading_code New spreading code
     */
    virtual void set_spreading_code(const std::vector<int>& spreading_code);

    /*!
     * \brief Set spreading factor
     *
     * \param spreading_factor New spreading factor value
     */
    virtual void set_spreading_factor(int spreading_factor);

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

    /*!
     * \brief Get multi-user interference estimate
     *
     * \return Interference estimate in dB
     */
    virtual float get_interference_estimate() const;

    virtual ~dsss_cdma_receiver_cc();

protected:
    dsss_cdma_receiver_cc(const std::string& name,
                         gr::io_signature::sptr input_signature,
                         gr::io_signature::sptr output_signature,
                         const std::vector<int>& spreading_code,
                         int spreading_factor,
                         float correlation_threshold,
                         int timing_error_tolerance)
        : block(name, input_signature, output_signature)
    {
        (void)spreading_code;
        (void)spreading_factor;
        (void)correlation_threshold;
        (void)timing_error_tolerance;
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DSSS_CDMA_RECEIVER_CC_H */

