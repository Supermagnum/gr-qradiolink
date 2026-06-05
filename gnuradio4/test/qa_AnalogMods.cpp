// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>
#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/qradiolink/DemodAm.hpp>
#include <gnuradio-4.0/qradiolink/ModAm.hpp>

#include <cmath>
#include <cstddef>
#include <numbers>
#include <span>
#include <vector>

namespace {

const boost::ut::suite qr_analog_am = [] {
    using namespace boost::ut;
    using gnuradio4::qradiolink::DemodAm;
    using gnuradio4::qradiolink::ModAm;

    "AM mod/demod mean tracks slow positive envelope (graph-free qa)"_test = [] {
        auto seq = std::make_shared<gr::Sequence>();
        ModAm mod{};
        DemodAm dem{};
        mod.init(seq);
        dem.init(seq);

        constexpr float sr        = 48000.F;
        constexpr float fc        = 5000.F;
        constexpr float audio_freq = 200.F;
        mod.sample_rate_hz         = sr;
        dem.sample_rate_hz         = sr;
        mod.carrier_hz             = fc;
        dem.carrier_hz             = fc;
        constexpr float dc         = 0.62F;

        constexpr std::size_t n           = 800U;
        std::vector<float>      audio_ll(n);
        for (std::size_t ix = 0; ix < n; ++ix) {
            const float t =
                dc + 0.12F * std::sin(std::numbers::pi_v<float> * 2.F * audio_freq * static_cast<float>(ix) / sr);
            audio_ll[ix] = t;
        }

        std::vector<std::complex<float>> iq(n);
        std::vector<float>               dem_aud(n);

        std::span<const float>         in(audio_ll.data(), audio_ll.size());
        std::span<std::complex<float>> rf_out(iq.data(), iq.size());
        std::span<const std::complex<float>> rf_in(iq.data(), iq.size());
        std::span<float>                     aud_wr(dem_aud.data(), dem_aud.size());

        expect(mod.processBulk(in, rf_out) == gr::work::Status::OK);
        expect(dem.processBulk(rf_in, aud_wr) == gr::work::Status::OK);

        float accum = 0.F;
        for (std::size_t j = 100; j + 100 < n; ++j) {
            accum += dem_aud[j];
        }
        const float mean_aud = accum / static_cast<float>(n - 200U);

        expect(std::abs(mean_aud - dc) < 0.18F)
            << "envelope-demod heuristic should loosely track bias for qa inputs";
    };
};

} // namespace

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
