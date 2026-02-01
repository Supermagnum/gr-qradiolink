/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_INTERLEAVER_BB_H
#define INCLUDED_QRADIOLINK_INTERLEAVER_BB_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief Block interleaver for HF burst error handling
 * \ingroup qradiolink
 *
 * This block implements a block (matrix) interleaver that spreads burst errors
 * across multiple symbol positions, making them easier for FEC codes to correct.
 * Common in HF data links where ionospheric fading causes burst errors.
 *
 * Interleave mode (TX path): Write row-by-row, read column-by-column.
 * Deinterleave mode (RX path): Write column-by-column, read row-by-row.
 *
 * Typical placement: FEC encoder -> interleaver -> modulator (TX)
 *                    demodulator -> deinterleaver -> FEC decoder (RX)
 */
class QRADIOLINK_API interleaver_bb : public gr::sync_block
{
public:
    typedef std::shared_ptr<interleaver_bb> sptr;

    /*!
     * \brief Create a block interleaver
     *
     * \param n_rows Number of rows (interleaving depth)
     * \param n_cols Number of columns
     * \param interleave True for interleave (TX), false for deinterleave (RX)
     */
    static sptr make(unsigned int n_rows, unsigned int n_cols, bool interleave = true);

protected:
    interleaver_bb(const std::string& name,
                   gr::io_signature::sptr input_signature,
                   gr::io_signature::sptr output_signature)
        : gr::sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_INTERLEAVER_BB_H */
