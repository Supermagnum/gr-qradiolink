/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>

namespace py = pybind11;

#include <gnuradio/qradiolink/demod_8fsk.h>

void bind_demod_8fsk(py::module& m)
{
    using demod_8fsk = gr::qradiolink::demod_8fsk;

    py::class_<demod_8fsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_8fsk>>(m, "demod_8fsk")

        .def(py::init(&demod_8fsk::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             py::arg("fm") = true,
             "Make an 8FSK demodulator block");
}

