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

#include <gnuradio/qradiolink/gdss_despreader_cc.h>

void bind_gdss_despreader_cc(py::module& m)
{
    using gdss_despreader_cc = gr::qradiolink::gdss_despreader_cc;

    py::class_<gdss_despreader_cc,
               gr::block,
               gr::basic_block,
               std::shared_ptr<gdss_despreader_cc>>(m, "gdss_despreader_cc")

        .def(py::init(&gdss_despreader_cc::make),
             py::arg("spreading_sequence"),
             py::arg("chips_per_symbol") = 42,
             py::arg("correlation_threshold") = 0.7f,
             py::arg("timing_error_tolerance") = 2,
             "Make a GDSS despreader block")

        .def("set_spreading_sequence",
             &gdss_despreader_cc::set_spreading_sequence,
             py::arg("spreading_sequence"),
             "Set spreading sequence")

        .def("set_chips_per_symbol",
             &gdss_despreader_cc::set_chips_per_symbol,
             py::arg("chips_per_symbol"),
             "Set chips per symbol")

        .def("get_sync_state",
             &gdss_despreader_cc::get_sync_state,
             "Get current sync state")

        .def("is_locked",
             &gdss_despreader_cc::is_locked,
             "Get lock status")

        .def("get_snr_estimate",
             &gdss_despreader_cc::get_snr_estimate,
             "Get SNR estimate in dB")

        .def("get_last_soft_metric",
             &gdss_despreader_cc::get_last_soft_metric,
             "Get last soft-decision metric (normalized correlation magnitude)")

        .def("get_frequency_error",
             &gdss_despreader_cc::get_frequency_error,
             "Get estimated frequency error in rad/symbol for AFC");

    // Bind sync_state enum
    py::enum_<gdss_despreader_cc::sync_state>(m, "gdss_sync_state")
        .value("STATE_ACQUISITION", gdss_despreader_cc::STATE_ACQUISITION)
        .value("STATE_TRACKING", gdss_despreader_cc::STATE_TRACKING)
        .value("STATE_LOCKED", gdss_despreader_cc::STATE_LOCKED);
}

