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

#include <gnuradio/qradiolink/dsss_cdma_transmitter_cc.h>
#include "dsss_cdma_transmitter_cc_impl.h"
#include <gnuradio/io_signature.h>
#include <algorithm>
#include <cmath>

namespace gr {
namespace qradiolink {

// Define virtual destructor - this forces vtable generation
dsss_cdma_transmitter_cc::~dsss_cdma_transmitter_cc() {}

// Base class implementations (should never be called, actual impl is in dsss_cdma_transmitter_cc_impl)
void dsss_cdma_transmitter_cc::set_spreading_codes(const std::vector<std::vector<int>>& spreading_codes)
{
    (void)spreading_codes; // Suppress unused parameter warning
}

void dsss_cdma_transmitter_cc::set_spreading_factor(int spreading_factor)
{
    (void)spreading_factor; // Suppress unused parameter warning
}

void dsss_cdma_transmitter_cc::set_num_users(int num_users)
{
    (void)num_users; // Suppress unused parameter warning
}

void dsss_cdma_transmitter_cc::set_normalize_power(bool normalize_power)
{
    (void)normalize_power; // Suppress unused parameter warning
}

dsss_cdma_transmitter_cc::sptr dsss_cdma_transmitter_cc::make(
    const std::vector<std::vector<int>>& spreading_codes,
    int spreading_factor,
    int num_users,
    bool normalize_power)
{
    return gnuradio::get_initial_sptr(
        new dsss_cdma_transmitter_cc_impl(spreading_codes, spreading_factor, num_users, normalize_power));
}

dsss_cdma_transmitter_cc_impl::dsss_cdma_transmitter_cc_impl(
    const std::vector<std::vector<int>>& spreading_codes,
    int spreading_factor,
    int num_users,
    bool normalize_power)
    : dsss_cdma_transmitter_cc("dsss_cdma_transmitter_cc",
                               gr::io_signature::make(1, num_users, sizeof(gr_complex)),
                               gr::io_signature::make(1, 1, sizeof(gr_complex)),
                               spreading_codes,
                               spreading_factor,
                               num_users,
                               normalize_power),
      d_spreading_codes(spreading_codes),
      d_spreading_factor(spreading_factor),
      d_num_users(num_users),
      d_normalize_power(normalize_power),
      d_chip_indices(num_users, 0)
{
    if (static_cast<int>(spreading_codes.size()) != num_users) {
        throw std::invalid_argument("Number of spreading codes must match number of users");
    }
    if (spreading_factor != 32 && spreading_factor != 64 && spreading_factor != 128 &&
        spreading_factor != 256 && spreading_factor != 512) {
        throw std::invalid_argument("Spreading factor must be 32, 64, 128, 256, or 512");
    }

    d_code_lengths.resize(num_users);
    for (int i = 0; i < num_users; i++) {
        if (spreading_codes[i].empty()) {
            throw std::invalid_argument("Spreading code cannot be empty");
        }
        d_code_lengths[i] = spreading_codes[i].size();
    }

    update_spreading_codes_complex();
    update_power_normalization();
}

dsss_cdma_transmitter_cc_impl::~dsss_cdma_transmitter_cc_impl() {}

void dsss_cdma_transmitter_cc_impl::update_spreading_codes_complex()
{
    d_spreading_codes_complex.resize(d_num_users);
    for (int user = 0; user < d_num_users; user++) {
        int code_len = d_code_lengths[user];
        d_spreading_codes_complex[user].resize(code_len);
        for (int i = 0; i < code_len; i++) {
            // Convert +1/-1 to complex
            d_spreading_codes_complex[user][i] =
                gr_complex(static_cast<float>(d_spreading_codes[user][i]), 0.0f);
        }
    }
}

void dsss_cdma_transmitter_cc_impl::update_power_normalization()
{
    if (d_normalize_power) {
        // Normalize by number of users to maintain total power
        d_power_normalization_factor = 1.0f / std::sqrt(static_cast<float>(d_num_users));
    } else {
        d_power_normalization_factor = 1.0f;
    }
}

void dsss_cdma_transmitter_cc_impl::set_spreading_codes(
    const std::vector<std::vector<int>>& spreading_codes)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    if (static_cast<int>(spreading_codes.size()) != d_num_users) {
        throw std::invalid_argument("Number of spreading codes must match number of users");
    }
    d_spreading_codes = spreading_codes;
    for (int i = 0; i < d_num_users; i++) {
        if (spreading_codes[i].empty()) {
            throw std::invalid_argument("Spreading code cannot be empty");
        }
        d_code_lengths[i] = spreading_codes[i].size();
        d_chip_indices[i] = 0; // Reset chip index
    }
    update_spreading_codes_complex();
}

void dsss_cdma_transmitter_cc_impl::set_spreading_factor(int spreading_factor)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    if (spreading_factor != 32 && spreading_factor != 64 && spreading_factor != 128 &&
        spreading_factor != 256 && spreading_factor != 512) {
        throw std::invalid_argument("Spreading factor must be 32, 64, 128, 256, or 512");
    }
    d_spreading_factor = spreading_factor;
    // Reset all chip indices
    std::fill(d_chip_indices.begin(), d_chip_indices.end(), 0);
}

