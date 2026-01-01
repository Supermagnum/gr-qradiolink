/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DSSS_CDMA_TRANSMITTER_CC_H
#define INCLUDED_QRADIOLINK_DSSS_CDMA_TRANSMITTER_CC_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/block.h>
#include <vector>

namespace gr {
namespace qradiolink {

/*!
 * \brief DSSS-CDMA Transmitter - combines multiple user signals with spreading codes
 * \ingroup qradiolink
 *
 * This block implements a Direct Sequence Spread Spectrum Code Division Multiple Access
 * (DSSS-CDMA) transmitter that accepts multiple user inputs, spreads each with their
 * respective PN sequence, and combines them into a single CDMA signal.
 *
 * Features:
 * - Multiple user inputs (one per user)
 * - Configurable spreading codes per user
 * - Configurable spreading factors: 32, 64, 128, 256, 512 chips per symbol
 * - Support for orthogonal/quasi-orthogonal codes (Gold codes, m-sequences)
 * - Automatic power normalization for equal user power
 *
 * Inputs: Multiple complex symbol streams (one per user) at symbol rate
 * Output: Combined CDMA signal at chip rate
 *
 * The block accepts complex inputs from modulators (2FSK, 4FSK, 8FSK, GMSK, BPSK, QPSK, SOQPSK)
 * and spreads each user's signal with their assigned spreading code.
 */
class QRADIOLINK_API dsss_cdma_transmitter_cc : public block
{
public:
    typedef std::shared_ptr<dsss_cdma_transmitter_cc> sptr;

    /*!
     * \brief Make a DSSS-CDMA transmitter block
     *
     * \param spreading_codes Vector of spreading codes (one per user), each code is +1/-1
     * \param spreading_factor Number of chips per symbol (32, 64, 128, 256, or 512)
     * \param num_users Number of users (must match spreading_codes size)
     * \param normalize_power If true, normalize power per user (default: true)
     */
    static sptr make(const std::vector<std::vector<int>>& spreading_codes,
                     int spreading_factor,
                     int num_users,
                     bool normalize_power = true);

    /*!
     * \brief Set spreading codes for all users
     *
     * \param spreading_codes Vector of spreading codes (one per user)
     */
    virtual void set_spreading_codes(const std::vector<std::vector<int>>& spreading_codes);

    /*!
     * \brief Set spreading factor
     *
     * \param spreading_factor Number of chips per symbol
     */
    virtual void set_spreading_factor(int spreading_factor);

    /*!
     * \brief Set number of users
     *
     * \param num_users Number of active users
     */
    virtual void set_num_users(int num_users);

    /*!
     * \brief Set power normalization
     *
     * \param normalize_power If true, normalize power per user
     */
    virtual void set_normalize_power(bool normalize_power);

    virtual ~dsss_cdma_transmitter_cc();

protected:
    dsss_cdma_transmitter_cc(const std::string& name,
                            gr::io_signature::sptr input_signature,
                            gr::io_signature::sptr output_signature,
                            const std::vector<std::vector<int>>& spreading_codes,
                            int spreading_factor,
                            int num_users,
                            bool normalize_power)
        : block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DSSS_CDMA_TRANSMITTER_CC_H */

