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

#include <gnuradio/qradiolink/demod_soqpsk.h>
#include "demod_soqpsk_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/window.h>
#include <complex>

namespace gr {
namespace qradiolink {

demod_soqpsk::sptr demod_soqpsk::make(int mode, int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(new demod_soqpsk_impl(mode, sps, samp_rate, carrier_freq, filter_width));
}

demod_soqpsk_impl::demod_soqpsk_impl(int mode, int sps, int samp_rate, int carrier_freq, int filter_width)
    : demod_soqpsk("demod_soqpsk",
                 gr::io_signature::make(1, 1, sizeof(gr_complex)),
                 gr::io_signature::makev(3, 3, std::vector<int>{sizeof(gr_complex), sizeof(gr_complex), sizeof(char)})),
      d_mode(mode),
      d_samples_per_symbol(sps),
      d_samp_rate(samp_rate),
      d_carrier_freq(carrier_freq)
{
    (void)filter_width;
    int decimation;
    int interpolation;
    float costas_bw;
    int fll_bw = 2;
    costas_bw = M_PI / 200;

    std::vector<int> map;
    map.push_back(0);
    map.push_back(1);
    map.push_back(3);
    map.push_back(2);

    std::vector<int> polys;
    polys.push_back(109);
    polys.push_back(79);

    if (d_mode == 1) {
        // Mode 1: Single channel at 14,400 bps
        if (d_samples_per_symbol > 4 && d_samples_per_symbol < 125) {
            interpolation = 1;
            decimation = 25;
            d_samples_per_symbol = d_samples_per_symbol * 4 / 25;
            d_target_samp_rate = 40000;
        } else if (d_samples_per_symbol >= 125) {
            interpolation = 1;
            decimation = 100;
            d_samples_per_symbol = d_samples_per_symbol / 25;
            d_target_samp_rate = 10000;
        } else {
            interpolation = 1;
            decimation = 2;
            d_target_samp_rate = 500000;
            costas_bw = M_PI / 400;
        }

        std::vector<float> taps = gr::filter::firdes::low_pass_2(
            interpolation, d_samp_rate * interpolation, d_target_samp_rate / 2, d_target_samp_rate / 10, 60,
            gr::fft::window::WIN_BLACKMAN_HARRIS);

        d_resampler = gr::filter::rational_resampler_ccf::make(interpolation, decimation, taps);
        d_resampler->set_thread_priority(99);
        d_agc = gr::analog::agc2_cc::make(1, 1e-1, 1.0, 1.0);

        d_fll = gr::digital::fll_band_edge_cc::make(d_samples_per_symbol, 0.35, 32, fll_bw * M_PI / 100);
        std::vector<float> rrc_taps = gr::filter::firdes::root_raised_cosine(
            d_samples_per_symbol, d_samples_per_symbol, 1, 0.35, 11 * d_samples_per_symbol);
        d_shaping_filter = gr::filter::fft_filter_ccf::make(1, rrc_taps);

        float symbol_rate = (float)d_target_samp_rate / (float)d_samples_per_symbol;
        float sps_deviation = 200.0f / symbol_rate;
        d_symbol_sync = gr::digital::symbol_sync_cc::make(
            gr::digital::TED_MOD_MUELLER_AND_MULLER, d_samples_per_symbol, 2 * M_PI / (symbol_rate / 10), 1.0, 0.2869,
            sps_deviation, 1, gr::digital::constellation_dqpsk::make(), gr::digital::IR_MMSE_8TAP);
        d_costas_pll = gr::digital::costas_loop_cc::make(M_PI / 200 / d_samples_per_symbol, 4, true);

        d_costas_loop = gr::digital::costas_loop_cc::make(costas_bw, 4, true);

        d_diff_phasor = gr::digital::diff_phasor_cc::make();
        const std::complex<float> i(0, 1);
        const std::complex<float> rot(-3 * M_PI / 4, 0);
        d_rotate_const = gr::blocks::multiply_const_cc::make(std::exp(i * rot));
        d_complex_to_float = gr::blocks::complex_to_float::make();
        d_interleave = gr::blocks::interleave::make(4);
        d_multiply_const_fec = gr::blocks::multiply_const_ff::make(48);
        d_float_to_uchar = gr::blocks::float_to_uchar::make();
        d_add_const_fec = gr::blocks::add_const_ff::make(128.0);
        gr::fec::code::cc_decoder::sptr decoder = gr::fec::code::cc_decoder::make(80, 7, 2, polys);
        d_decode_ccsds = gr::fec::decoder::make(decoder, 1, 1);
        d_descrambler = gr::digital::descrambler_bb::make(0x8A, 0x7F, 7);

        // Carrier frequency rotation if needed
        if (d_carrier_freq != 0) {
            float phase_inc = -2.0f * M_PI * d_carrier_freq / d_samp_rate;
            d_rotator = gr::blocks::rotator_cc::make(phase_inc);
        } else {
            d_rotator = gr::blocks::rotator_cc::make(0.0f);
        }

        connect(self(), 0, d_rotator, 0);
        connect(d_rotator, 0, d_resampler, 0);
        if (d_samples_per_symbol > 4) {
            connect(d_resampler, 0, d_fll, 0);
            connect(d_fll, 0, d_shaping_filter, 0);
        } else {
            connect(d_resampler, 0, d_shaping_filter, 0);
        }

        connect(d_shaping_filter, 0, d_agc, 0);
        connect(d_shaping_filter, 0, self(), 0);
        connect(d_agc, 0, d_costas_pll, 0);
        connect(d_costas_pll, 0, d_symbol_sync, 0);
        connect(d_symbol_sync, 0, d_costas_loop, 0);
        connect(d_costas_loop, 0, d_diff_phasor, 0);
        connect(d_diff_phasor, 0, d_rotate_const, 0);
        connect(d_rotate_const, 0, self(), 1);
        connect(d_rotate_const, 0, d_complex_to_float, 0);
        connect(d_complex_to_float, 0, d_interleave, 0);
        connect(d_complex_to_float, 1, d_interleave, 1);
        connect(d_interleave, 0, d_multiply_const_fec, 0);
        connect(d_multiply_const_fec, 0, d_add_const_fec, 0);
        connect(d_add_const_fec, 0, d_float_to_uchar, 0);
        connect(d_float_to_uchar, 0, d_decode_ccsds, 0);
        connect(d_decode_ccsds, 0, d_descrambler, 0);
        connect(d_descrambler, 0, self(), 2);

    } else if (d_mode == 2) {
        // Mode 2: Multi-carrier with 3 carriers at 4,800 bps each
        d_target_samp_rate = 40000;
        
        // Adjust samples per symbol for 4,800 bps per carrier
        if (d_samples_per_symbol > 4 && d_samples_per_symbol < 125) {
            interpolation = 1;
            decimation = 25;
            d_samples_per_symbol = d_samples_per_symbol * 4 / 25;
        } else if (d_samples_per_symbol >= 125) {
            interpolation = 1;
            decimation = 100;
            d_samples_per_symbol = d_samples_per_symbol / 25;
        } else {
            interpolation = 1;
            decimation = 2;
        }

        // Carrier frequency offsets: -4 kHz, 0, +4 kHz
        float carrier_offsets[3] = {-4000.0f, 0.0f, 4000.0f};

        // Resize vectors for 3 carriers
        d_channel_filters.resize(3);
        d_rotator_mc.resize(3);
        d_resampler_mc.resize(3);
        d_shaping_filter_mc.resize(3);
        d_agc_mc.resize(3);
        d_symbol_sync_mc.resize(3);
        d_costas_loop_mc.resize(3);
        d_diff_phasor_mc.resize(3);
        d_rotate_const_mc.resize(3);
        d_multiply_const_fec_mc.resize(3);
        d_complex_to_float_mc.resize(3);
        d_interleave_mc.resize(3);
        d_float_to_uchar_mc.resize(3);
        d_add_const_fec_mc.resize(3);
        d_decode_ccsds_mc.resize(3);
        d_descrambler_mc.resize(3);

        // Initial resampler
        std::vector<float> taps = gr::filter::firdes::low_pass_2(
            interpolation, d_samp_rate * interpolation, d_target_samp_rate / 2, d_target_samp_rate / 10, 60,
            gr::fft::window::WIN_BLACKMAN_HARRIS);
        d_resampler = gr::filter::rational_resampler_ccf::make(interpolation, decimation, taps);

        std::vector<float> rrc_taps = gr::filter::firdes::root_raised_cosine(
            d_samples_per_symbol, d_samples_per_symbol, 1, 0.35, 11 * d_samples_per_symbol);
        
        float symbol_rate = (float)d_target_samp_rate / (float)d_samples_per_symbol;
        float sps_deviation = 200.0f / symbol_rate;

        // Complex number for rotation calculation (moved outside loop to avoid name conflict)
        const std::complex<float> j(0, 1);
        const std::complex<float> rot(-3 * M_PI / 4, 0);

        for (int i = 0; i < 3; i++) {
            // Channel filter to extract each carrier
            d_channel_filters[i] = gr::filter::fft_filter_ccc::make(
                1,
                gr::filter::firdes::complex_band_pass_2(
                    1, d_target_samp_rate, carrier_offsets[i] - 2000, carrier_offsets[i] + 2000, 200, 60,
                    gr::fft::window::WIN_BLACKMAN_HARRIS));

            // Frequency shift to baseband
            float phase_inc = -2.0f * M_PI * carrier_offsets[i] / d_target_samp_rate;
            d_rotator_mc[i] = gr::blocks::rotator_cc::make(phase_inc);

            // Shaping filter
            d_shaping_filter_mc[i] = gr::filter::fft_filter_ccf::make(1, rrc_taps);
            d_agc_mc[i] = gr::analog::agc2_cc::make(1, 1e-1, 1.0, 1.0);

            // Symbol synchronizer
            d_symbol_sync_mc[i] = gr::digital::symbol_sync_cc::make(
                gr::digital::TED_MOD_MUELLER_AND_MULLER, d_samples_per_symbol, 2 * M_PI / (symbol_rate / 10), 1.0, 0.2869,
                sps_deviation, 1, gr::digital::constellation_dqpsk::make(), gr::digital::IR_MMSE_8TAP);

            d_costas_loop_mc[i] = gr::digital::costas_loop_cc::make(costas_bw, 4, true);
            d_diff_phasor_mc[i] = gr::digital::diff_phasor_cc::make();
            
            d_rotate_const_mc[i] = gr::blocks::multiply_const_cc::make(std::exp(j * rot));
            d_complex_to_float_mc[i] = gr::blocks::complex_to_float::make();
            d_interleave_mc[i] = gr::blocks::interleave::make(4);
            d_multiply_const_fec_mc[i] = gr::blocks::multiply_const_ff::make(48);
            d_float_to_uchar_mc[i] = gr::blocks::float_to_uchar::make();
            d_add_const_fec_mc[i] = gr::blocks::add_const_ff::make(128.0);
            
            gr::fec::code::cc_decoder::sptr decoder = gr::fec::code::cc_decoder::make(80, 7, 2, polys);
            d_decode_ccsds_mc[i] = gr::fec::decoder::make(decoder, 1, 1);
            d_descrambler_mc[i] = gr::digital::descrambler_bb::make(0x8A, 0x7F, 7);
        }

        // Combine decoded outputs from 3 carriers
        d_stream_combiner = gr::blocks::streams_to_stream::make(sizeof(char), 3);

        // Connect blocks for mode 2
        connect(self(), 0, d_resampler, 0);

        for (int i = 0; i < 3; i++) {
            // Each carrier path
            connect(d_resampler, 0, d_channel_filters[i], 0);
            connect(d_channel_filters[i], 0, d_rotator_mc[i], 0);
            connect(d_rotator_mc[i], 0, d_shaping_filter_mc[i], 0);
            connect(d_shaping_filter_mc[i], 0, d_agc_mc[i], 0);
            connect(d_agc_mc[i], 0, d_symbol_sync_mc[i], 0);
            connect(d_symbol_sync_mc[i], 0, d_costas_loop_mc[i], 0);
            connect(d_costas_loop_mc[i], 0, d_diff_phasor_mc[i], 0);
            connect(d_diff_phasor_mc[i], 0, d_rotate_const_mc[i], 0);
            connect(d_rotate_const_mc[i], 0, d_complex_to_float_mc[i], 0);
            connect(d_complex_to_float_mc[i], 0, d_interleave_mc[i], 0);
            connect(d_complex_to_float_mc[i], 1, d_interleave_mc[i], 1);
            connect(d_interleave_mc[i], 0, d_multiply_const_fec_mc[i], 0);
            connect(d_multiply_const_fec_mc[i], 0, d_add_const_fec_mc[i], 0);
            connect(d_add_const_fec_mc[i], 0, d_float_to_uchar_mc[i], 0);
            connect(d_float_to_uchar_mc[i], 0, d_decode_ccsds_mc[i], 0);
            connect(d_decode_ccsds_mc[i], 0, d_descrambler_mc[i], 0);
            connect(d_descrambler_mc[i], 0, d_stream_combiner, i);
        }

        // Outputs: filtered signal (from first carrier), constellation (from first carrier), combined decoded bytes
        connect(d_shaping_filter_mc[0], 0, self(), 0);
        connect(d_rotate_const_mc[0], 0, self(), 1);
        connect(d_stream_combiner, 0, self(), 2);
    }
}

demod_soqpsk_impl::~demod_soqpsk_impl() {}

} // namespace qradiolink
} // namespace gr

