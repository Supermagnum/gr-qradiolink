/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DSSS_SPREADER_CC_H
#define INCLUDED_QRADIOLINK_DSSS_SPREADER_CC_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_interpolator.h>
#include <vector>

namespace gr {
namespace qradiolink {

/*!
 * \brief DSSS Spreader - spreads complex symbols using PN sequence
 * \ingroup qradiolink
 *
 * This block accepts complex symbol stream at symbol rate and spreads
 * each symbol using a PN sequence to produce a wideband signal at chip rate.
 *
 * Input: Complex symbols at symbol rate (e.g., 12 ksps)
 * Output: Spread spectrum signal at chip rate (e.g., 500 ksps)
 *
 * Each input symbol is interpolated to chips_per_symbol samples and
 * multiplied by the PN sequence.
 */
class QRADIOLINK_API dsss_spreader_cc : public sync_interpolator
{
public:
    typedef std::shared_ptr<dsss_spreader_cc> sptr;

    /*!
     * \brief Make a DSSS spreader block
     *
     * \param pn_sequence PN sequence as vector of +1/-1 values
     * \param chips_per_symbol Number of chips per symbol (default: 42)
     */
    static sptr make(const std::vector<int>& pn_sequence, int chips_per_symbol = 42);

    /*!
     * \brief Set PN sequence (allows runtime updates)
     *
     * \param pn_sequence New PN sequence as vector of +1/-1 values
     */
    virtual void set_pn_sequence(const std::vector<int>& pn_sequence);

    /*!
     * \brief Set chips per symbol
     *
     * \param chips_per_symbol New chips per symbol value
     */
    virtual void set_chips_per_symbol(int chips_per_symbol);

    virtual ~dsss_spreader_cc();

protected:
    dsss_spreader_cc(const std::string& name,
                     gr::io_signature::sptr input_signature,
                     gr::io_signature::sptr output_signature,
                     const std::vector<int>& pn_sequence,
                     int chips_per_symbol)
        : sync_interpolator(name, input_signature, output_signature, chips_per_symbol)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DSSS_SPREADER_CC_H */

