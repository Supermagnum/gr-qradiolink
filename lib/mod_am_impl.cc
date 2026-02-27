/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <climits>
#include <cstdint>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gnuradio/qradiolink/mod_am.h>
#include "mod_am_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/qradiolink/clipper_cc.h>
#include <gnuradio/qradiolink/stretcher_cc.h>

namespace gr {
namespace qradiolink {

mod_am::sptr mod_am::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(
        new mod_am_impl(sps, samp_rate, carrier_freq, filter_width));
}

mod_am_impl::mod_am_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : mod_am("mod_am",
             gr::io_signature::make(1, 1, sizeof(float)),
             gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    d_samp_rate = samp_rate;
    d_sps = sps;
    float target_samp_rate = 8000.0f;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;

    d_signal_source = gr::analog::sig_source_f::make(
        target_samp_rate, gr::analog::GR_COS_WAVE, 0, 0.5);
    d_add = gr::blocks::add_ff::make();
    d_rescale = gr::blocks::multiply_const_cc::make(1.0f / 1.5f * 0.3535533f, 1);
    d_audio_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::low_pass_2(
            1, 8000, 3400, 400, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_float_to_complex = gr::blocks::float_to_complex::make();
    std::vector<float> interp_taps = gr::filter::firdes::low_pass_2(
        d_sps, d_samp_rate, d_filter_width, 400, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(d_sps, 1, interp_taps);
    d_amplify = gr::blocks::multiply_const_cc::make(0.8f / 0.3535533f, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);
    d_filter = gr::filter::fft_filter_ccc::make(
        1,
        gr::filter::firdes::complex_band_pass_2(
            1, d_samp_rate, -d_filter_width, d_filter_width, 600, 90,
            gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_clipper = gr::qradiolink::clipper_cc::make(0.3535533f);
    d_stretcher = gr::qradiolink::stretcher_cc::make();

    connect(self(), 0, d_audio_filter, 0);
    connect(d_audio_filter, 0, d_add, 0);
    connect(d_signal_source, 0, d_add, 1);
    connect(d_add, 0, d_float_to_complex, 0);
    connect(d_float_to_complex, 0, d_rescale, 0);
    connect(d_rescale, 0, d_clipper, 0);
    connect(d_clipper, 0, d_stretcher, 0);
    connect(d_stretcher, 0, d_resampler, 0);
    connect(d_resampler, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_filter, 0);
    connect(d_filter, 0, self(), 0);
}

mod_am_impl::~mod_am_impl() {}

void mod_am_impl::set_filter_width(int filter_width)
{
    d_filter_width = filter_width;
    std::vector<float> interp_taps = gr::filter::firdes::low_pass(
        d_sps, d_samp_rate, d_filter_width, d_filter_width);
    std::vector<gr_complex> filter_taps = gr::filter::firdes::complex_band_pass_2(
        1, d_samp_rate, -d_filter_width, d_filter_width, 1200, 90,
        gr::fft::window::WIN_BLACKMAN_HARRIS);

    d_resampler->set_taps(interp_taps);
    d_filter->set_taps(filter_taps);
}

void mod_am_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_am::set_filter_width(int filter_width)
{
    (void)filter_width;
}

void mod_am::set_bb_gain(float value)
{
    (void)value;
}

} // namespace qradiolink
} // namespace gr
