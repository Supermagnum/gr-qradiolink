/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_QRADIOLINK_GRIDENT_MODE_MAP_H
#define INCLUDED_QRADIOLINK_GRIDENT_MODE_MAP_H

#include <gnuradio/qradiolink/grident_zmq_protocol.h>

#include <cstdint>
#include <optional>
#include <string_view>

namespace gr {
namespace qradiolink {
namespace grident {

enum class modem_route : std::uint8_t {
    unknown = 0,
    demod_am,
    demod_ssb_usb,
    demod_ssb_lsb,
    demod_nbfm,
    demod_wbfm,
    demod_2fsk,
    demod_4fsk,
    demod_8fsk,
    demod_bpsk,
    demod_qpsk,
    demod_gmsk,
    demod_dmr,
    demod_dpmr,
    demod_nxdn,
    demod_m17,
    demod_freedv,
    demod_mmdvm_multi,
    demod_dstar,
    demod_ysf,
    demod_p25,
    mod_am,
    mod_ssb,
    mod_nbfm,
    mod_wbfm,
    mod_2fsk,
    mod_4fsk,
    mod_8fsk,
    mod_bpsk,
    mod_qpsk,
    mod_gmsk,
    mod_dmr,
    mod_dpmr,
    mod_nxdn,
    mod_m17,
    mod_freedv,
    mod_mmdvm_multi2,
};

inline std::string_view route_block_name(modem_route r) noexcept
{
    switch (r) {
    case modem_route::demod_am:
        return "demod_am";
    case modem_route::demod_ssb_usb:
    case modem_route::demod_ssb_lsb:
        return "demod_ssb";
    case modem_route::demod_nbfm:
        return "demod_nbfm";
    case modem_route::demod_wbfm:
        return "demod_wbfm";
    case modem_route::demod_2fsk:
        return "demod_2fsk";
    case modem_route::demod_4fsk:
        return "demod_4fsk";
    case modem_route::demod_8fsk:
        return "demod_8fsk";
    case modem_route::demod_bpsk:
        return "demod_bpsk";
    case modem_route::demod_qpsk:
        return "demod_qpsk";
    case modem_route::demod_gmsk:
        return "demod_gmsk";
    case modem_route::demod_dmr:
        return "demod_dmr";
    case modem_route::demod_dpmr:
        return "demod_dpmr";
    case modem_route::demod_nxdn:
        return "demod_nxdn";
    case modem_route::demod_m17:
        return "demod_m17";
    case modem_route::demod_freedv:
        return "demod_freedv";
    case modem_route::demod_mmdvm_multi:
        return "demod_mmdvm_multi";
    case modem_route::demod_dstar:
        return "dstar_decoder";
    case modem_route::demod_ysf:
        return "ysf_decoder";
    case modem_route::demod_p25:
        return "p25_decoder";
    case modem_route::mod_am:
        return "mod_am";
    case modem_route::mod_ssb:
        return "mod_ssb";
    case modem_route::mod_nbfm:
        return "mod_nbfm";
    case modem_route::mod_wbfm:
        return "mod_wbfm";
    case modem_route::mod_2fsk:
        return "mod_2fsk";
    case modem_route::mod_4fsk:
        return "mod_4fsk";
    case modem_route::mod_8fsk:
        return "mod_8fsk";
    case modem_route::mod_bpsk:
        return "mod_bpsk";
    case modem_route::mod_qpsk:
        return "mod_qpsk";
    case modem_route::mod_gmsk:
        return "mod_gmsk";
    case modem_route::mod_dmr:
        return "mod_dmr";
    case modem_route::mod_dpmr:
        return "mod_dpmr";
    case modem_route::mod_nxdn:
        return "mod_nxdn";
    case modem_route::mod_m17:
        return "mod_m17";
    case modem_route::mod_freedv:
        return "mod_freedv";
    case modem_route::mod_mmdvm_multi2:
        return "mod_mmdvm_multi2";
    default:
        return "unknown";
    }
}

inline modem_route demod_route_for_mode_id(std::uint16_t mode_id) noexcept
{
    switch (mode_id) {
    case 1:
    case 2:
    case 6:
    case 62:
    case 60:
    case 61:
        return modem_route::demod_am;
    case 3:
    case 4:
    case 63:
        return modem_route::demod_ssb_usb;
    case 5:
        return modem_route::demod_ssb_lsb;
    case 10:
    case 11:
    case 12:
    case 13:
    case 33:
    case 34:
    case 43:
    case 44:
        return modem_route::demod_wbfm;
    case 20:
    case 21:
    case 22:
    case 30:
    case 31:
    case 32:
    case 40:
    case 41:
    case 42:
    case 50:
    case 51:
    case 52:
    case 110:
    case 111:
    case 112:
    case 113:
    case 153:
        return modem_route::demod_nbfm;
    case 100:
    case 101:
    case 102:
    case 106:
    case 109:
        return modem_route::demod_dmr;
    case 103:
    case 115:
        return modem_route::demod_dstar;
    case 104:
    case 105:
    case 114:
        return modem_route::demod_ysf;
    case 107:
        return modem_route::demod_nxdn;
    case 108:
        return modem_route::demod_dpmr;
    case 120:
    case 121:
        return modem_route::demod_m17;
    case 122:
    case 123:
    case 124:
        return modem_route::demod_freedv;
    case 150:
    case 151:
    case 155:
    case 159:
        return modem_route::demod_2fsk;
    case 158:
        return modem_route::demod_bpsk;
    default:
        if (mode_id >= 300 && mode_id <= 498) {
            return modem_route::demod_qpsk;
        }
        return modem_route::unknown;
    }
}

inline modem_route mod_route_for_mode_id(std::uint16_t mode_id) noexcept
{
    switch (demod_route_for_mode_id(mode_id)) {
    case modem_route::demod_am:
        return modem_route::mod_am;
    case modem_route::demod_ssb_usb:
    case modem_route::demod_ssb_lsb:
        return modem_route::mod_ssb;
    case modem_route::demod_nbfm:
        return modem_route::mod_nbfm;
    case modem_route::demod_wbfm:
        return modem_route::mod_wbfm;
    case modem_route::demod_2fsk:
        return modem_route::mod_2fsk;
    case modem_route::demod_4fsk:
        return modem_route::mod_4fsk;
    case modem_route::demod_8fsk:
        return modem_route::mod_8fsk;
    case modem_route::demod_bpsk:
        return modem_route::mod_bpsk;
    case modem_route::demod_qpsk:
        return modem_route::mod_qpsk;
    case modem_route::demod_gmsk:
        return modem_route::mod_gmsk;
    case modem_route::demod_dmr:
        return modem_route::mod_dmr;
    case modem_route::demod_dpmr:
        return modem_route::mod_dpmr;
    case modem_route::demod_nxdn:
        return modem_route::mod_nxdn;
    case modem_route::demod_m17:
        return modem_route::mod_m17;
    case modem_route::demod_freedv:
        return modem_route::mod_freedv;
    case modem_route::demod_mmdvm_multi:
    case modem_route::demod_ysf:
    case modem_route::demod_p25:
        return modem_route::mod_mmdvm_multi2;
    case modem_route::demod_dstar:
        return modem_route::mod_dmr;
    default:
        return modem_route::unknown;
    }
}

struct mode_selection {
    preamble_result preamble{};
    modem_route     demod_route{modem_route::unknown};
    modem_route     mod_route{modem_route::unknown};
    bool            route_valid{false};
};

inline mode_selection selection_from_preamble(const preamble_result& p) noexcept
{
    mode_selection sel{};
    sel.preamble    = p;
    sel.demod_route = demod_route_for_mode_id(p.mode_id);
    sel.mod_route   = mod_route_for_mode_id(p.mode_id);
    sel.route_valid = sel.demod_route != modem_route::unknown;
    return sel;
}

inline std::optional<mode_selection> selection_from_json(std::string_view json)
{
    const auto parsed = parse_preamble_json(json);
    if (!parsed) {
        return std::nullopt;
    }
    return selection_from_preamble(*parsed);
}

} // namespace grident
} // namespace qradiolink
} // namespace gr

#endif
