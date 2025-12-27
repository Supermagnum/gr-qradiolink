/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/qradiolink/demod_soqpsk.h>

void bind_demod_soqpsk(py::module& m)
{
    using demod_soqpsk = gr::qradiolink::demod_soqpsk;

    py::class_<demod_soqpsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_soqpsk>>(m, "demod_soqpsk")

        .def(py::init(&demod_soqpsk::make),
             py::arg("mode") = 1,
             py::arg("sps") = 10,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 0,
             py::arg("filter_width") = 10000,
             "Make a SOQPSK demodulator block");
}

