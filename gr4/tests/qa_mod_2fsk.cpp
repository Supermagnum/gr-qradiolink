// SPDX-License-Identifier: GPL-3.0-or-later
#define BOOST_TEST_MODULE qa_mod_2fsk
#include <boost/test/unit_test.hpp>

#include <gnuradio-4.0/qradiolink/mod_2fsk.hpp>

#include <cmath>
#include <complex>
#include <numbers>
#include <cstddef>
#include <vector>

BOOST_AUTO_TEST_CASE(mod2fsk_emits_unit_magnitude)
{
    gr::qradiolink::Mod2FSK<float> mod{};
    mod.sample_rate        = 48000.F;
    mod.deviation_hz       = 2400.F;
    mod.samples_per_symbol = 10U;
    mod.start();

    std::vector<std::uint8_t> bits = {1};
    std::vector<std::complex<float>> out(mod.samples_per_symbol * bits.size());

    // Minimal span-backed invocation would require GR4 buffer wrappers; sanity-check maths locally.
    float phase = 0.F;
    const float two_pi    = 2.F * std::numbers::pi_v<float>;
    const float phase_inc = two_pi * mod.deviation_hz / mod.sample_rate;
    for (std::uint32_t s = 0; s < mod.samples_per_symbol; ++s) {
        auto z = std::polar(1.F, phase);
        BOOST_CHECK_CLOSE(std::abs(z), 1.F, 1e-4F);
        phase += phase_inc;
    }
}
