/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_QRADIOLINK_GRIDENT_MODE_CONTROL_IMPL_H
#define INCLUDED_QRADIOLINK_GRIDENT_MODE_CONTROL_IMPL_H

#include <gnuradio/qradiolink/grident_mode_control.h>
#include <gnuradio/qradiolink/grident_mode_map.h>
#include <string>

namespace gr {
namespace qradiolink {

class grident_mode_control_impl : public grident_mode_control
{
private:
    grident::mode_selection d_sel{};
    std::string             d_demod_block{"unknown"};
    std::string             d_mod_block{"unknown"};

    void handle_preamble_msg(pmt::pmt_t msg);
    void apply_selection(const grident::mode_selection& sel);
    pmt::pmt_t make_route_dict() const;

public:
    grident_mode_control_impl();
    ~grident_mode_control_impl() override;

    std::uint16_t mode_id() const override { return d_sel.preamble.mode_id; }
    bool          digital() const override { return d_sel.preamble.digital; }
    bool          encrypted() const override { return d_sel.preamble.encrypted; }
    bool          metadata_present() const override { return d_sel.preamble.metadata_present; }
    bool          route_valid() const override { return d_sel.route_valid; }
    std::string   demod_block() const override { return d_demod_block; }
    std::string   mod_block() const override { return d_mod_block; }
};

} // namespace qradiolink
} // namespace gr

#endif
