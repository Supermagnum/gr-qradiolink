/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DSSS_CDMA_RECEIVER_CC_IMPL_H
#define INCLUDED_QRADIOLINK_DSSS_CDMA_RECEIVER_CC_IMPL_H

#include <gnuradio/qradiolink/dsss_cdma_receiver_cc.h>
#include <vector>
#include <deque>
#include <mutex>
#include <complex>

namespace gr {
namespace qradiolink {

class dsss_cdma_receiver_cc_impl : public dsss_cdma_receiver_cc
{
private:
    std::vector<int> d_spreading_code;
    std::vector<gr_complex> d_spreading_code_complex;
    int d_code_length;
    int d_spreading_factor;
    float d_correlation_threshold;
    int d_timing_error_tolerance;

    // State machine
    sync_state d_state;
    int d_code_phase;
    int d_timing_offset;
    int d_acquisition_counter;
    int d_lock_counter;
    static const int ACQUISITION_TIMEOUT = 10000;
    static const int LOCK_THRESHOLD = 10;

    // Correlation buffers
    std::deque<gr_complex> d_input_buffer;
    static const int BUFFER_SIZE = 2048;

    // Early-late gate for timing tracking
    float d_early_correlation;
    float d_prompt_correlation;
    float d_late_correlation;
    float d_timing_error;

    // Lock detection
    float d_correlation_peak;
    float d_correlation_avg;
    bool d_is_locked;

    // SNR estimation
    float d_signal_power;
    float d_noise_power;
    float d_snr_db;

    // Multi-user interference estimation
    float d_interference_power;
    float d_interference_db;

    mutable std::mutex d_mutex;

    // Helper functions
    gr_complex correlate(const gr_complex* samples, int offset, int length);
    void update_timing();
    void update_lock_detection(float correlation);
    void update_snr_estimate(gr_complex symbol, float correlation);
    void update_interference_estimate(float correlation);

    void forecast(int noutput_items, gr_vector_int& ninput_items_required) override;

public:
    dsss_cdma_receiver_cc_impl(const std::vector<int>& spreading_code,
                                int spreading_factor,
                                float correlation_threshold,
                                int timing_error_tolerance);
    ~dsss_cdma_receiver_cc_impl();

    void set_spreading_code(const std::vector<int>& spreading_code) override;
    void set_spreading_factor(int spreading_factor) override;
    sync_state get_sync_state() const override;
    bool is_locked() const override;
    float get_snr_estimate() const override;
    float get_interference_estimate() const override;

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DSSS_CDMA_RECEIVER_CC_IMPL_H */

