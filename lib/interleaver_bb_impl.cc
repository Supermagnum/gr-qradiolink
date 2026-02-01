/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/qradiolink/interleaver_bb.h>
#include "interleaver_bb_impl.h"
#include <gnuradio/io_signature.h>
#include <cstring>
#include <stdexcept>

namespace gr {
namespace qradiolink {

interleaver_bb::sptr interleaver_bb::make(unsigned int n_rows,
                                          unsigned int n_cols,
                                          bool interleave)
{
    return gnuradio::get_initial_sptr(
        new interleaver_bb_impl(n_rows, n_cols, interleave));
}

interleaver_bb_impl::interleaver_bb_impl(unsigned int n_rows,
                                         unsigned int n_cols,
                                         bool interleave)
    : interleaver_bb("interleaver_bb",
                     gr::io_signature::make(1, 1, sizeof(unsigned char)),
                     gr::io_signature::make(1, 1, sizeof(unsigned char))),
      d_n_rows(n_rows),
      d_n_cols(n_cols),
      d_interleave(interleave)
{
    if (n_rows == 0 || n_cols == 0) {
        throw std::invalid_argument("interleaver_bb: n_rows and n_cols must be > 0");
    }
    d_block_size = n_rows * n_cols;
    build_permutation();
    set_output_multiple(d_block_size);
}

interleaver_bb_impl::~interleaver_bb_impl() {}

void interleaver_bb_impl::build_permutation()
{
    d_permutation.resize(d_block_size);

    if (d_interleave) {
        for (unsigned int out_idx = 0; out_idx < d_block_size; out_idx++) {
            unsigned int col = out_idx / d_n_rows;
            unsigned int row = out_idx % d_n_rows;
            unsigned int in_idx = row * d_n_cols + col;
            d_permutation[out_idx] = in_idx;
        }
    } else {
        for (unsigned int out_idx = 0; out_idx < d_block_size; out_idx++) {
            unsigned int row = out_idx / d_n_cols;
            unsigned int col = out_idx % d_n_cols;
            unsigned int in_idx = col * d_n_rows + row;
            d_permutation[out_idx] = in_idx;
        }
    }
}

int interleaver_bb_impl::work(int noutput_items,
                              gr_vector_const_void_star& input_items,
                              gr_vector_void_star& output_items)
{
    const unsigned char* in = (const unsigned char*)input_items[0];
    unsigned char* out = (unsigned char*)output_items[0];

    int n_blocks = noutput_items / d_block_size;
    for (int b = 0; b < n_blocks; b++) {
        const unsigned char* block_in = in + b * d_block_size;
        unsigned char* block_out = out + b * d_block_size;
        for (unsigned int i = 0; i < d_block_size; i++) {
            block_out[i] = block_in[d_permutation[i]];
        }
    }

    return n_blocks * d_block_size;
}

} // namespace qradiolink
} // namespace gr
