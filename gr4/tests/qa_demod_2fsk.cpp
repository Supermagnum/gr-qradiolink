// SPDX-License-Identifier: GPL-3.0-or-later
#define BOOST_TEST_MODULE qa_demod_2fsk
#include <boost/test/unit_test.hpp>

#include <gnuradio-4.0/qradiolink/demod_2fsk.hpp>

BOOST_AUTO_TEST_CASE(demod2fsk_default_construct)
{
    gr::qradiolink::Demod2FSK<float> demod{};
    demod.sample_rate        = 48000.F;
    demod.deviation_hz       = 2400.F;
    demod.samples_per_symbol = 10U;
    demod.start();
    (void)demod;
}
