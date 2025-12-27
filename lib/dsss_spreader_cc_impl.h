/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DSSS_SPREADER_CC_IMPL_H
#define INCLUDED_QRADIOLINK_DSSS_SPREADER_CC_IMPL_H

#include <gnuradio/qradiolink/dsss_spreader_cc.h>
#include <vector>
#include <mutex>

namespace gr {
namespace qradiolink {

class dsss_spreader_cc_impl : public dsss_spreader_cc
{
private:
    std::vector<int> d_pn_sequence;
    std::vector<gr_complex> d_pn_sequence_complex;
    int d_code_length;
    int d_chips_per_symbol;
    int d_chip_index;
    std::mutex d_mutex; // For thread-safe parameter updates

    void update_pn_sequence_complex();

public:
    dsss_spreader_cc_impl(const std::vector<int>& pn_sequence, int chips_per_symbol);
    ~dsss_spreader_cc_impl();

    void set_pn_sequence(const std::vector<int>& pn_sequence) override;
    void set_chips_per_symbol(int chips_per_symbol) override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DSSS_SPREADER_CC_IMPL_H */

