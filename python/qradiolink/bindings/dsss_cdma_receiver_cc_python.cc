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

#include <gnuradio/qradiolink/dsss_cdma_receiver_cc.h>

void bind_dsss_cdma_receiver_cc(py::module& m)
{
    using dsss_cdma_receiver_cc = gr::qradiolink::dsss_cdma_receiver_cc;

    py::class_<dsss_cdma_receiver_cc,
               gr::block,
               gr::basic_block,
               std::shared_ptr<dsss_cdma_receiver_cc>>(m, "dsss_cdma_receiver_cc")

        .def(py::init(&dsss_cdma_receiver_cc::make),
             py::arg("spreading_code"),
             py::arg("spreading_factor"),
             py::arg("correlation_threshold") = 0.7f,
             py::arg("timing_error_tolerance") = 2,
             "Make a DSSS-CDMA receiver block")

        .def("set_spreading_code",
             &dsss_cdma_receiver_cc::set_spreading_code,
             py::arg("spreading_code"),
             "Set spreading code for the desired user")

        .def("set_spreading_factor",
             &dsss_cdma_receiver_cc::set_spreading_factor,
             py::arg("spreading_factor"),
             "Set spreading factor")

        .def("get_sync_state",
             &dsss_cdma_receiver_cc::get_sync_state,
             "Get current sync state")

        .def("is_locked",
             &dsss_cdma_receiver_cc::is_locked,
             "Get lock status")

        .def("get_snr_estimate",
             &dsss_cdma_receiver_cc::get_snr_estimate,
             "Get SNR estimate in dB")

        .def("get_interference_estimate",
             &dsss_cdma_receiver_cc::get_interference_estimate,
             "Get multi-user interference estimate in dB");

    // Bind sync_state enum
    py::enum_<dsss_cdma_receiver_cc::sync_state>(m, "cdma_sync_state")
        .value("STATE_ACQUISITION", dsss_cdma_receiver_cc::STATE_ACQUISITION)
        .value("STATE_TRACKING", dsss_cdma_receiver_cc::STATE_TRACKING)
        .value("STATE_LOCKED", dsss_cdma_receiver_cc::STATE_LOCKED);
}

