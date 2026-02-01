/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_INTERLEAVER_BB_IMPL_H
#define INCLUDED_QRADIOLINK_INTERLEAVER_BB_IMPL_H

#include <gnuradio/qradiolink/interleaver_bb.h>
#include <vector>

namespace gr {
namespace qradiolink {

class interleaver_bb_impl : public interleaver_bb
{
private:
    unsigned int d_n_rows;
    unsigned int d_n_cols;
    unsigned int d_block_size;
    bool d_interleave;
    std::vector<unsigned int> d_permutation;

    void build_permutation();

public:
    interleaver_bb_impl(unsigned int n_rows, unsigned int n_cols, bool interleave);
    ~interleaver_bb_impl() override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_INTERLEAVER_BB_IMPL_H */
