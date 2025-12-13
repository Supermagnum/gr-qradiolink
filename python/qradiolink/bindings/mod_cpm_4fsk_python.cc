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

#include <gnuradio/qradiolink/mod_cpm_4fsk.h>

void bind_mod_cpm_4fsk(py::module& m)
{
    using mod_cpm_4fsk = gr::qradiolink::mod_cpm_4fsk;

    py::class_<mod_cpm_4fsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_cpm_4fsk>>(m, "mod_cpm_4fsk")

        .def(py::init(&mod_cpm_4fsk::make),
             py::arg("sps") = 2,
             py::arg("samp_rate") = 96000,
             py::arg("carrier_freq") = 0,
             py::arg("filter_width") = 5000,
             py::arg("h") = 0.5f,
             py::arg("L") = 4,
             py::arg("beta") = 0.3f,
             "Make a 4FSK CPM modulator block")

        .def("set_bb_gain",
             &mod_cpm_4fsk::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

