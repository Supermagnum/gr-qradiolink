/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_SOQPSK_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_SOQPSK_IMPL_H

#include <gnuradio/qradiolink/demod_soqpsk.h>
#include <gnuradio/digital/symbol_sync_cc.h>
#include <gnuradio/analog/agc2_cc.h>
#include <gnuradio/digital/fll_band_edge_cc.h>
#include <gnuradio/digital/costas_loop_cc.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/digital/constellation.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/digital/descrambler_bb.h>
#include <gnuradio/digital/diff_phasor_cc.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/blocks/interleave.h>
#include <gnuradio/blocks/float_to_uchar.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/fec/decoder.h>
#include <gnuradio/fec/cc_decoder.h>
#include <gnuradio/blocks/rotator_cc.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/blocks/stream_to_streams.h>
#include <gnuradio/blocks/streams_to_stream.h>
#include <gnuradio/filter/fft_filter_ccc.h>
#include <vector>

namespace gr {
namespace qradiolink {

class demod_soqpsk_impl : public demod_soqpsk
{
private:
    // Mode 1 (single channel) blocks
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::fft_filter_ccf::sptr d_shaping_filter;
    gr::analog::agc2_cc::sptr d_agc;
    gr::digital::fll_band_edge_cc::sptr d_fll;
    gr::digital::symbol_sync_cc::sptr d_symbol_sync;
    gr::digital::costas_loop_cc::sptr d_costas_loop;
    gr::digital::costas_loop_cc::sptr d_costas_pll;
    gr::digital::diff_phasor_cc::sptr d_diff_phasor;
    gr::blocks::multiply_const_cc::sptr d_rotate_const;
    gr::blocks::multiply_const_ff::sptr d_multiply_const_fec;
    gr::blocks::complex_to_float::sptr d_complex_to_float;
    gr::blocks::interleave::sptr d_interleave;
    gr::blocks::float_to_uchar::sptr d_float_to_uchar;
    gr::blocks::add_const_ff::sptr d_add_const_fec;
    gr::fec::decoder::sptr d_decode_ccsds;
    gr::digital::descrambler_bb::sptr d_descrambler;
    gr::blocks::rotator_cc::sptr d_rotator;

    // Mode 2 (multi-carrier) blocks
    std::vector<gr::filter::fft_filter_ccc::sptr> d_channel_filters;
    std::vector<gr::blocks::rotator_cc::sptr> d_rotator_mc;
    std::vector<gr::filter::rational_resampler_ccf::sptr> d_resampler_mc;
    std::vector<gr::filter::fft_filter_ccf::sptr> d_shaping_filter_mc;
    std::vector<gr::analog::agc2_cc::sptr> d_agc_mc;
    std::vector<gr::digital::symbol_sync_cc::sptr> d_symbol_sync_mc;
    std::vector<gr::digital::costas_loop_cc::sptr> d_costas_loop_mc;
    std::vector<gr::digital::diff_phasor_cc::sptr> d_diff_phasor_mc;
    std::vector<gr::blocks::multiply_const_cc::sptr> d_rotate_const_mc;
    std::vector<gr::blocks::multiply_const_ff::sptr> d_multiply_const_fec_mc;
    std::vector<gr::blocks::complex_to_float::sptr> d_complex_to_float_mc;
    std::vector<gr::blocks::interleave::sptr> d_interleave_mc;
    std::vector<gr::blocks::float_to_uchar::sptr> d_float_to_uchar_mc;
    std::vector<gr::blocks::add_const_ff::sptr> d_add_const_fec_mc;
    std::vector<gr::fec::decoder::sptr> d_decode_ccsds_mc;
    std::vector<gr::digital::descrambler_bb::sptr> d_descrambler_mc;
    gr::blocks::streams_to_stream::sptr d_stream_combiner;

    int d_mode;
    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;

public:
    demod_soqpsk_impl(int mode, int sps, int samp_rate, int carrier_freq, int filter_width);
    ~demod_soqpsk_impl();
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_SOQPSK_IMPL_H */