void dsss_cdma_transmitter_cc_impl::set_num_users(int num_users)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    if (num_users < 1) {
        throw std::invalid_argument("Number of users must be at least 1");
    }
    d_num_users = num_users;
    d_chip_indices.resize(num_users, 0);
    d_code_lengths.resize(num_users);
    update_power_normalization();
}

void dsss_cdma_transmitter_cc_impl::set_normalize_power(bool normalize_power)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    d_normalize_power = normalize_power;
    update_power_normalization();
}

void dsss_cdma_transmitter_cc_impl::forecast(int noutput_items,
                                             gr_vector_int& ninput_items_required)
{
    // Each output symbol requires spreading_factor chips
    // Each input symbol from each user contributes spreading_factor chips
    int ninput_symbols = noutput_items / d_spreading_factor;
    for (int i = 0; i < d_num_users; i++) {
        ninput_items_required[i] = ninput_symbols;
    }
}

int dsss_cdma_transmitter_cc_impl::general_work(
    int noutput_items,
    gr_vector_int& ninput_items,
    gr_vector_const_void_star& input_items,
    gr_vector_void_star& output_items)
{
    gr_complex* out = (gr_complex*)output_items[0];

    std::lock_guard<std::mutex> lock(d_mutex);

    // Determine how many symbols we can process
    int ninput_symbols = noutput_items / d_spreading_factor;
    // Find minimum available input across all users
    for (int i = 0; i < d_num_users; i++) {
        ninput_symbols = std::min(ninput_symbols, ninput_items[i]);
    }

    // Check if all inputs are done (no items available)
    bool all_inputs_done = true;
    for (int i = 0; i < d_num_users; i++) {
        if (ninput_items[i] > 0) {
            all_inputs_done = false;
            break;
        }
    }
    
    if (all_inputs_done) {
        // All inputs are done, signal completion (WORK_DONE = -1)
        return -1;
    }

    if (ninput_symbols <= 0) {
        return 0;
    }

    // Initialize output to zero
    std::fill(out, out + noutput_items, gr_complex(0.0f, 0.0f));

    // Process each user
    for (int user = 0; user < d_num_users; user++) {
        const gr_complex* in_user = (const gr_complex*)input_items[user];
        int code_len = d_code_lengths[user];
        int chip_idx = d_chip_indices[user];

        // Spread and accumulate this user's signal
        for (int sym = 0; sym < ninput_symbols; sym++) {
            gr_complex symbol = in_user[sym];

            // Spread this symbol
            for (int chip = 0; chip < d_spreading_factor; chip++) {
                int output_idx = sym * d_spreading_factor + chip;
                if (output_idx >= noutput_items) {
                    break;
                }

                // Get PN chip (wraparound if needed)
                int pn_idx = (chip_idx + chip) % code_len;
                gr_complex pn_chip = d_spreading_codes_complex[user][pn_idx];

                // Multiply symbol by PN chip and add to output
                out[output_idx] += symbol * pn_chip * d_power_normalization_factor;
            }

            // Advance chip index for next symbol
            chip_idx = (chip_idx + d_spreading_factor) % code_len;
        }

        d_chip_indices[user] = chip_idx;
    }

    // Consume input from all users
    for (int i = 0; i < d_num_users; i++) {
        consume(i, ninput_symbols);
    }

    return ninput_symbols * d_spreading_factor;
}

} // namespace qradiolink
} // namespace gr

