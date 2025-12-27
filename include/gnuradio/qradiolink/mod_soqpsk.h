/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_SOQPSK_H
#define INCLUDED_QRADIOLINK_MOD_SOQPSK_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief Shaped Offset Quadrature Phase Shift Keying (SOQPSK) Modulator
 * \ingroup qradiolink
 *
 * This block implements a Shaped Offset Quadrature Phase Shift Keying (SOQPSK) modulator
 * with support for two modes:
 * - Mode 1: Single channel at 14,400 bps (~10 kHz bandwidth)
 * - Mode 2: Multi-carrier with 3 carriers at 4,800 bps each, 4 kHz spacing (~12 kHz total bandwidth)
 */
class QRADIOLINK_API mod_soqpsk : public hier_block2
{
public:
    typedef std::shared_ptr<mod_soqpsk> sptr;

    /*!
     * \brief Make a SOQPSK modulator block
     *
     * \param mode Modulation mode (1 = single channel, 2 = multi-carrier)
     * \param sps Samples per symbol (default: 10 for mode 1, 25 for mode 2)
     * \param samp_rate Sample rate (default: 250000)
     * \param carrier_freq Carrier frequency in Hz (default: 0, only used in mode 1)
     * \param filter_width Filter width in Hz (default: 10000 for mode 1, 12000 for mode 2)
     */
    static sptr make(int mode = 1,
                     int sps = 10,
                     int samp_rate = 250000,
                     int carrier_freq = 0,
                     int filter_width = 10000);

    /*!
     * \brief Set baseband gain
     * \param value Gain value
     */
    virtual void set_bb_gain(float value);

protected:
    mod_soqpsk(const std::string& name,
               gr::io_signature::sptr input_signature,
               gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_SOQPSK_H */

