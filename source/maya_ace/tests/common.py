# SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# constants used in unit tests
# NOTE: if this variable is not set or empty, we'll start a local mock grpc server for tests.
import os

# remove Qt warnings with vanila mayapy
try:
    from PySide2 import QtCore, QtWidgets

    QtCore.QCoreApplication.setAttribute(QtCore.Qt.AA_ShareOpenGLContexts)
except ImportError:
    pass

PLUGIN_NAME = "maya_ace"
A2F_PLAYER_NODE_NAME = "A2FAnimationPlayer"
ACE_PLAYER_NODE_NAME = "AceAnimationPlayer"
ENV_API_KEY = "NVCF_API_KEY"
ENV_DEFAULT_URL = "ACE_ANIMATION_CONTROLLER_DEFAULT_URL"

TEST_DIR = os.path.dirname(__file__)  # .../tests
ROOT_DIR = os.path.abspath(os.path.join(TEST_DIR, "../../../"))  # repo root
TEST_AUDIO_FILEPATH = os.path.abspath(
    os.path.join(ROOT_DIR, "sample_data/audio_4sec_16k_s16le.wav")
)
