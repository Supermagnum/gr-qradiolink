/*
 * Copyright 2025 QRadioLink Contributors
 * Adapted from gr-m17
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/qradiolink/m17_decoder.h>

void bind_m17_decoder(py::module& m)
{
    using m17_decoder = gr::qradiolink::m17_decoder;

    py::class_<m17_decoder,
               gr::block,
               gr::basic_block,
               std::shared_ptr<m17_decoder>>(m, "m17_decoder")

        .def(py::init(&m17_decoder::make),
             py::arg("debug_data"),
             py::arg("debug_ctrl"),
             py::arg("sw_threshold"),
             py::arg("vt_threshold"),
             py::arg("callsign"),
             py::arg("signed_str"),
             py::arg("encr_type"),
             py::arg("key"),
             py::arg("seed"),
             "Make an M17 protocol decoder block")

        .def("set_debug_data", &m17_decoder::set_debug_data, py::arg("debug"), "Set data debug mode")
        .def("set_debug_ctrl", &m17_decoder::set_debug_ctrl, py::arg("debug"), "Set control debug mode")
        .def("set_callsign", &m17_decoder::set_callsign, py::arg("callsign"), "Set callsign display")
        .def("set_sw_threshold", &m17_decoder::set_sw_threshold, py::arg("sw_threshold"), "Set syncword threshold")
        .def("set_vt_threshold", &m17_decoder::set_vt_threshold, py::arg("vt_threshold"), "Set Viterbi threshold")
        .def("set_signed", &m17_decoder::set_signed, py::arg("signed_str"), "Set signed stream flag")
        .def("set_key", &m17_decoder::set_key, py::arg("key"), "Set AES key")
        .def("set_seed", &m17_decoder::set_seed, py::arg("seed"), "Set scrambler seed")
        .def("parse_raw_key_string", &m17_decoder::parse_raw_key_string, py::arg("dest"), py::arg("inp"), "Parse raw key string")
        .def("scrambler_sequence_generator", &m17_decoder::scrambler_sequence_generator, "Generate scrambler sequence")
        .def("scrambler_seed_calculation", &m17_decoder::scrambler_seed_calculation, py::arg("subtype"), py::arg("key"), py::arg("fn"), "Calculate scrambler seed");

    // Export the encr_t enum
    py::enum_<m17_decoder::encr_t>(m, "m17_decoder_encr_t")
        .value("ENCR_NONE", m17_decoder::ENCR_NONE)
        .value("ENCR_SCRAM", m17_decoder::ENCR_SCRAM)
        .value("ENCR_AES", m17_decoder::ENCR_AES)
        .value("ENCR_RES", m17_decoder::ENCR_RES);
}

