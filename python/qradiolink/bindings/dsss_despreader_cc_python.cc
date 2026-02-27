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

#include <gnuradio/qradiolink/dsss_despreader_cc.h>

void bind_dsss_despreader_cc(py::module& m)
{
    using dsss_despreader_cc = gr::qradiolink::dsss_despreader_cc;

    py::enum_<dsss_despreader_cc::sync_state>(m, "dsss_sync_state")
        .value("STATE_ACQUISITION", dsss_despreader_cc::STATE_ACQUISITION)
        .value("STATE_TRACKING", dsss_despreader_cc::STATE_TRACKING)
        .value("STATE_LOCKED", dsss_despreader_cc::STATE_LOCKED);

    py::class_<dsss_despreader_cc,
               gr::block,
               gr::basic_block,
               std::shared_ptr<dsss_despreader_cc>>(m, "dsss_despreader_cc")

        .def(py::init(&dsss_despreader_cc::make),
             py::arg("pn_sequence"),
             py::arg("chips_per_symbol") = 42,
             py::arg("correlation_threshold") = 0.7f,
             py::arg("timing_error_tolerance") = 2,
             "Make a DSSS despreader block")

        .def("set_pn_sequence",
             &dsss_despreader_cc::set_pn_sequence,
             py::arg("pn_sequence"),
             "Set PN sequence")

        .def("set_chips_per_symbol",
             &dsss_despreader_cc::set_chips_per_symbol,
             py::arg("chips_per_symbol"),
             "Set chips per symbol")

        .def("get_sync_state",
             &dsss_despreader_cc::get_sync_state,
             "Get current synchronization state")

        .def("is_locked",
             &dsss_despreader_cc::is_locked,
             "Get lock status")

        .def("get_snr_estimate",
             &dsss_despreader_cc::get_snr_estimate,
             "Get SNR estimate in dB")

        .def("get_last_soft_metric",
             &dsss_despreader_cc::get_last_soft_metric,
             "Get last soft-decision metric (normalized correlation magnitude)")

        .def("get_frequency_error",
             &dsss_despreader_cc::get_frequency_error,
             "Get estimated frequency error in rad/symbol for AFC");
}

