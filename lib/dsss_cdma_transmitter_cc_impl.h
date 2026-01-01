/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DSSS_CDMA_TRANSMITTER_CC_IMPL_H
#define INCLUDED_QRADIOLINK_DSSS_CDMA_TRANSMITTER_CC_IMPL_H

#include <gnuradio/qradiolink/dsss_cdma_transmitter_cc.h>
#include <vector>
#include <mutex>
#include <complex>

namespace gr {
namespace qradiolink {

class dsss_cdma_transmitter_cc_impl : public dsss_cdma_transmitter_cc
{
private:
    std::vector<std::vector<int>> d_spreading_codes;
    std::vector<std::vector<gr_complex>> d_spreading_codes_complex;
    std::vector<int> d_code_lengths;
    std::vector<int> d_chip_indices;
    int d_spreading_factor;
    int d_num_users;
    bool d_normalize_power;
    float d_power_normalization_factor;
    std::mutex d_mutex;

    void update_spreading_codes_complex();
    void update_power_normalization();

public:
    dsss_cdma_transmitter_cc_impl(const std::vector<std::vector<int>>& spreading_codes,
                                   int spreading_factor,
                                   int num_users,
                                   bool normalize_power);
    ~dsss_cdma_transmitter_cc_impl();

    void set_spreading_codes(const std::vector<std::vector<int>>& spreading_codes) override;
    void set_spreading_factor(int spreading_factor) override;
    void set_num_users(int num_users) override;
    void set_normalize_power(bool normalize_power) override;

    void forecast(int noutput_items, gr_vector_int& ninput_items_required) override;

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DSSS_CDMA_TRANSMITTER_CC_IMPL_H */

