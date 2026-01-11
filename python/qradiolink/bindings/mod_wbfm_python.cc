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

#include <gnuradio/qradiolink/mod_wbfm.h>

void bind_mod_wbfm(py::module& m)
{
    using mod_wbfm = gr::qradiolink::mod_wbfm;

    py::class_<mod_wbfm,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_wbfm>>(m, "mod_wbfm")

        .def(py::init(&mod_wbfm::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make a WBFM modulator block")

        .def("set_filter_width",
             &mod_wbfm::set_filter_width,
             py::arg("filter_width"),
             "Set filter width")

        .def("set_bb_gain",
             &mod_wbfm::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

