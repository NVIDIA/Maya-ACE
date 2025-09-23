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


class AEA2FShared(aetemplate.Template):

    def buildUI(self, nodeName):
        pass

    def addAudioControls(self):
        self.suppress("audioParameters")
        with Layout(self, "Audio", False):
            self.audio_menu = AudioSelection()
            self.addControl(
                "audiofile",
                annotation="A filepath to a valid audio file. The file will be read by this node before sending it.",
                callback=lambda _: self.audio_menu.populateAudios(),
            )
            self.defineCustom(self.audio_menu, ["audiofile"])

    def addEmotionControls(self):
        self.suppress("emotionParameters")
        with Layout(self, "Emotion", False):
            self.defineCustom(
                PrecisionField(
                    label="Strength", 
                    annotation="Controls the intensity of the final emotion"
                ), 
                ["emotionStrength"]
            )

            with Layout(self, "Preferred Emotion", False):
                self.addControl(
                    "enablePreferredEmotion",
                    label="Enable",
                    annotation="Enable adding user-defined emotion to generated emotion.",
                )
                self.defineCustom(
                    PrecisionField(
                        label="Weight", 
                        annotation="Sets the strength of the preferred emotion (if is loaded) relative to generated emotions"
                    ), 
                    ["preferredEmotionStrength"]
                )

                self.addSeparator()
                self.defineCustom(ResetAllElementsButton(), ["preferredEmotions"])
                # Preferred emotion represents a single static user-defined emotion (main 'tone') and is used in a mixture with generated emotions
                self.defineCustom(AliasArrayControl(), ["preferredEmotions"])

            with Layout(self, "Auto Emotion", False):
                self.defineCustom(PrecisionField(annotation="Increases the spread between emotion values by pushing them higher or lower"), ["emotionContrast"])
                self.defineCustom(
                    PrecisionField(
                        label="Smoothing",
                        annotation="Coefficient for exponential smoothing of emotion"
                    ), 
                    ["liveBlendCoef"]
                )
                self.defineCustom(PrecisionField(annotation="Sets a firm limit on the quantity of emotion sliders engaged by A2E - emotions with highest weight will be prioritized"), ["maxEmotions"])

    def addFaceControls(self):
        self.suppress("faceParameters")
        with Layout(self, "Face", False):
            self.defineCustom(ResetAllElementsButton(), ["faceParameters"])
            self.defineCustom(
                PrecisionField(annotation="Applies temporal smoothing to the lower face motion"), 
                ["lowerFaceSmoothing"]
            )
            self.defineCustom(
                PrecisionField(annotation="Applies temporal smoothing to the upper face motion"), 
                ["upperFaceSmoothing"]
            )
            self.defineCustom(
                PrecisionField(annotation="Controls the range of motion on the lower regions of the face"), 
                ["lowerFaceStrength"]
            )
            self.defineCustom(
                PrecisionField(annotation="Controls the range of motion on the upper regions of the face"), 
                ["upperFaceStrength"]
            )
            self.defineCustom(
                PrecisionField(annotation="Determines the boundary between the upper and lower regions of the face"), 
                ["faceMaskLevel"]
            )
            self.defineCustom(
                PrecisionField(annotation="Determines how smoothly the upper and lower face regions blend on the boundary"), 
                ["faceMaskSoftness"]
            )
            self.defineCustom(
                PrecisionField(annotation="Controls the range of motion of the skin"), 
                ["skinStrength"]
            )
            self.defineCustom(
                PrecisionField(annotation="Adjusts the default pose of eyelid open-close"), 
                ["eyelidOpenOffset"]
            )
            self.defineCustom(
                PrecisionField(annotation="Adjusts the default pose of lip close-open"), 
                ["lipOpenOffset"]
            )

    def addTongueControls(self):
        self.suppress("tongueParameters")
        with Layout(self, "Tongue", False):
            self.defineCustom(ResetAllElementsButton(), ["tongueParameters"])
            self.defineCustom(
                PrecisionField(annotation="Controls the range of motion of the tongue"), 
                ["tongueStrength"]
            )
            self.defineCustom(PrecisionField(annotation="Adjusts the default height of the tongue"), 
                ["tongueHeightOffset"]
            )
            self.defineCustom(PrecisionField(annotation="Adjusts the default depth of the tongue"), 
                ["tongueDepthOffset"]
            )

    def addBlendshapeControls(self, enable_tongue=True):
        self.suppress("blendshapeParameters")
        with Layout(self, "Blendshape", True):
            with Layout(self, "Face Multipliers", True):
                self.defineCustom(AliasArrayControl(), ["faceMultipliers"])
            with Layout(self, "Face Offsets", True):
                self.defineCustom(AliasArrayControl(), ["faceOffsets"])
            if enable_tongue:
                with Layout(self, "Tongue Multipliers", True):
                    self.defineCustom(AliasArrayControl(), ["tongueMultipliers"])
                with Layout(self, "Tongue Offsets", True):
                    self.defineCustom(AliasArrayControl(), ["tongueOffsets"])


# define node specific controls
class AliasArrayControl(BaseArrayControl):
    def __init__(self, *args, **kwargs):
        super().__init__(PrecisionField, *args, **kwargs)

    def nice_alias(self, alias):
        return get_nice_name(alias.split("_")[-1])
