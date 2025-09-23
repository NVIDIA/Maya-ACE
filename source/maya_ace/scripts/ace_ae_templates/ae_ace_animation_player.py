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
import maya.internal.common.ae.custom as aecustom
import maya.internal.common.ae.template as aetemplate
from maya.api.OpenMaya import MGlobal
from maya.internal.common.ae.template import Layout, plugAttr

from .arraycontrols import ArrayListView, BaseArrayControl
from .audiocontrols import AudioSelection
from .buttoncontrols import RequestButton, ResetAllElementsButton
from .fieldcontrols import PrecisionField
from .fixed import get_nice_name
from .presetcontrol import PresetControl

from .ae_a2f_shared import AEA2FShared, AliasArrayControl


class AEAceAnimationPlayer(AEA2FShared):
    def __init__(self, nodeName):
        self.nodeName = nodeName  # duplicate, but needed for adding custom controls

        self.defineCustom(RequestButton(), ["triggerSendAudio"])
        super().__init__(nodeName)

    def buildUI(self, nodeName):
        self.suppress("time")

        self.suppress("serviceParameters")
        self.network_preset = NetworkPresetControl(
            label="Preset",
            annotation="Select an ACE or a custom preset.",
        )
        with Layout(self, "Service", False):
            self.defineCustom(self.network_preset, ["networkAddress", "functionId"])
            self.addControl(
                "networkAddress",
                label="Address",
                annotation="A url or an ip address to the service. Please specify http:// or https:// correctly.",
                callback=lambda _: self.network_preset.populateOptions(),
            )
            self.addControl(
                "functionId",
                label="Function ID",
                annotation="A function id for Audio2Face-3D service on nvcf.",
                callback=lambda _: self.network_preset.populateOptions(),
            )
            self.addSeparator()
            self.addControl(
                "apiKey",
                label="API Key",
                annotation="An nvcf api key if accesing to nvcf address.",
            )

        self.addAudioControls()
        self.addEmotionControls()
        self.addFaceControls()
        self.addBlendshapeControls(enable_tongue=False)

        self.suppress("status")
        with Layout(self, "Status", True):
            self.addControl("currentFrame", annotation="The current animation frame.")
            self.addControl("loaded", annotation="True when audio file is loaded.")
            self.addControl("loadedAudio", annotation="The loaded audio file path.")
            self.addControl("audioSamples", annotation="The number of samples of used audio data.")
            self.addControl("received", annotation="True when valid animation is received.")
            self.addControl(
                "receivedTime", annotation="A tick count when the latest animation is received."
            )
            self.addControl("receivedFrames", annotation="The number of received animation frames.")
            self.addControl("needsUpdate", annotation="The number of received animation frames.")

        self.suppress("outputAnimationResults")
        self.suppress("outputEmotionStates")
        with Layout(self, "Output", True):
            with Layout(self, "Blendshape Weights", True):
                self.defineCustom(ArrayListView(), ["outputWeights"])
            with Layout(self, "Emotion Results", True):
                self.defineCustom(ArrayListView(), ["outputEmotions"])

        self.callTemplate("dependNode")


# define node specific controls
class NetworkPresetControl(PresetControl):
    """Provides a quick network setup"""

    affecting_attributes = ["networkAddress", "functionId"]

    def listOptions(self):
        from ace_tools import preset

        for i, data in enumerate(preset.read_presets()):
            yield data
        return
