# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Install __init__.py and helper modules
########################################################################
GR_PYTHON_INSTALL(
    FILES __init__.py ldpc_code_helper.py
    DESTINATION ${GR_PYTHON_DIR}/gnuradio/qradiolink
)

########################################################################
# Install bindings
########################################################################
add_subdirectory(bindings)

