// SPDX-License-Identifier: GPL-3.0-or-later
#include <boost/ut.hpp>
#include <gnuradio-4.0/Sequence.hpp>
#include <gnuradio-4.0/qradiolink/Demod2Fsk.hpp>
#include <gnuradio-4.0/qradiolink/Mod2Fsk.hpp>

#include <cmath>
#include <cstdint>
#include <span>
#include <vector>

namespace {

const boost::ut::suite qr_digital_mods = [] {
    using namespace boost::ut;
    using gnuradio4::qradiolink::Demod2Fsk;
    using gnuradio4::qradiolink::Mod2Fsk;

    "2FSK smoke (standalone mod+demod, no scheduler graph)"_test = [] {
        auto      seq = std::make_shared<gr::Sequence>();
        Mod2Fsk   mod{};
        Demod2Fsk dem{};
        mod.init(seq);
        dem.init(seq);

        constexpr gr::Size_t sps_u       = gr::Size_t{128U};
        mod.samples_per_symbol            = sps_u;
        dem.samples_per_symbol            = sps_u;
        constexpr float freq_sep           = 3200.F;
        mod.frequency_separation_hz        = freq_sep;
        dem.frequency_separation_hz        = freq_sep;
        mod.carrier_offset_hz              = 0.F;
        mod.sample_rate_hz                 = 192000.F;

        const std::vector<std::uint8_t> bits{0U, 1U, 1U, 0U, 1U, 0U, 0U, 1U};
        const std::size_t                need_wave = bits.size() * static_cast<std::size_t>(sps_u);
        std::vector<std::complex<float>> wave(need_wave);
        std::span<const std::uint8_t>    in_bs(bits.data(), bits.size());
        std::span<std::complex<float>>   out_wave(wave.data(), wave.size());

        expect(mod.processBulk(in_bs, out_wave) == gr::work::Status::OK);

        std::vector<std::uint8_t> recovered(bits.size(), 255U);
        std::span<const std::complex<float>> in_wave(wave.data(), wave.size());
        std::span<std::uint8_t>              out_bs(recovered.data(), recovered.size());
        expect(dem.processBulk(in_wave, out_bs) == gr::work::Status::OK);

        unsigned matches = 0U;
        for (std::size_t i = 0; i < bits.size(); ++i) {
            if (recovered[i] == bits[i]) {
                ++matches;
            }
        }
        expect(matches >= 6U)
            << "simplified qa 2FSK chain ought to recover much of this short vector without a timed receiver";
    };
};

} // namespace

int main() { return boost::ut::cfg<boost::ut::override>.run(); }
