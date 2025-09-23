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
from maya import cmds
from maya.internal.common.ae.template import Layout, plugAttr

from .arraycontrols import (
    ArrayListView,
    BaseArrayControl,
)
from .audiocontrols import AudioSelection
from .buttoncontrols import ResetAllElementsButton
from .fieldcontrols import PrecisionField
from .fixed import H_BTN, W_BTN, get_nice_name
from .modelcontrols import (
    IdentityIndex,
    LocalA2EModelPath,
    LocalA2FModelPath,
)

from .ae_a2f_shared import AEA2FShared

class AEA2FAnimationPlayer(AEA2FShared):

    def buildUI(self, nodeName):
        self.suppress("time")

        self.a2e_model_preset = LocalA2EModelPath(
            label="Audio2Emotion",
            annotation="Select a local audio2emotion model.",
        )
        self.a2f_model_preset = LocalA2FModelPath(
            label="Audio2Face",
            annotation="Select a local audio2face model.",
        )
        self.identity_index = IdentityIndex(
            label="Identity", 
            annotation="Select an identity to use on audio2face."
        )

        self.suppress("modelConfigs")
        with Layout(self, "Models", False):
            self.defineCustom(self.a2f_model_preset, ["a2fModelPath"])
            self.addControl(
                "a2fModelPath",
                label="Model Card",
                annotation="A path to the audio2face network's model.json file.",
                callback=lambda _: self.a2f_model_preset.populateOptions()
                or self.identity_index.populateOptions(),
            )
            self.defineCustom(self.identity_index, ["identityIndex"])
            self.addControl(
                "useGpuBlendshapeSolver", 
                annotation="Use GPU for blendshape solver.",
                label="Use GPU",
            )

            self.addSeparator()
            self.defineCustom(self.a2e_model_preset, ["a2eModelPath"])
            self.addControl(
                "a2eModelPath",
                label="Model Card",
                annotation="A path to the audio2emotion network's model.json file.",
                callback=lambda _: self.a2e_model_preset.populateOptions(),
            )

            self.defineCustom(ManageModelFilesButton(), [""])

        self.addAudioControls()
        self.addEmotionControls()
        self.addFaceControls()
        self.addTongueControls()
        self.addBlendshapeControls()

        self.suppress("outputAnimationResults")
        self.suppress("outputEmotionStates")
        self.suppress("outputInferenceResults")
        with Layout(self, "Output", True):
            with Layout(self, "Blendshsape Weights", True):
                self.defineCustom(ArrayListView(), ["outputWeights"])
            with Layout(self, "Emotion Results", True):
                self.defineCustom(ArrayListView(), ["outputEmotions"])
            self.addControl("jawTransform")
            self.addControl("rightEyeRotation")
            self.addControl("leftEyeRotation")
            self.addControl("faceWeightsCount")
            self.addControl("tongueWeightsCount")

        self.callTemplate("dependNode")


# define node specific controls
class ManageModelFilesButton(aecustom.CustomControl):
    def buildControlUI(self):
        cmds.rowLayout(numberOfColumns=2, adj=2)
        self._spacer = cmds.text("", width=136)  # align left
        cmds.button(
            label="Manage Model Files",
            annotation="Manage TnesorRT files for the selected model.",
            command=self.onRequest,
            height=H_BTN,
        )
        cmds.setParent("..")

    def onRequest(self, other):
        from trtgen.builder_ui import show_trt_builder_dialog

        show_trt_builder_dialog(self.nodeName)
