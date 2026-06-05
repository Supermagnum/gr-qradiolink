/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <gnuradio/qradiolink/grident_mode_control.h>

void bind_grident_mode_control(py::module& m)
{
    using grident_mode_control = gr::qradiolink::grident_mode_control;

    py::class_<grident_mode_control,
               gr::block,
               gr::basic_block,
               std::shared_ptr<grident_mode_control>>(m, "grident_mode_control")

        .def(py::init(&grident_mode_control::make), "Make gr-ident mode control block")

        .def("mode_id", &grident_mode_control::mode_id)
        .def("digital", &grident_mode_control::digital)
        .def("encrypted", &grident_mode_control::encrypted)
        .def("metadata_present", &grident_mode_control::metadata_present)
        .def("route_valid", &grident_mode_control::route_valid)
        .def("demod_block", &grident_mode_control::demod_block)
        .def("mod_block", &grident_mode_control::mod_block);
}
