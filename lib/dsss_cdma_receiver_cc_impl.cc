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

#include <gnuradio/qradiolink/dsss_cdma_receiver_cc.h>
#include "dsss_cdma_receiver_cc_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>
#include <algorithm>

namespace gr {
namespace qradiolink {

// Define virtual destructor - this forces vtable generation
dsss_cdma_receiver_cc::~dsss_cdma_receiver_cc() {}

// Base class implementations (should never be called, actual impl is in dsss_cdma_receiver_cc_impl)
void dsss_cdma_receiver_cc::set_spreading_code(const std::vector<int>& spreading_code)
{
    (void)spreading_code; // Suppress unused parameter warning
}

void dsss_cdma_receiver_cc::set_spreading_factor(int spreading_factor)
{
    (void)spreading_factor; // Suppress unused parameter warning
}

dsss_cdma_receiver_cc::sync_state dsss_cdma_receiver_cc::get_sync_state() const
{
    return STATE_ACQUISITION;
}

bool dsss_cdma_receiver_cc::is_locked() const
{
    return false;
}

float dsss_cdma_receiver_cc::get_snr_estimate() const
{
    return 0.0f;
}

float dsss_cdma_receiver_cc::get_interference_estimate() const
{
    return 0.0f;
}

dsss_cdma_receiver_cc::sptr dsss_cdma_receiver_cc::make(
    const std::vector<int>& spreading_code,
    int spreading_factor,
    float correlation_threshold,
    int timing_error_tolerance)
{
    return gnuradio::get_initial_sptr(
        new dsss_cdma_receiver_cc_impl(spreading_code, spreading_factor, correlation_threshold, timing_error_tolerance));
}

dsss_cdma_receiver_cc_impl::dsss_cdma_receiver_cc_impl(
    const std::vector<int>& spreading_code,
    int spreading_factor,
    float correlation_threshold,
    int timing_error_tolerance)
    : dsss_cdma_receiver_cc("dsss_cdma_receiver_cc",
                            gr::io_signature::make(1, 1, sizeof(gr_complex)),
                            gr::io_signature::makev(4, 4, std::vector<int>{sizeof(gr_complex), sizeof(float), sizeof(float), sizeof(float)}),
                            spreading_code,
                            spreading_factor,
                            correlation_threshold,
                            timing_error_tolerance),
      d_spreading_code(spreading_code),
      d_code_length(spreading_code.size()),
      d_spreading_factor(spreading_factor),
      d_correlation_threshold(correlation_threshold),
      d_timing_error_tolerance(timing_error_tolerance),
      d_state(STATE_ACQUISITION),
      d_code_phase(0),
      d_timing_offset(0),
      d_acquisition_counter(0),
      d_lock_counter(0),
      d_early_correlation(0.0f),
      d_prompt_correlation(0.0f),
      d_late_correlation(0.0f),
      d_timing_error(0.0f),
      d_correlation_peak(0.0f),
      d_correlation_avg(0.0f),
      d_is_locked(false),
      d_signal_power(0.0f),
      d_noise_power(0.0f),
      d_snr_db(0.0f),
      d_interference_power(0.0f),
      d_interference_db(0.0f)
{
    if (d_code_length == 0) {
        throw std::invalid_argument("Spreading code cannot be empty");
    }
    if (d_spreading_factor != 32 && d_spreading_factor != 64 && d_spreading_factor != 128 &&
        d_spreading_factor != 256 && d_spreading_factor != 512) {
        throw std::invalid_argument("Spreading factor must be 32, 64, 128, 256, or 512");
    }

    // Convert spreading code to complex
    d_spreading_code_complex.resize(d_code_length);
    for (int i = 0; i < d_code_length; i++) {
        d_spreading_code_complex[i] = gr_complex(static_cast<float>(d_spreading_code[i]), 0.0f);
    }

    // Initialize input buffer
    d_input_buffer.clear();
}

dsss_cdma_receiver_cc_impl::~dsss_cdma_receiver_cc_impl() {}

void dsss_cdma_receiver_cc_impl::forecast(int noutput_items, gr_vector_int& ninput_items_required)
{
    // Need spreading_factor input samples per output symbol
    ninput_items_required[0] = noutput_items * d_spreading_factor;
}

gr_complex dsss_cdma_receiver_cc_impl::correlate(const gr_complex* samples, int offset, int length)
{
    gr_complex correlation = gr_complex(0.0f, 0.0f);

    // Correlate samples with spreading code
    int chips_to_correlate = std::min(length, d_spreading_factor);
    for (int i = 0; i < chips_to_correlate; i++) {
        int code_idx = (d_code_phase + i) % d_code_length;
        correlation += samples[offset + i] * std::conj(d_spreading_code_complex[code_idx]);
    }

    // Normalize by spreading factor
    correlation /= static_cast<float>(d_spreading_factor);

    return correlation;
}

void dsss_cdma_receiver_cc_impl::update_timing()
{
    // Early-late gate timing recovery
    float error = d_early_correlation - d_late_correlation;
    d_timing_error = error * 0.1f;  // Damping factor

    // Update timing offset
    if (std::abs(d_timing_error) > 0.5f) {
        d_timing_offset += (d_timing_error > 0) ? 1 : -1;
        d_timing_offset = std::max(-d_timing_error_tolerance,
                                  std::min(d_timing_error_tolerance, d_timing_offset));
    }
}

void dsss_cdma_receiver_cc_impl::update_lock_detection(float correlation)
{
    float corr_mag = std::abs(correlation);

    // Update running average
    d_correlation_avg = 0.9f * d_correlation_avg + 0.1f * corr_mag;

    // Update peak
    if (corr_mag > d_correlation_peak) {
        d_correlation_peak = corr_mag;
    }

    // Check for lock
    if (corr_mag > d_correlation_threshold) {
        d_lock_counter++;
        if (d_lock_counter >= LOCK_THRESHOLD) {
            d_is_locked = true;
            if (d_state == STATE_ACQUISITION) {
                d_state = STATE_TRACKING;
            }
            if (d_state == STATE_TRACKING && d_lock_counter >= LOCK_THRESHOLD * 2) {
                d_state = STATE_LOCKED;
            }
        }
    } else {
        d_lock_counter = std::max(0, d_lock_counter - 1);
        if (d_lock_counter == 0 && d_state == STATE_LOCKED) {
            d_state = STATE_TRACKING;
            d_is_locked = false;
        }
    }
}

void dsss_cdma_receiver_cc_impl::update_snr_estimate(gr_complex symbol, float correlation)
{
    float symbol_power = std::norm(symbol);
    float corr_mag = std::abs(correlation);

    // Update signal power (from correlation peak)
    d_signal_power = 0.95f * d_signal_power + 0.05f * corr_mag * corr_mag;

    // Estimate noise from correlation sidelobes
    float noise_est = std::max(0.0f, symbol_power - d_signal_power);
    d_noise_power = 0.95f * d_noise_power + 0.05f * noise_est;

    // Calculate SNR in dB
    if (d_noise_power > 0.0f) {
        d_snr_db = 10.0f * std::log10(d_signal_power / d_noise_power);
    } else {
        d_snr_db = 100.0f;  // Very high SNR
    }
}

void dsss_cdma_receiver_cc_impl::update_interference_estimate(float correlation)
{
    // Estimate multi-user interference from correlation sidelobes
    // In CDMA, interference appears as reduced correlation magnitude
    // compared to the ideal case
    float ideal_correlation = 1.0f;  // Ideal case: perfect correlation
    float corr_mag = std::abs(correlation);
    
    // Interference reduces correlation magnitude
    float interference_est = std::max(0.0f, ideal_correlation - corr_mag);
    d_interference_power = 0.95f * d_interference_power + 0.05f * interference_est;

    // Calculate interference in dB
    if (d_interference_power > 0.0f) {
        d_interference_db = 10.0f * std::log10(d_interference_power);
    } else {
        d_interference_db = -100.0f;  // No interference
    }
}

void dsss_cdma_receiver_cc_impl::set_spreading_code(const std::vector<int>& spreading_code)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    d_spreading_code = spreading_code;
    d_code_length = spreading_code.size();
    if (d_code_length == 0) {
        throw std::invalid_argument("Spreading code cannot be empty");
    }

