/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <gnuradio/qradiolink/interleaver_bb.h>

void bind_interleaver_bb(py::module& m)
{
    using interleaver_bb = gr::qradiolink::interleaver_bb;

    py::class_<interleaver_bb,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<interleaver_bb>>(m, "interleaver_bb")

        .def(py::init(&interleaver_bb::make),
             py::arg("n_rows"),
             py::arg("n_cols"),
             py::arg("interleave") = true,
             "Make a block interleaver for HF burst error handling");
}
