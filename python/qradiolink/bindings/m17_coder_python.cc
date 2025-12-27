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

#include <gnuradio/qradiolink/m17_coder.h>

void bind_m17_coder(py::module& m)
{
    using m17_coder = gr::qradiolink::m17_coder;

    py::class_<m17_coder,
               gr::block,
               gr::basic_block,
               std::shared_ptr<m17_coder>>(m, "m17_coder")

        .def(py::init(&m17_coder::make),
             py::arg("src_id"),
             py::arg("dst_id"),
             py::arg("mode"),
             py::arg("data"),
             py::arg("encr_type"),
             py::arg("encr_subtype"),
             py::arg("aes_subtype"),
             py::arg("can"),
             py::arg("meta"),
             py::arg("key"),
             py::arg("priv_key"),
             py::arg("debug"),
             py::arg("signed_str"),
             py::arg("seed"),
             py::arg("eot_cnt"),
             "Make an M17 protocol encoder block")

        .def("set_key", &m17_coder::set_key, py::arg("meta"), "Set AES key")
        .def("set_priv_key", &m17_coder::set_priv_key, py::arg("meta"), "Set private key for ECDSA")
        .def("set_seed", &m17_coder::set_seed, py::arg("seed"), "Set scrambler seed")
        .def("set_eot_cnt", &m17_coder::set_eot_cnt, py::arg("arg"), "Set end-of-transmission count")
        .def("set_meta", &m17_coder::set_meta, py::arg("meta"), "Set metadata")
        .def("set_src_id", &m17_coder::set_src_id, py::arg("src_id"), "Set source callsign")
        .def("set_dst_id", &m17_coder::set_dst_id, py::arg("dst_id"), "Set destination callsign")
        .def("set_debug", &m17_coder::set_debug, py::arg("debug"), "Set debug mode")
        .def("set_signed", &m17_coder::set_signed, py::arg("signed_str"), "Set signed stream flag")
        .def("set_type", &m17_coder::set_type, py::arg("mode"), py::arg("data"), py::arg("encr_type"), py::arg("encr_subtype"), py::arg("can"), "Set transmission type")
        .def("set_mode", &m17_coder::set_mode, py::arg("mode"), "Set mode")
        .def("set_data", &m17_coder::set_data, py::arg("data"), "Set data type")
        .def("set_encr_type", &m17_coder::set_encr_type, py::arg("encr_type"), "Set encryption type")
        .def("set_encr_subtype", &m17_coder::set_encr_subtype, py::arg("encr_subtype"), "Set encryption subtype")
        .def("set_aes_subtype", &m17_coder::set_aes_subtype, py::arg("aes_subtype"), py::arg("encr_type"), "Set AES subtype")
        .def("set_can", &m17_coder::set_can, py::arg("can"), "Set CAN");

    // Export the encr_t enum
    py::enum_<m17_coder::encr_t>(m, "m17_encr_t")
        .value("ENCR_NONE", m17_coder::ENCR_NONE)
        .value("ENCR_SCRAM", m17_coder::ENCR_SCRAM)
        .value("ENCR_AES", m17_coder::ENCR_AES)
        .value("ENCR_RES", m17_coder::ENCR_RES);
}

