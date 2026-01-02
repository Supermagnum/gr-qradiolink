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

#include <gnuradio/qradiolink/dsss_spreader_cc.h>
#include "dsss_spreader_cc_impl.h"
#include <gnuradio/io_signature.h>
#include <cstring>

namespace gr {
namespace qradiolink {

// Define virtual destructor - this forces vtable generation
dsss_spreader_cc::~dsss_spreader_cc() {}

// Base class implementations (should never be called, actual impl is in dsss_spreader_cc_impl)
void dsss_spreader_cc::set_pn_sequence(const std::vector<int>& pn_sequence)
{
    (void)pn_sequence; // Suppress unused parameter warning
}

void dsss_spreader_cc::set_chips_per_symbol(int chips_per_symbol)
{
    (void)chips_per_symbol; // Suppress unused parameter warning
}

dsss_spreader_cc::sptr dsss_spreader_cc::make(const std::vector<int>& pn_sequence, int chips_per_symbol)
{
    return gnuradio::get_initial_sptr(
        new dsss_spreader_cc_impl(pn_sequence, chips_per_symbol));
}

dsss_spreader_cc_impl::dsss_spreader_cc_impl(const std::vector<int>& pn_sequence, int chips_per_symbol)
    : dsss_spreader_cc("dsss_spreader_cc",
                       gr::io_signature::make(1, 1, sizeof(gr_complex)),
                       gr::io_signature::make(1, 1, sizeof(gr_complex)),
                       pn_sequence,
                       chips_per_symbol),
      d_pn_sequence(pn_sequence),
      d_code_length(pn_sequence.size()),
      d_chips_per_symbol(chips_per_symbol),
      d_chip_index(0)
{
    if (d_code_length == 0) {
        throw std::invalid_argument("PN sequence cannot be empty");
    }
    if (d_chips_per_symbol <= 0) {
        throw std::invalid_argument("Chips per symbol must be positive");
    }

    update_pn_sequence_complex();
}

dsss_spreader_cc_impl::~dsss_spreader_cc_impl() {}

void dsss_spreader_cc_impl::update_pn_sequence_complex()
{
    d_pn_sequence_complex.resize(d_code_length);
    for (int i = 0; i < d_code_length; i++) {
        // Convert +1/-1 to complex
        d_pn_sequence_complex[i] = gr_complex(static_cast<float>(d_pn_sequence[i]), 0.0f);
    }
}

void dsss_spreader_cc_impl::set_pn_sequence(const std::vector<int>& pn_sequence)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    d_pn_sequence = pn_sequence;
    d_code_length = pn_sequence.size();
    if (d_code_length == 0) {
        throw std::invalid_argument("PN sequence cannot be empty");
    }
    update_pn_sequence_complex();
    d_chip_index = 0; // Reset chip index
}

void dsss_spreader_cc_impl::set_chips_per_symbol(int chips_per_symbol)
{
    std::lock_guard<std::mutex> lock(d_mutex);
    if (chips_per_symbol <= 0) {
        throw std::invalid_argument("Chips per symbol must be positive");
    }
    d_chips_per_symbol = chips_per_symbol;
    set_interpolation(chips_per_symbol);
}

int dsss_spreader_cc_impl::work(int noutput_items,
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

        // Spread this symbol: repeat it chips_per_symbol times and multiply by PN sequence
        for (int chip = 0; chip < d_chips_per_symbol; chip++) {
            // Get PN chip (wraparound if needed)
            int pn_idx = (d_chip_index + chip) % d_code_length;
            gr_complex pn_chip = d_pn_sequence_complex[pn_idx];

            // Multiply symbol by PN chip
            out[output_idx++] = symbol * pn_chip;
        }

        // Advance chip index for next symbol
        d_chip_index = (d_chip_index + d_chips_per_symbol) % d_code_length;
    }

    return output_idx;
}

} // namespace qradiolink
} // namespace gr

