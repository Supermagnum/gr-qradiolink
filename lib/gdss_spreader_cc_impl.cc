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

#include <gnuradio/qradiolink/gdss_spreader_cc.h>
#include "gdss_spreader_cc_impl.h"
#include <gnuradio/io_signature.h>
#include <chrono>
#include <cmath>

namespace gr {
namespace qradiolink {

// Define virtual destructor - this forces vtable generation
gdss_spreader_cc::~gdss_spreader_cc() {}

// Base class implementations (should never be called, actual impl is in gdss_spreader_cc_impl)
void gdss_spreader_cc::set_spreading_sequence(const std::vector<float>& sequence)
{
    (void)sequence; // Suppress unused parameter warning
}

void gdss_spreader_cc::set_chips_per_symbol(int chips_per_symbol)
{
    (void)chips_per_symbol; // Suppress unused parameter warning
}

void gdss_spreader_cc::regenerate_sequence(float variance, unsigned int seed)
{
    (void)variance; // Suppress unused parameter warning
    (void)seed; // Suppress unused parameter warning
}

gdss_spreader_cc::sptr gdss_spreader_cc::make(int sequence_length,
                                               int chips_per_symbol,
                                               float variance,
                                               unsigned int seed)
{
    return gnuradio::get_initial_sptr(
        new gdss_spreader_cc_impl(sequence_length, chips_per_symbol, variance, seed));
}

gdss_spreader_cc_impl::gdss_spreader_cc_impl(int sequence_length,
                                               int chips_per_symbol,
                                               float variance,
                                               unsigned int seed)
    : gdss_spreader_cc("gdss_spreader_cc",
                       gr::io_signature::make(1, 1, sizeof(gr_complex)),
                       gr::io_signature::make(1, 1, sizeof(gr_complex)),
                       sequence_length,
                       chips_per_symbol,
                       variance,
                       seed),
      d_sequence_length(sequence_length),
      d_chips_per_symbol(chips_per_symbol),
      d_chip_index(0),
      d_variance(variance),
      d_seed(seed == 0 ? static_cast<unsigned int>(
                             std::chrono::system_clock::now().time_since_epoch().count())
                       : seed),
      d_rng(d_seed),
      d_gaussian(0.0f, std::sqrt(variance))
{
    if (d_sequence_length <= 0) {
        throw std::invalid_argument("Sequence length must be positive");
    }
    if (d_chips_per_symbol <= 0) {
        throw std::invalid_argument("Chips per symbol must be positive");
    }
    if (d_variance <= 0.0f) {
        throw std::invalid_argument("Variance must be positive");
    }

    generate_sequence();
    update_sequence_complex();
}

gdss_spreader_cc_impl::~gdss_spreader_cc_impl() {}

void gdss_spreader_cc_impl::generate_sequence()
{
    d_spreading_sequence.resize(d_sequence_length);
    for (int i = 0; i < d_sequence_length; i++) {
        d_spreading_sequence[i] = d_gaussian(d_rng);
    }
}

void gdss_spreader_cc_impl::update_sequence_complex()
{
    d_spreading_sequence_complex.resize(d_sequence_length);
    for (int i = 0; i < d_sequence_length; i++) {
        // Convert Gaussian value to complex (real part only for now)
        d_spreading_sequence_complex[i] = gr_complex(d_spreading_sequence[i], 0.0f);
    }
}

void gdss_spreader_cc_impl::set_spreading_sequence(const std::vector<float>& sequence)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    if (static_cast<int>(sequence.size()) != d_sequence_length) {
        throw std::invalid_argument("Sequence length must match");
    }
    d_spreading_sequence = sequence;
    update_sequence_complex();
    d_chip_index = 0; // Reset chip index
}

void gdss_spreader_cc_impl::set_chips_per_symbol(int chips_per_symbol)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    if (chips_per_symbol <= 0) {
        throw std::invalid_argument("Chips per symbol must be positive");
    }
    d_chips_per_symbol = chips_per_symbol;
    set_interpolation(chips_per_symbol);
}

void gdss_spreader_cc_impl::regenerate_sequence(float variance, unsigned int seed)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    if (variance <= 0.0f) {
        throw std::invalid_argument("Variance must be positive");
    }
    d_variance = variance;
    d_seed = seed == 0 ? static_cast<unsigned int>(
                             std::chrono::system_clock::now().time_since_epoch().count())
                       : seed;
    d_rng.seed(d_seed);
    d_gaussian = std::normal_distribution<float>(0.0f, std::sqrt(variance));
    generate_sequence();
    update_sequence_complex();
    d_chip_index = 0;
}

int gdss_spreader_cc_impl::work(int noutput_items,
                                 gr_vector_const_void_star& input_items,
                                 gr_vector_void_star& output_items)
{
    const gr_complex* in = (const gr_complex*)input_items[0];
    gr_complex* out = (gr_complex*)output_items[0];

    std::lock_guard<std::mutex> lock(d_mutex);

    int ninput_items = noutput_items / d_chips_per_symbol;
    int output_idx = 0;

    for (int sym_idx = 0; sym_idx < ninput_items; sym_idx++) {
        gr_complex symbol = in[sym_idx];

        // Spread this symbol: repeat it chips_per_symbol times and multiply by Gaussian sequence
        for (int chip = 0; chip < d_chips_per_symbol; chip++) {
            // Get spreading chip (wraparound if needed)
            int seq_idx = (d_chip_index + chip) % d_sequence_length;
            gr_complex spreading_chip = d_spreading_sequence_complex[seq_idx];

            // Multiply symbol by spreading chip
            out[output_idx++] = symbol * spreading_chip;
        }

        // Advance chip index for next symbol
        d_chip_index = (d_chip_index + d_chips_per_symbol) % d_sequence_length;
    }

    return output_idx;
}

} // namespace qradiolink
} // namespace gr

