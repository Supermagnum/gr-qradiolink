/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_CPM_4FSK_H
#define INCLUDED_QRADIOLINK_MOD_CPM_4FSK_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief 4FSK CPM (Continuous Phase Modulation) Modulator
 * \ingroup qradiolink
 *
 * This block implements a 4-level FSK modulator using Continuous Phase
 * Modulation (CPM) with LREC (Length-Rectangular) phase response.
 * CPM provides better spectral efficiency and phase continuity compared
 * to standard FSK modulation.
 */
class QRADIOLINK_API mod_cpm_4fsk : public hier_block2
{
public:
    typedef std::shared_ptr<mod_cpm_4fsk> sptr;

    /*!
     * \brief Make a 4FSK CPM modulator block
     *
     * \param sps Samples per symbol (default: 2)
     * \param samp_rate Sample rate (default: 96000)
     * \param carrier_freq Carrier frequency in Hz (default: 0)
     * \param filter_width Filter width in Hz (default: 5000)
     * \param h Modulation index (default: 0.5)
     * \param L Pulse length (default: 4)
     * \param beta Roll-off factor (default: 0.3)
     */
    static sptr make(int sps = 2,
                     int samp_rate = 96000,
                     int carrier_freq = 0,
                     int filter_width = 5000,
                     float h = 0.5,
                     int L = 4,
                     float beta = 0.3);

    /*!
     * \brief Set baseband gain
     * \param value Gain value
     */
    virtual void set_bb_gain(float value);

protected:
    mod_cpm_4fsk(const std::string& name,
                 gr::io_signature::sptr input_signature,
                 gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_CPM_4FSK_H */

