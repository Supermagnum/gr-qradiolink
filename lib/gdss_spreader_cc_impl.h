/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_GDSS_SPREADER_CC_IMPL_H
#define INCLUDED_QRADIOLINK_GDSS_SPREADER_CC_IMPL_H

#include <gnuradio/qradiolink/gdss_spreader_cc.h>
#include <vector>
#include <mutex>
#include <random>
#include <complex>

namespace gr {
namespace qradiolink {

class gdss_spreader_cc_impl : public gdss_spreader_cc
{
private:
    std::vector<float> d_spreading_sequence;
    std::vector<gr_complex> d_spreading_sequence_complex;
    int d_sequence_length;
    int d_chips_per_symbol;
    int d_chip_index;
    float d_variance;
    unsigned int d_seed;
    std::mt19937 d_rng;
    std::normal_distribution<float> d_gaussian;
    std::mutex d_mutex;

    void generate_sequence();
    void update_sequence_complex();

public:
    gdss_spreader_cc_impl(int sequence_length,
                           int chips_per_symbol,
                           float variance,
                           unsigned int seed);
    ~gdss_spreader_cc_impl();

    void set_spreading_sequence(const std::vector<float>& sequence) override;
    void set_chips_per_symbol(int chips_per_symbol) override;
    void regenerate_sequence(float variance, unsigned int seed) override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_GDSS_SPREADER_CC_IMPL_H */

