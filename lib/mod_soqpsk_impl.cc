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
#include <cmath>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gnuradio/qradiolink/mod_soqpsk.h>
#include "mod_soqpsk_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/delay.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/blocks/stream_to_streams.h>

namespace gr {
namespace qradiolink {

mod_soqpsk::sptr mod_soqpsk::make(int mode, int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(
        new mod_soqpsk_impl(mode, sps, samp_rate, carrier_freq, filter_width));
}

mod_soqpsk_impl::mod_soqpsk_impl(int mode, int sps, int samp_rate, int carrier_freq, int filter_width)
    : mod_soqpsk("mod_soqpsk",
               gr::io_signature::make(1, 1, sizeof(char)),
               gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_mode(mode),
      d_samples_per_symbol(sps),
      d_samp_rate(samp_rate),
      d_carrier_freq(carrier_freq),
      d_filter_width(filter_width)
{
    // QPSK symbol table
    std::vector<gr_complex> symbol_table;
    symbol_table.push_back(gr_complex(-0.707, -0.707));
    symbol_table.push_back(gr_complex(-0.707, 0.707));
    symbol_table.push_back(gr_complex(0.707, 0.707));
    symbol_table.push_back(gr_complex(0.707, -0.707));

    std::vector<int> map;
    map.push_back(0);
    map.push_back(1);
    map.push_back(3);
    map.push_back(2);

    if (d_mode == 1) {
        // Mode 1: Single channel at 14,400 bps
        // Bit rate: 14,400 bps = 7,200 symbols/sec (QPSK = 2 bits/symbol)
        
        d_packed_to_unpacked = gr::blocks::packed_to_unpacked_bb::make(1, gr::GR_MSB_FIRST);
        d_packer = gr::blocks::pack_k_bits_bb::make(2);
        d_map = gr::digital::map_bb::make(map);
        d_diff_encoder = gr::digital::diff_encoder_bb::make(4);
        d_chunks_to_symbols = gr::digital::chunks_to_symbols_bc::make(symbol_table);

        // Pulse shaping filter (root raised cosine)
        int nfilts = 32;
        if (d_samples_per_symbol > 120)
            nfilts = 11;
        else if (d_samples_per_symbol > 10)
            nfilts = 13;
        else
            nfilts = 15;

        std::vector<float> rrc_taps = gr::filter::firdes::root_raised_cosine(
            d_samples_per_symbol, d_samples_per_symbol, 1.0, 0.35, nfilts * d_samples_per_symbol);

        // Upsample symbols with pulse shaping
        d_resampler = gr::filter::rational_resampler_ccf::make(
            d_samples_per_symbol, 1, rrc_taps);

        // Split into I and Q, delay Q by half symbol period for SOQPSK offset
        gr::blocks::complex_to_float::sptr c2f = gr::blocks::complex_to_float::make();
        gr::blocks::delay::sptr q_delay = gr::blocks::delay::make(sizeof(float), d_samples_per_symbol / 2);
        gr::blocks::float_to_complex::sptr f2c = gr::blocks::float_to_complex::make();

        d_amplify = gr::blocks::multiply_const_cc::make(0.6, 1);
        d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);

        // Carrier frequency rotation if needed
        if (d_carrier_freq != 0) {
            float phase_inc = 2.0f * M_PI * d_carrier_freq / d_samp_rate;
            d_rotator = gr::blocks::rotator_cc::make(phase_inc);
        } else {
            d_rotator = gr::blocks::rotator_cc::make(0.0f);
        }

        // Connect blocks for mode 1
        connect(self(), 0, d_packed_to_unpacked, 0);
        connect(d_packed_to_unpacked, 0, d_packer, 0);
        connect(d_packer, 0, d_map, 0);
        connect(d_map, 0, d_diff_encoder, 0);
        connect(d_diff_encoder, 0, d_chunks_to_symbols, 0);
        connect(d_chunks_to_symbols, 0, d_resampler, 0);
        
        // Split I/Q and delay Q by half symbol for SOQPSK offset
        connect(d_resampler, 0, c2f, 0);
        connect(c2f, 0, f2c, 0);  // I channel (real)
        connect(c2f, 1, q_delay, 0);  // Q channel
        connect(q_delay, 0, f2c, 1);  // Delayed Q channel (imag)
        
        // Apply carrier frequency and gain
        connect(f2c, 0, d_rotator, 0);
        connect(d_rotator, 0, d_amplify, 0);
        connect(d_amplify, 0, d_bb_gain, 0);
        connect(d_bb_gain, 0, self(), 0);

    } else if (d_mode == 2) {
        // Mode 2: Multi-carrier with 3 carriers at 4,800 bps each
        // Bit rate per carrier: 4,800 bps = 2,400 symbols/sec
        // Carrier spacing: 4 kHz
        // Total bandwidth: ~12 kHz
        
        // Split input into 3 streams (round-robin)
        d_stream_splitter = gr::blocks::stream_to_streams::make(sizeof(char), 3);
        
        // Create blocks for each carrier
        d_packed_to_unpacked_mc.resize(3);
        d_packer_mc.resize(3);
        d_map_mc.resize(3);
        d_diff_encoder_mc.resize(3);
        d_chunks_to_symbols_mc.resize(3);
        d_resampler_mc.resize(3);
        d_rotator_mc.resize(3);
        d_amplify_mc.resize(3);
        d_adders.resize(2);  // 2 adders to combine 3 carriers

        // Helper blocks for SOQPSK offset per carrier
        std::vector<gr::blocks::complex_to_float::sptr> c2f_mc(3);
        std::vector<gr::blocks::delay::sptr> q_delay_mc(3);
        std::vector<gr::blocks::float_to_complex::sptr> f2c_mc(3);

        int nfilts = 13;
        std::vector<float> rrc_taps_mc = gr::filter::firdes::root_raised_cosine(
            d_samples_per_symbol, d_samples_per_symbol, 1.0, 0.35, nfilts * d_samples_per_symbol);

        // Carrier frequency offsets: -4 kHz, 0, +4 kHz
        float carrier_offsets[3] = {-4000.0f, 0.0f, 4000.0f};

        for (int i = 0; i < 3; i++) {
            d_packed_to_unpacked_mc[i] = gr::blocks::packed_to_unpacked_bb::make(1, gr::GR_MSB_FIRST);
            d_packer_mc[i] = gr::blocks::pack_k_bits_bb::make(2);
            d_map_mc[i] = gr::digital::map_bb::make(map);
            d_diff_encoder_mc[i] = gr::digital::diff_encoder_bb::make(4);
            d_chunks_to_symbols_mc[i] = gr::digital::chunks_to_symbols_bc::make(symbol_table);
            d_resampler_mc[i] = gr::filter::rational_resampler_ccf::make(
                d_samples_per_symbol, 1, rrc_taps_mc);
            
            // SOQPSK offset: split I/Q and delay Q
            c2f_mc[i] = gr::blocks::complex_to_float::make();
            q_delay_mc[i] = gr::blocks::delay::make(sizeof(float), d_samples_per_symbol / 2);
            f2c_mc[i] = gr::blocks::float_to_complex::make();
            
            float phase_inc = 2.0f * M_PI * carrier_offsets[i] / d_samp_rate;
            d_rotator_mc[i] = gr::blocks::rotator_cc::make(phase_inc);
            d_amplify_mc[i] = gr::blocks::multiply_const_cc::make(0.33, 1);  // Scale down for combining
        }

        d_adders[0] = gr::blocks::add_cc::make();
        d_adders[1] = gr::blocks::add_cc::make();
        d_amplify = gr::blocks::multiply_const_cc::make(0.6, 1);
        d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);

        // Connect blocks for mode 2
        connect(self(), 0, d_stream_splitter, 0);
        
        for (int i = 0; i < 3; i++) {
            // Each carrier path
            connect(d_stream_splitter, i, d_packed_to_unpacked_mc[i], 0);
            connect(d_packed_to_unpacked_mc[i], 0, d_packer_mc[i], 0);
            connect(d_packer_mc[i], 0, d_map_mc[i], 0);
            connect(d_map_mc[i], 0, d_diff_encoder_mc[i], 0);
            connect(d_diff_encoder_mc[i], 0, d_chunks_to_symbols_mc[i], 0);
            connect(d_chunks_to_symbols_mc[i], 0, d_resampler_mc[i], 0);
            
            // SOQPSK offset: split I/Q and delay Q
            connect(d_resampler_mc[i], 0, c2f_mc[i], 0);
            connect(c2f_mc[i], 0, f2c_mc[i], 0);  // I channel
            connect(c2f_mc[i], 1, q_delay_mc[i], 0);  // Q channel
            connect(q_delay_mc[i], 0, f2c_mc[i], 1);  // Delayed Q channel
            
            connect(f2c_mc[i], 0, d_rotator_mc[i], 0);
            connect(d_rotator_mc[i], 0, d_amplify_mc[i], 0);
        }

        // Combine carriers
        connect(d_amplify_mc[0], 0, d_adders[0], 0);
        connect(d_amplify_mc[1], 0, d_adders[0], 1);
        connect(d_adders[0], 0, d_adders[1], 0);
        connect(d_amplify_mc[2], 0, d_adders[1], 1);
        connect(d_adders[1], 0, d_amplify, 0);
        connect(d_amplify, 0, d_bb_gain, 0);
        connect(d_bb_gain, 0, self(), 0);
    }
}

mod_soqpsk_impl::~mod_soqpsk_impl() {}

void mod_soqpsk_impl::set_bb_gain(float value)
{
    if (d_mode == 1) {
        d_bb_gain->set_k(value);
    } else {
        d_bb_gain->set_k(value);
    }
}

void mod_soqpsk::set_bb_gain(float value)
{
    // This should never be called, as mod_soqpsk is only an interface
    // The actual implementation is in mod_soqpsk_impl
}

} // namespace qradiolink
} // namespace gr

