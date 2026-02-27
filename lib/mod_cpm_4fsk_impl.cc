/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/qradiolink/mod_cpm_4fsk.h>
#include "mod_cpm_4fsk_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>

namespace gr {
namespace qradiolink {

mod_cpm_4fsk::sptr mod_cpm_4fsk::make(int sps,
                                      int samp_rate,
                                      int carrier_freq,
                                      int filter_width,
                                      float h,
                                      int L,
                                      float beta)
{
    return gnuradio::get_initial_sptr(
        new mod_cpm_4fsk_impl(sps, samp_rate, carrier_freq, filter_width, h, L, beta));
}

mod_cpm_4fsk_impl::mod_cpm_4fsk_impl(int sps,
                                     int samp_rate,
                                     int carrier_freq,
                                     int filter_width,
                                     float h,
                                     int L,
                                     float beta)
    : mod_cpm_4fsk("mod_cpm_4fsk",
                   gr::io_signature::make(1, 1, sizeof(char)),
                   gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_samples_per_symbol(sps),
      d_samp_rate(samp_rate),
      d_filter_width(filter_width),
      d_h(h),
      d_L(L),
      d_beta(beta)
{
    (void)carrier_freq;
    // 4FSK constellation: -1.5, -0.5, 0.5, 1.5
    std::vector<float> constellation;
    constellation.push_back(-1.5);
    constellation.push_back(-0.5);
    constellation.push_back(0.5);
    constellation.push_back(1.5);

    // Gray code mapping for 4FSK
    std::vector<int> map;
    map.push_back(0);
    map.push_back(1);
    map.push_back(3);
    map.push_back(2);

    // CCSDS convolutional encoder polynomials
    std::vector<int> polys;
    polys.push_back(109);
    polys.push_back(79);

    // Create blocks
    d_packed_to_unpacked = gr::blocks::packed_to_unpacked_bb::make(1, gr::GR_MSB_FIRST);
    d_packer = gr::blocks::pack_k_bits_bb::make(2);  // Pack 2 bits for 4FSK
    d_scrambler = gr::digital::scrambler_bb::make(0x8A, 0x7F, 7);

    gr::fec::code::cc_encoder::sptr encoder =
        gr::fec::code::cc_encoder::make(80, 7, 2, polys);
    d_encode_ccsds = gr::fec::encoder::make(encoder, 1, 1);

    d_map = gr::digital::map_bb::make(map);
    d_chunks_to_symbols = gr::digital::chunks_to_symbols_bf::make(constellation);

    // Get CPM phase response taps using analog.cpm
    // Note: We need to use the C++ API for phase_response
    // For now, we'll use a simple approach with interp_fir_filter
    // In a full implementation, we'd call cpm::phase_response() via C++ bindings
    
    // Create interpolation filter for CPM
    // Use root raised cosine as approximation, or get actual CPM taps
    int nfilts = d_samples_per_symbol * d_L * 4;
    if ((nfilts % 2) == 0)
        nfilts += 1;
    
    std::vector<float> taps = gr::filter::firdes::root_raised_cosine(
        d_samples_per_symbol, d_samples_per_symbol, 1.0, d_beta, nfilts);
    
    d_interp_fir = gr::filter::interp_fir_filter_fff::make(d_samples_per_symbol, taps);

    // Frequency modulator with modulation index h
    d_freq_modulator = gr::analog::frequency_modulator_fc::make(
        (d_h * M_PI) / d_samples_per_symbol);

    d_amplify = gr::blocks::multiply_const_cc::make(0.8, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);

    // Final resampler
    int second_interp = 10;
    if (d_samples_per_symbol == 2) {
        second_interp = 2;
    }
    d_resampler2 = gr::filter::rational_resampler_ccf::make(
        second_interp,
        1,
        gr::filter::firdes::low_pass(
            second_interp, d_samp_rate, d_filter_width, d_filter_width));

    // Connect blocks
    connect(self(), 0, d_packed_to_unpacked, 0);
    connect(d_packed_to_unpacked, 0, d_scrambler, 0);
    connect(d_scrambler, 0, d_encode_ccsds, 0);
    connect(d_encode_ccsds, 0, d_packer, 0);
    connect(d_packer, 0, d_map, 0);
    connect(d_map, 0, d_chunks_to_symbols, 0);
    connect(d_chunks_to_symbols, 0, d_interp_fir, 0);
    connect(d_interp_fir, 0, d_freq_modulator, 0);
    connect(d_freq_modulator, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_resampler2, 0);
    connect(d_resampler2, 0, self(), 0);
}

mod_cpm_4fsk_impl::~mod_cpm_4fsk_impl() {}

void mod_cpm_4fsk_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_cpm_4fsk::set_bb_gain(float value)
{
    (void)value;
    // This should never be called, as mod_cpm_4fsk is only an interface
    // The actual implementation is in mod_cpm_4fsk_impl
}

} // namespace qradiolink
} // namespace gr

