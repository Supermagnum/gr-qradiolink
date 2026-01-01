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

#include <gnuradio/qradiolink/gdss_spreader_cc.h>

void bind_gdss_spreader_cc(py::module& m)
{
    using gdss_spreader_cc = gr::qradiolink::gdss_spreader_cc;

    py::class_<gdss_spreader_cc,
               gr::sync_interpolator,
               gr::block,
               gr::basic_block,
               std::shared_ptr<gdss_spreader_cc>>(m, "gdss_spreader_cc")

        .def(py::init(&gdss_spreader_cc::make),
             py::arg("sequence_length"),
             py::arg("chips_per_symbol") = 42,
             py::arg("variance") = 1.0f,
             py::arg("seed") = 0,
             "Make a GDSS spreader block")

        .def("set_spreading_sequence",
             &gdss_spreader_cc::set_spreading_sequence,
             py::arg("sequence"),
             "Set spreading sequence")

        .def("set_chips_per_symbol",
             &gdss_spreader_cc::set_chips_per_symbol,
             py::arg("chips_per_symbol"),
             "Set chips per symbol")

        .def("regenerate_sequence",
             &gdss_spreader_cc::regenerate_sequence,
             py::arg("variance"),
             py::arg("seed") = 0,
             "Regenerate spreading sequence");
}

