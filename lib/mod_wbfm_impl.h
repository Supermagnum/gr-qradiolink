/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_WBFM_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_WBFM_IMPL_H

#include <gnuradio/qradiolink/mod_wbfm.h>
#include <gnuradio/analog/frequency_modulator_fc.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/filter/iir_filter_ffd.h>
#include <gnuradio/blocks/multiply_const.h>
#include <vector>

namespace gr {
namespace qradiolink {

class mod_wbfm_impl : public mod_wbfm
{
private:
    gr::analog::frequency_modulator_fc::sptr d_fm_modulator;
    gr::filter::iir_filter_ffd::sptr d_pre_emph_filter;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::rational_resampler_fff::sptr d_audio_resampler;
    gr::blocks::multiply_const_cc::sptr d_amplify;
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::blocks::multiply_const_ff::sptr d_audio_amplify;
    gr::filter::fft_filter_fff::sptr d_audio_filter;
    gr::filter::fft_filter_ccf::sptr d_filter;

    int d_samp_rate;
    int d_sps;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;
    int d_if_samp_rate;
    std::vector<double> d_btaps;
    std::vector<double> d_ataps;

public:
    mod_wbfm_impl(int sps, int samp_rate, int carrier_freq, int filter_width);
    ~mod_wbfm_impl();

    void set_filter_width(int filter_width) override;
    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_WBFM_IMPL_H */

