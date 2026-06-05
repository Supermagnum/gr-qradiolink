/*
 * Copyright 2024 QRadioLink Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <gnuradio/qradiolink/grident_preamble_sub.h>

void bind_grident_preamble_sub(py::module& m)
{
    using grident_preamble_sub = gr::qradiolink::grident_preamble_sub;

    py::class_<grident_preamble_sub,
               gr::block,
               gr::basic_block,
               std::shared_ptr<grident_preamble_sub>>(m, "grident_preamble_sub")

        .def(py::init(&grident_preamble_sub::make),
             py::arg("endpoint") = gr::qradiolink::grident::k_preamble_pub_default,
             py::arg("topic_filter") = gr::qradiolink::grident::k_preamble_topic,
             py::arg("bind_socket") = false,
             "SUBscribe to gr-ident preamble ZMQ PUB");
}
