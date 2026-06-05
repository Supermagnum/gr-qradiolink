// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef GNURADIO4_QRAD_DETAIL_GRIDENTMODEMAP_HPP
#define GNURADIO4_QRAD_DETAIL_GRIDENTMODEMAP_HPP

#include <gnuradio-4.0/qradiolink/detail/GrIdentZmqProtocol.hpp>

#include <cstdint>
#include <optional>
#include <string_view>

namespace gnuradio4::qradiolink::detail {

// Selects which gr-qradiolink GR4 demod/mod block family to activate for a gr-ident mode_id.
enum class QrModemRoute : std::uint8_t {
    Unknown = 0,
    DemodAm,
    DemodSsbUsb,
    DemodSsbLsb,
    DemodNbfm,
    DemodWbfm,
    Demod2Fsk,
    Demod4Fsk,
    Demod8Fsk,
    DemodBpsk,
    DemodQpsk,
    DemodGmsk,
    DemodDmr,
    DemodDpmr,
    DemodNxdn,
    DemodM17,
    DemodFreeDv,
    DemodMmdvmMulti,
    DemodDstar,
    DemodYsf,
    DemodP25,
    ModAm,
    ModSsb,
    ModNbfm,
    ModWbfm,
    Mod2Fsk,
    Mod4Fsk,
    Mod8Fsk,
    ModBpsk,
    ModQpsk,
    ModGmsk,
    ModDmr,
    ModDpmr,
    ModNxdn,
    ModM17,
    ModFreeDv,
    ModMmdvmMulti,
};

[[nodiscard]] constexpr bool qrRouteIsDemod(QrModemRoute r) noexcept
{
    return r >= QrModemRoute::DemodAm && r <= QrModemRoute::DemodP25;
}

[[nodiscard]] constexpr bool qrRouteIsMod(QrModemRoute r) noexcept
{
    return r >= QrModemRoute::ModAm && r <= QrModemRoute::ModMmdvmMulti;
}

[[nodiscard]] inline std::string_view qrRouteName(QrModemRoute r) noexcept
{
    switch (r) {
    case QrModemRoute::DemodAm:
        return "DemodAm";
    case QrModemRoute::DemodSsbUsb:
        return "DemodSsbUsb";
    case QrModemRoute::DemodSsbLsb:
        return "DemodSsbLsb";
    case QrModemRoute::DemodNbfm:
        return "DemodNbfm";
    case QrModemRoute::DemodWbfm:
        return "DemodWbfm";
    case QrModemRoute::Demod2Fsk:
        return "Demod2Fsk";
    case QrModemRoute::Demod4Fsk:
        return "Demod4Fsk";
    case QrModemRoute::Demod8Fsk:
        return "Demod8Fsk";
    case QrModemRoute::DemodBpsk:
        return "DemodBpsk";
    case QrModemRoute::DemodQpsk:
        return "DemodQpsk";
    case QrModemRoute::DemodGmsk:
        return "DemodGmsk";
    case QrModemRoute::DemodDmr:
        return "DemodDmr";
    case QrModemRoute::DemodDpmr:
        return "DemodDpmr";
    case QrModemRoute::DemodNxdn:
        return "DemodNxdn";
    case QrModemRoute::DemodM17:
        return "DemodM17";
    case QrModemRoute::DemodFreeDv:
        return "DemodFreeDv";
    case QrModemRoute::DemodMmdvmMulti:
        return "DemodMmdvmMulti";
    case QrModemRoute::DemodDstar:
        return "DstarDecoder";
    case QrModemRoute::DemodYsf:
        return "YsfDecoder";
    case QrModemRoute::DemodP25:
        return "P25Decoder";
    case QrModemRoute::ModAm:
        return "ModAm";
    case QrModemRoute::ModSsb:
        return "ModSsb";
    case QrModemRoute::ModNbfm:
        return "ModNbfm";
    case QrModemRoute::ModWbfm:
        return "ModWbfm";
    case QrModemRoute::Mod2Fsk:
        return "Mod2Fsk";
    case QrModemRoute::Mod4Fsk:
        return "Mod4Fsk";
    case QrModemRoute::Mod8Fsk:
        return "Mod8Fsk";
    case QrModemRoute::ModBpsk:
        return "ModBpsk";
    case QrModemRoute::ModQpsk:
        return "ModQpsk";
    case QrModemRoute::ModGmsk:
        return "ModGmsk";
    case QrModemRoute::ModDmr:
        return "ModDmr";
    case QrModemRoute::ModDpmr:
        return "ModDpmr";
    case QrModemRoute::ModNxdn:
        return "ModNxdn";
    case QrModemRoute::ModM17:
        return "ModM17";
    case QrModemRoute::ModFreeDv:
        return "ModFreeDv";
    case QrModemRoute::ModMmdvmMulti:
        return "ModMmdvmMulti2";
    default:
        return "Unknown";
    }
}

[[nodiscard]] inline QrModemRoute qrDemodRouteForModeId(std::uint16_t mode_id) noexcept
{
    switch (mode_id) {
    case 1:
    case 2:
    case 6:
    case 62:
        return QrModemRoute::DemodAm;
    case 3:
    case 63:
        return QrModemRoute::DemodSsbUsb;
    case 4:
        return QrModemRoute::DemodSsbUsb;
    case 5:
        return QrModemRoute::DemodSsbLsb;
    case 10:
    case 11:
        return QrModemRoute::DemodWbfm;
    case 12:
    case 13:
    case 33:
    case 34:
    case 43:
    case 44:
        return QrModemRoute::DemodWbfm;
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
        return QrModemRoute::DemodNbfm;
    case 60:
    case 61:
        return QrModemRoute::DemodAm;
    case 100:
    case 101:
    case 102:
    case 106:
    case 109:
        return QrModemRoute::DemodDmr;
    case 103:
    case 115:
        return QrModemRoute::DemodDstar;
    case 104:
    case 105:
    case 114:
        return QrModemRoute::DemodYsf;
    case 107:
        return QrModemRoute::DemodNxdn;
    case 108:
        return QrModemRoute::DemodDpmr;
    case 120:
    case 121:
        return QrModemRoute::DemodM17;
    case 122:
    case 123:
    case 124:
        return QrModemRoute::DemodFreeDv;
    case 150:
    case 151:
        return QrModemRoute::Demod2Fsk;
    case 155:
        return QrModemRoute::Demod2Fsk;
    case 158:
        return QrModemRoute::DemodBpsk;
    case 159:
        return QrModemRoute::Demod2Fsk;
    default:
        if (mode_id >= 300U && mode_id <= 498U) {
            return QrModemRoute::DemodQpsk;
        }
        return QrModemRoute::Unknown;
    }
}

[[nodiscard]] inline QrModemRoute qrModRouteForModeId(std::uint16_t mode_id) noexcept
{
    const QrModemRoute dem = qrDemodRouteForModeId(mode_id);
    switch (dem) {
    case QrModemRoute::DemodAm:
        return QrModemRoute::ModAm;
    case QrModemRoute::DemodSsbUsb:
    case QrModemRoute::DemodSsbLsb:
        return QrModemRoute::ModSsb;
    case QrModemRoute::DemodNbfm:
        return QrModemRoute::ModNbfm;
    case QrModemRoute::DemodWbfm:
        return QrModemRoute::ModWbfm;
    case QrModemRoute::Demod2Fsk:
        return QrModemRoute::Mod2Fsk;
    case QrModemRoute::Demod4Fsk:
        return QrModemRoute::Mod4Fsk;
    case QrModemRoute::Demod8Fsk:
        return QrModemRoute::Mod8Fsk;
    case QrModemRoute::DemodBpsk:
        return QrModemRoute::ModBpsk;
    case QrModemRoute::DemodQpsk:
        return QrModemRoute::ModQpsk;
    case QrModemRoute::DemodGmsk:
        return QrModemRoute::ModGmsk;
    case QrModemRoute::DemodDmr:
        return QrModemRoute::ModDmr;
    case QrModemRoute::DemodDpmr:
        return QrModemRoute::ModDpmr;
    case QrModemRoute::DemodNxdn:
        return QrModemRoute::ModNxdn;
    case QrModemRoute::DemodM17:
        return QrModemRoute::ModM17;
    case QrModemRoute::DemodFreeDv:
        return QrModemRoute::ModFreeDv;
    case QrModemRoute::DemodMmdvmMulti:
    case QrModemRoute::DemodYsf:
    case QrModemRoute::DemodP25:
        return QrModemRoute::ModMmdvmMulti;
    case QrModemRoute::DemodDstar:
        return QrModemRoute::ModDmr;
    default:
        return QrModemRoute::Unknown;
    }
}

struct GrIdentModeSelection {
    GrIdentPreambleResult preamble{};
    QrModemRoute          demod_route{QrModemRoute::Unknown};
    QrModemRoute          mod_route{QrModemRoute::Unknown};
    bool                  route_valid{false};
};

[[nodiscard]] inline GrIdentModeSelection selectionFromPreamble(const GrIdentPreambleResult& p) noexcept
{
    GrIdentModeSelection sel{};
    sel.preamble     = p;
    sel.demod_route  = qrDemodRouteForModeId(p.mode_id);
    sel.mod_route    = qrModRouteForModeId(p.mode_id);
    sel.route_valid  = sel.demod_route != QrModemRoute::Unknown;
    return sel;
}

[[nodiscard]] inline std::optional<GrIdentModeSelection> selectionFromPreambleJson(std::string_view json)
{
    const auto parsed = parsePreambleResultJson(json);
    if (!parsed) {
        return std::nullopt;
    }
    return selectionFromPreamble(*parsed);
}

} // namespace gnuradio4::qradiolink::detail

#endif
