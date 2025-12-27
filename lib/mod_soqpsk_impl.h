/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_SOQPSK_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_SOQPSK_IMPL_H

#include <gnuradio/qradiolink/mod_soqpsk.h>
#include <gnuradio/blocks/packed_to_unpacked.h>
#include <gnuradio/blocks/pack_k_bits_bb.h>
#include <gnuradio/digital/chunks_to_symbols.h>
#include <gnuradio/digital/map_bb.h>
#include <gnuradio/digital/diff_encoder_bb.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/rotator_cc.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/blocks/stream_to_streams.h>
#include <gnuradio/blocks/streams_to_stream.h>
#include <vector>

namespace gr {
namespace qradiolink {

class mod_soqpsk_impl : public mod_soqpsk
{
private:
    // Mode 1 (single channel) blocks
    gr::blocks::packed_to_unpacked_bb::sptr d_packed_to_unpacked;
    gr::digital::chunks_to_symbols_bc::sptr d_chunks_to_symbols;
    gr::blocks::multiply_const_cc::sptr d_amplify;
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::digital::diff_encoder_bb::sptr d_diff_encoder;
    gr::blocks::pack_k_bits_bb::sptr d_packer;
    gr::digital::map_bb::sptr d_map;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::blocks::rotator_cc::sptr d_rotator;

    // Mode 2 (multi-carrier) blocks
    gr::blocks::stream_to_streams::sptr d_stream_splitter;
    gr::blocks::streams_to_stream::sptr d_stream_combiner;
    std::vector<gr::blocks::packed_to_unpacked_bb::sptr> d_packed_to_unpacked_mc;
    std::vector<gr::digital::chunks_to_symbols_bc::sptr> d_chunks_to_symbols_mc;
    std::vector<gr::digital::diff_encoder_bb::sptr> d_diff_encoder_mc;
    std::vector<gr::blocks::pack_k_bits_bb::sptr> d_packer_mc;
    std::vector<gr::digital::map_bb::sptr> d_map_mc;
    std::vector<gr::filter::rational_resampler_ccf::sptr> d_resampler_mc;
    std::vector<gr::blocks::rotator_cc::sptr> d_rotator_mc;
    std::vector<gr::blocks::multiply_const_cc::sptr> d_amplify_mc;
    std::vector<gr::blocks::add_cc::sptr> d_adders;

    int d_mode;
    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;

public:
    mod_soqpsk_impl(int mode, int sps, int samp_rate, int carrier_freq, int filter_width);
    ~mod_soqpsk_impl();

    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_SOQPSK_IMPL_H */

