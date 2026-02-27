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
#include <gnuradio/qradiolink/mod_ssb.h>
#include "mod_ssb_impl.h"
#include <gnuradio/io_signature.h>

// CESSB blocks - migrated to qradiolink namespace
#include <gnuradio/qradiolink/clipper_cc.h>
#include <gnuradio/qradiolink/stretcher_cc.h>

namespace gr {
namespace qradiolink {

mod_ssb::sptr mod_ssb::make(int sps, int samp_rate, int carrier_freq, int filter_width, int sb)
{
    return gnuradio::get_initial_sptr(
        new mod_ssb_impl(sps, samp_rate, carrier_freq, filter_width, sb));
}

mod_ssb_impl::mod_ssb_impl(int sps, int samp_rate, int carrier_freq, int filter_width, int sb)
    : mod_ssb("mod_ssb",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    d_samp_rate = samp_rate;
    d_sps = sps;
    float target_samp_rate = 8000.0f;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    d_sb = sb;

    d_audio_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::band_pass_2(
            1, target_samp_rate, 300, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_float_to_complex = gr::blocks::float_to_complex::make();
    std::vector<float> interp_taps = gr::filter::firdes::low_pass_2(
        d_sps, d_samp_rate, d_filter_width, d_filter_width, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);

    d_resampler = gr::filter::rational_resampler_ccf::make(d_sps, 1, interp_taps);
    d_re_scale1 = gr::blocks::multiply_const_cc::make(2.0f * 0.353553390593f, 1);
    d_re_scale2 = gr::blocks::multiply_const_cc::make(0.9f / 0.353553390593f, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);
    d_filter_usb1 = gr::filter::fft_filter_ccc::make(
        1.0,
        gr::filter::firdes::complex_band_pass_2(
            1, target_samp_rate, 200, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_filter_lsb1 = gr::filter::fft_filter_ccc::make(
        1.0,
        gr::filter::firdes::complex_band_pass_2(
            1, target_samp_rate, -d_filter_width, -200, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_filter_usb2 = gr::filter::fft_filter_ccc::make(
        1.0,
        gr::filter::firdes::complex_band_pass_2(
            1, target_samp_rate, 200, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_filter_lsb2 = gr::filter::fft_filter_ccc::make(
        1.0,
        gr::filter::firdes::complex_band_pass_2(
            1, target_samp_rate, -d_filter_width, -200, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_filter_usb3 = gr::filter::fft_filter_ccc::make(
        1.0,
        gr::filter::firdes::complex_band_pass_2(
            1, target_samp_rate, 200, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_filter_lsb3 = gr::filter::fft_filter_ccc::make(
        1.0,
        gr::filter::firdes::complex_band_pass_2(
            1, target_samp_rate, -d_filter_width, -200, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_clipper = gr::qradiolink::clipper_cc::make(0.3535533f);
    d_stretcher = gr::qradiolink::stretcher_cc::make();

    connect(self(), 0, d_audio_filter, 0);
    connect(d_audio_filter, 0, d_float_to_complex, 0);
    connect(d_float_to_complex, 0, d_re_scale1, 0);
    if (!d_sb) {
        connect(d_re_scale1, 0, d_filter_usb1, 0);
        connect(d_filter_usb1, 0, d_clipper, 0);
        connect(d_clipper, 0, d_filter_usb2, 0);
        connect(d_filter_usb2, 0, d_stretcher, 0);
        connect(d_stretcher, 0, d_filter_usb3, 0);
        connect(d_filter_usb3, 0, d_re_scale2, 0);
    } else {
        connect(d_re_scale1, 0, d_filter_lsb1, 0);
        connect(d_filter_lsb1, 0, d_clipper, 0);
        connect(d_clipper, 0, d_filter_lsb2, 0);
        connect(d_filter_lsb2, 0, d_stretcher, 0);
        connect(d_stretcher, 0, d_filter_lsb3, 0);
        connect(d_filter_lsb3, 0, d_re_scale2, 0);
    }
    connect(d_re_scale2, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_resampler, 0);
    connect(d_resampler, 0, self(), 0);
}

mod_ssb_impl::~mod_ssb_impl() {}

void mod_ssb_impl::set_filter_width(int filter_width)
{
    d_filter_width = filter_width;
    float target_samp_rate = 8000.0f;
    std::vector<float> interp_taps = gr::filter::firdes::low_pass_2(
        d_sps, d_samp_rate, d_filter_width, d_filter_width, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);

    std::vector<gr_complex> filter_usb_taps = gr::filter::firdes::complex_band_pass_2(
        1, target_samp_rate, 300, d_filter_width, 250, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<gr_complex> filter_lsb_taps = gr::filter::firdes::complex_band_pass_2(
        1, target_samp_rate, -d_filter_width, -300, 250, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);

    d_resampler->set_taps(interp_taps);
    d_filter_usb1->set_taps(filter_usb_taps);
    d_filter_lsb1->set_taps(filter_lsb_taps);
    d_filter_usb2->set_taps(filter_usb_taps);
    d_filter_lsb2->set_taps(filter_lsb_taps);
    d_filter_usb3->set_taps(filter_usb_taps);
    d_filter_lsb3->set_taps(filter_lsb_taps);
}

void mod_ssb_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_ssb::set_filter_width(int filter_width)
{
    (void)filter_width;
    // This should never be called, as mod_ssb is only an interface
    // The actual implementation is in mod_ssb_impl
}

void mod_ssb::set_bb_gain(float value)
{
    (void)value;
    // This should never be called, as mod_ssb is only an interface
    // The actual implementation is in mod_ssb_impl
}

} // namespace qradiolink
} // namespace gr

