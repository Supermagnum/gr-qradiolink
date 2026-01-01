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

#include <gnuradio/qradiolink/mod_8fsk.h>

void bind_mod_8fsk(py::module& m)
{
    using mod_8fsk = gr::qradiolink::mod_8fsk;

    py::class_<mod_8fsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_8fsk>>(m, "mod_8fsk")

        .def(py::init(&mod_8fsk::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             py::arg("fm") = true,
             "Make an 8FSK modulator block")

        .def("set_bb_gain",
             &mod_8fsk::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