    d_spreading_code_complex.resize(d_code_length);
    for (int i = 0; i < d_code_length; i++) {
        d_spreading_code_complex[i] = gr_complex(static_cast<float>(d_spreading_code[i]), 0.0f);
    }

    // Reset state
    d_code_phase = 0;
    d_state = STATE_ACQUISITION;
    d_lock_counter = 0;
    d_is_locked = false;
}

void dsss_cdma_receiver_cc_impl::set_spreading_factor(int spreading_factor)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    if (spreading_factor != 32 && spreading_factor != 64 && spreading_factor != 128 &&
        spreading_factor != 256 && spreading_factor != 512) {
        throw std::invalid_argument("Spreading factor must be 32, 64, 128, 256, or 512");
    }
    d_spreading_factor = spreading_factor;
}

dsss_cdma_receiver_cc::sync_state dsss_cdma_receiver_cc_impl::get_sync_state() const
{
    std::lock_guard<std::mutex> lock(d_mutex);
    return d_state;
}

bool dsss_cdma_receiver_cc_impl::is_locked() const
{
    std::lock_guard<std::mutex> lock(d_mutex);
    return d_is_locked;
}

float dsss_cdma_receiver_cc_impl::get_snr_estimate() const
{
    std::lock_guard<std::mutex> lock(d_mutex);
    return d_snr_db;
}

