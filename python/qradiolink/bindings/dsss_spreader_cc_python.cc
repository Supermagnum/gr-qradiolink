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

#include <gnuradio/qradiolink/dsss_spreader_cc.h>

void bind_dsss_spreader_cc(py::module& m)
{
    using dsss_spreader_cc = gr::qradiolink::dsss_spreader_cc;

    py::class_<dsss_spreader_cc,
               gr::sync_interpolator,
               gr::block,
               gr::basic_block,
               std::shared_ptr<dsss_spreader_cc>>(m, "dsss_spreader_cc")

        .def(py::init(&dsss_spreader_cc::make),
             py::arg("pn_sequence"),
             py::arg("chips_per_symbol") = 42,
             "Make a DSSS spreader block")

        .def("set_pn_sequence",
             &dsss_spreader_cc::set_pn_sequence,
             py::arg("pn_sequence"),
             "Set PN sequence")

        .def("set_chips_per_symbol",
             &dsss_spreader_cc::set_chips_per_symbol,
             py::arg("chips_per_symbol"),
             "Set chips per symbol");
}

