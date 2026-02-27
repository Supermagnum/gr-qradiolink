/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_CPM_4FSK_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_CPM_4FSK_IMPL_H

#include <gnuradio/qradiolink/mod_cpm_4fsk.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/packed_to_unpacked.h>
#include <gnuradio/blocks/pack_k_bits_bb.h>
#include <gnuradio/digital/chunks_to_symbols.h>
#include <gnuradio/digital/map_bb.h>
#include <gnuradio/digital/scrambler_bb.h>
#include <gnuradio/fec/cc_encoder.h>
#include <gnuradio/fec/encoder.h>
#include <gnuradio/filter/interp_fir_filter.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/analog/frequency_modulator_fc.h>
#include <gnuradio/analog/cpm.h>
#include <vector>

namespace gr {
namespace qradiolink {

class mod_cpm_4fsk_impl : public mod_cpm_4fsk
{
private:
    gr::blocks::packed_to_unpacked_bb::sptr d_packed_to_unpacked;
    gr::blocks::pack_k_bits_bb::sptr d_packer;
    gr::digital::scrambler_bb::sptr d_scrambler;
    gr::fec::encoder::sptr d_encode_ccsds;
    gr::digital::map_bb::sptr d_map;
    gr::digital::chunks_to_symbols_bf::sptr d_chunks_to_symbols;
    gr::filter::interp_fir_filter_fff::sptr d_interp_fir;
    gr::analog::frequency_modulator_fc::sptr d_freq_modulator;
    gr::blocks::multiply_const_cc::sptr d_amplify;
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::filter::rational_resampler_ccf::sptr d_resampler2;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_filter_width;
    float d_h;
    int d_L;
    float d_beta;

public:
    mod_cpm_4fsk_impl(int sps,
                      int samp_rate,
                      int carrier_freq,
                      int filter_width,
                      float h,
                      int L,
                      float beta);
    ~mod_cpm_4fsk_impl();

    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_CPM_4FSK_IMPL_H */

