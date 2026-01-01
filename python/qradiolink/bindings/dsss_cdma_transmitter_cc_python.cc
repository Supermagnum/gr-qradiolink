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

#include <gnuradio/qradiolink/dsss_cdma_transmitter_cc.h>

void bind_dsss_cdma_transmitter_cc(py::module& m)
{
    using dsss_cdma_transmitter_cc = gr::qradiolink::dsss_cdma_transmitter_cc;

    py::class_<dsss_cdma_transmitter_cc,
               gr::block,
               gr::basic_block,
               std::shared_ptr<dsss_cdma_transmitter_cc>>(m, "dsss_cdma_transmitter_cc")

        .def(py::init(&dsss_cdma_transmitter_cc::make),
             py::arg("spreading_codes"),
             py::arg("spreading_factor"),
             py::arg("num_users"),
             py::arg("normalize_power") = true,
             "Make a DSSS-CDMA transmitter block")

        .def("set_spreading_codes",
             &dsss_cdma_transmitter_cc::set_spreading_codes,
             py::arg("spreading_codes"),
             "Set spreading codes for all users")

        .def("set_spreading_factor",
             &dsss_cdma_transmitter_cc::set_spreading_factor,
             py::arg("spreading_factor"),
             "Set spreading factor")

        .def("set_num_users",
             &dsss_cdma_transmitter_cc::set_num_users,
             py::arg("num_users"),
             "Set number of users")

        .def("set_normalize_power",
             &dsss_cdma_transmitter_cc::set_normalize_power,
             py::arg("normalize_power"),
             "Set power normalization");
}

