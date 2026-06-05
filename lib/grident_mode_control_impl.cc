/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "grident_mode_control_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/pmt_fmt.h>

namespace gr {
namespace qradiolink {

static const pmt::pmt_t PORT_PREAMBLE_IN = pmt::mp("preamble_in");
static const pmt::pmt_t PORT_ROUTE_OUT  = pmt::mp("route_out");

grident_mode_control::sptr grident_mode_control::make() { return gnuradio::get_initial_sptr(new grident_mode_control_impl()); }

grident_mode_control_impl::grident_mode_control_impl()
    : grident_mode_control("grident_mode_control", gr::io_signature::make(0, 0, 0), gr::io_signature::make(0, 0, 0))
{
    message_port_register_in(PORT_PREAMBLE_IN);
    message_port_register_out(PORT_ROUTE_OUT);
    set_msg_handler(PORT_PREAMBLE_IN, [this](pmt::pmt_t msg) { handle_preamble_msg(msg); });
}

grident_mode_control_impl::~grident_mode_control_impl() {}

pmt::pmt_t grident_mode_control_impl::make_route_dict() const
{
    pmt::pmt_t d = pmt::make_dict();
    d            = pmt::dict_add(d, pmt::mp("mode_id"), pmt::from_uint64(d_sel.preamble.mode_id));
    d            = pmt::dict_add(d, pmt::mp("digital"), pmt::from_bool(d_sel.preamble.digital));
    d            = pmt::dict_add(d, pmt::mp("encrypted"), pmt::from_bool(d_sel.preamble.encrypted));
    d            = pmt::dict_add(d, pmt::mp("metadata_present"), pmt::from_bool(d_sel.preamble.metadata_present));
    d            = pmt::dict_add(d, pmt::mp("route_valid"), pmt::from_bool(d_sel.route_valid));
    d            = pmt::dict_add(d, pmt::mp("active_demod_route"), pmt::from_uint64(static_cast<std::uint64_t>(d_sel.demod_route)));
    d            = pmt::dict_add(d, pmt::mp("active_mod_route"), pmt::from_uint64(static_cast<std::uint64_t>(d_sel.mod_route)));
    d            = pmt::dict_add(d, pmt::mp("demod_block"), pmt::mp(d_demod_block));
    d            = pmt::dict_add(d, pmt::mp("mod_block"), pmt::mp(d_mod_block));
    return d;
}

void grident_mode_control_impl::apply_selection(const grident::mode_selection& sel)
{
    d_sel        = sel;
    d_demod_block = std::string(grident::route_block_name(sel.demod_route));
    d_mod_block   = std::string(grident::route_block_name(sel.mod_route));
    message_port_pub(PORT_ROUTE_OUT, make_route_dict());
}

void grident_mode_control_impl::handle_preamble_msg(pmt::pmt_t msg)
{
    if (pmt::is_symbol(msg)) {
        const std::string json = pmt::symbol_to_string(msg);
        const auto        sel  = grident::selection_from_json(json);
        if (sel) {
            apply_selection(*sel);
        }
        return;
    }

    if (!pmt::is_dict(msg)) {
        return;
    }

    const pmt::pmt_t topic_k = pmt::mp("topic");
    const pmt::pmt_t json_k  = pmt::mp("json");
    if (pmt::dict_has_key(msg, topic_k) && pmt::dict_has_key(msg, json_k)) {
        const std::string topic = pmt::symbol_to_string(pmt::dict_ref(msg, topic_k, pmt::PMT_NIL));
        const std::string json  = pmt::symbol_to_string(pmt::dict_ref(msg, json_k, pmt::PMT_NIL));
        const auto        parsed = grident::parse_preamble_multipart(topic, json);
        if (parsed) {
            apply_selection(grident::selection_from_preamble(*parsed));
        }
        return;
    }

    if (pmt::dict_has_key(msg, json_k)) {
        const std::string json = pmt::symbol_to_string(pmt::dict_ref(msg, json_k, pmt::PMT_NIL));
        const auto        sel  = grident::selection_from_json(json);
        if (sel) {
            apply_selection(*sel);
        }
        return;
    }

    if (pmt::dict_has_key(msg, pmt::mp("preamble_json"))) {
        const std::string json =
            pmt::symbol_to_string(pmt::dict_ref(msg, pmt::mp("preamble_json"), pmt::PMT_NIL));
        const auto sel = grident::selection_from_json(json);
        if (sel) {
            apply_selection(*sel);
        }
    }
}

} // namespace qradiolink
} // namespace gr