float dsss_cdma_receiver_cc_impl::get_interference_estimate() const
{
    std::lock_guard<std::mutex> lock(d_mutex);
    return d_interference_db;
}

int dsss_cdma_receiver_cc_impl::general_work(
    int noutput_items,
    gr_vector_int& ninput_items,
    gr_vector_const_void_star& input_items,
    gr_vector_void_star& output_items)
{
    const gr_complex* in = (const gr_complex*)input_items[0];
    gr_complex* out_symbols = (gr_complex*)output_items[0];
    float* out_lock = (float*)output_items[1];
    float* out_snr = (float*)output_items[2];
    float* out_interference = (float*)output_items[3];

    std::lock_guard<std::mutex> lock(d_mutex);

    int ninput_items_available = ninput_items[0];
    int ninput_items_needed = noutput_items * d_spreading_factor;
    int ninput_items_used = std::min(ninput_items_available, ninput_items_needed);
    int actual_output_items = ninput_items_used / d_spreading_factor;

    int output_idx = 0;

    for (int sym = 0; sym < actual_output_items; sym++) {
        int input_offset = sym * d_spreading_factor;

        if (input_offset + d_spreading_factor > ninput_items_used) {
            break;
        }

        if (d_state == STATE_ACQUISITION) {
            // Search for code alignment
            float best_correlation = 0.0f;
            int best_phase = d_code_phase;

            // Search over code phases
            for (int phase = 0; phase < d_code_length; phase++) {
                d_code_phase = phase;
                gr_complex corr = correlate(in, input_offset, d_spreading_factor);
                float corr_mag = std::abs(corr);

                if (corr_mag > best_correlation) {
                    best_correlation = corr_mag;
                    best_phase = phase;
                }
            }

            d_code_phase = best_phase;
            d_prompt_correlation = best_correlation;
            update_lock_detection(best_correlation);

            d_acquisition_counter++;
            if (d_acquisition_counter > ACQUISITION_TIMEOUT) {
                d_acquisition_counter = 0;  // Reset and try again
            }

            // Despread using best phase
            gr_complex despread = correlate(in, input_offset, d_spreading_factor);
            out_symbols[output_idx] = despread;
            update_interference_estimate(d_prompt_correlation);

        } else {
            // STATE_TRACKING or STATE_LOCKED - use early-late gate

            // Early correlation (one sample early)
            int early_offset = std::max(0, input_offset - 1);
            d_early_correlation = std::abs(correlate(in, early_offset, d_spreading_factor));

            // Prompt correlation (on-time)
            d_prompt_correlation = std::abs(correlate(in, input_offset, d_spreading_factor));

            // Late correlation (one sample late)
            int late_offset = std::min(ninput_items_used - d_spreading_factor, input_offset + 1);
            d_late_correlation = std::abs(correlate(in, late_offset, d_spreading_factor));

            // Update timing
            update_timing();

            // Despread using prompt correlation
            gr_complex despread = correlate(in, input_offset + d_timing_offset, d_spreading_factor);
            out_symbols[output_idx] = despread;

            // Update lock detection, SNR, and interference
            update_lock_detection(d_prompt_correlation);
            update_snr_estimate(despread, d_prompt_correlation);
            update_interference_estimate(d_prompt_correlation);

            // Advance code phase for next symbol
            d_code_phase = (d_code_phase + d_spreading_factor) % d_code_length;
        }

        // Output lock status, SNR, and interference
        out_lock[output_idx] = d_is_locked ? 1.0f : 0.0f;
        out_snr[output_idx] = d_snr_db;
        out_interference[output_idx] = d_interference_db;

        output_idx++;
    }

    // Consume input items
    consume(0, output_idx * d_spreading_factor);

    return output_idx;
}

} // namespace qradiolink
} // namespace gr

