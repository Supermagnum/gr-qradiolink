// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>
#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/qradiolink/GrIdentModeControl.hpp>
#include <gnuradio-4.0/qradiolink/ModDemodSwitch.hpp>
#include <gnuradio-4.0/qradiolink/detail/GrIdentModeMap.hpp>
#include <gnuradio-4.0/qradiolink/detail/GrIdentZmqProtocol.hpp>

#include <complex>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

const boost::ut::suite qr_grident_zmq = [] {
    using namespace boost::ut;
    using gnuradio4::qradiolink::GrIdentModeControl;
    using gnuradio4::qradiolink::ModDemodSwitchRx;
    using gnuradio4::qradiolink::detail::QrModemRoute;

    "parse gr-ident preamble JSON (SDR-repeater section 6.1)"_test = [] {
        constexpr std::string_view sample = R"({"mode_id":20,"digital":false,"encrypted":false,"metadata_present":false})";
        const auto parsed                 = gnuradio4::qradiolink::detail::parsePreambleResultJson(sample);
        expect(parsed.has_value());
        expect(parsed->mode_id == 20U);
        expect(parsed->digital == false);
        expect(parsed->encrypted == false);
        expect(parsed->metadata_present == false);
    };

    "mode_id 104 routes to YSF demod family"_test = [] {
        const auto sel = gnuradio4::qradiolink::detail::selectionFromPreambleJson(
            R"({"mode_id":104,"digital":true,"encrypted":false,"metadata_present":false})");
        expect(sel.has_value());
        expect(sel->demod_route == QrModemRoute::DemodYsf);
        expect(sel->mod_route == QrModemRoute::ModMmdvmMulti);
    };

    "GrIdentModeControl applies ZMQ JSON and selects DemodNbfm for mode 20"_test = [] {
        auto seq = std::make_shared<gr::Sequence>();
        GrIdentModeControl ctrl{};
        ctrl.init(seq);

        ctrl.applyPreambleZmqFrames("grident",
            R"({"mode_id":20,"digital":false,"encrypted":false,"metadata_present":false})");

        expect(ctrl.mode_id.value == 20U);
        expect(ctrl.digital.value == false);
        expect(ctrl.route_valid.value == true);
        expect(ctrl.active_demod_route.value == static_cast<std::uint8_t>(QrModemRoute::DemodNbfm));
    };

    "ModDemodSwitchRx follows active_demod_route from gr-ident control"_test = [] {
        auto seq = std::make_shared<gr::Sequence>();
        ModDemodSwitchRx sw{};
        sw.init(seq);
        sw.active_demod_route.value = static_cast<std::uint8_t>(QrModemRoute::DemodNbfm);
        sw.sample_rate_hz.value     = 48000.F;

        std::vector<std::complex<float>> iq(64UZ, {0.9F, 0.1F});
        std::vector<float>               audio(iq.size(), 0.F);
        std::span<const std::complex<float>> in(iq.data(), iq.size());
        std::span<float>                   out(audio.data(), audio.size());
        expect(sw.processBulk(in, out) == gr::work::Status::OK);
        expect(audio[0UZ] != 0.F);
    };
};

} // namespace

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
