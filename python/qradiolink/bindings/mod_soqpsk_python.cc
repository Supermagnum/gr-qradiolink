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

#include <gnuradio/qradiolink/mod_soqpsk.h>

void bind_mod_soqpsk(py::module& m)
{
    using mod_soqpsk = gr::qradiolink::mod_soqpsk;

    py::class_<mod_soqpsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_soqpsk>>(m, "mod_soqpsk")

        .def(py::init(&mod_soqpsk::make),
             py::arg("mode") = 1,
             py::arg("sps") = 10,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 0,
             py::arg("filter_width") = 10000,
             "Make a SOQPSK modulator block")

        .def("set_bb_gain",
             &mod_soqpsk::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

