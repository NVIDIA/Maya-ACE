# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

from .controls import AudioSelection, PrecisionField, RequestButton


class AEAceAnimationPlayer(aetemplate.Template):

    def buildUI(self, nodeName):
        self.suppress("time")

        self.addControl(
            "networkAddress",
            annotation="A url or an ip address to the service. Please specify http:// or https:// correctly.",
        )
        self.addControl("apiKey", annotation="An nvcf api key if accesing to nvcf address.")
        self.addControl("functionId", annotation="A function id for Audio2Face service on nvcf.")
        self.defineCustom(RequestButton(), ["triggerRequest"])

        self.suppress("audio")
        with Layout(self, "Audio", False):
            self.addControl("audiofile")
            self.defineCustom(AudioSelection(), ["audiofile"])

        self.suppress("emotions")
        self.suppress("emotionParameters")
        with Layout(self, "Emotion Parameters", False):
            self.defineCustom(PrecisionField(), ["emotionStrength"])

            with Layout(self, "Preferred Emotion", False):
                self.addControl("enablePreferredEmotion", label="Enable", annotation="enablePreferredEmotion")
                self.defineCustom(PrecisionField(label="Weight"), ["preferredEmotionStrength"])
                self.addSeparator()
                self.addSeparator()

                self.defineCustom(PrecisionField(), ["amazement"])
                self.defineCustom(PrecisionField(), ["anger"])
                self.defineCustom(PrecisionField(), ["cheekiness"])
                self.defineCustom(PrecisionField(), ["disgust"])
                self.defineCustom(PrecisionField(), ["fear"])
                self.defineCustom(PrecisionField(), ["grief"])
                self.defineCustom(PrecisionField(), ["joy"])
                self.defineCustom(PrecisionField(), ["outofbreath"])
                self.defineCustom(PrecisionField(), ["pain"])
                self.defineCustom(PrecisionField(), ["sadness"])

            with Layout(self, "Auto Emotion", False):
                self.defineCustom(PrecisionField(), ["emotionContrast"])
                self.defineCustom(PrecisionField(label="Smoothing"), ["liveBlendCoef"])
                self.defineCustom(PrecisionField(), ["maxEmotion"])

        self.suppress("faceParameters")
        with Layout(self, "Face Parameters", False):
            self.defineCustom(PrecisionField(), ["lowerFaceSmoothing"])
            self.defineCustom(PrecisionField(), ["upperFaceSmoothing"])
            self.defineCustom(PrecisionField(), ["lowerFaceStrength"])
            self.defineCustom(PrecisionField(), ["upperFaceStrength"])
            self.defineCustom(PrecisionField(), ["faceMaskLevel"])
            self.defineCustom(PrecisionField(), ["faceMaskSoftness"])
            self.defineCustom(PrecisionField(), ["skinStrength"])
            self.defineCustom(PrecisionField(), ["eyelidOpenOffset"])
            self.defineCustom(PrecisionField(), ["lipOpenOffset"])

        self.suppress("blendshapeParameters")
        with Layout(self, "Blendshape Multiplier", True):
            for bs_name in arkit_faces:
                self.defineCustom(PrecisionField(label=bs_name), ["multiply_" + bs_name])
        with Layout(self, "Blendshape Offset", True):
            for bs_name in arkit_faces:
                self.defineCustom(PrecisionField(label=bs_name), ["offset_" + bs_name])

        self.suppress("status")
        with Layout(self, "Status", True):
            self.addControl("currentFrame")
            self.addControl("loaded")
            self.addControl("loadedAudio")
            self.addControl("audioSamples")
            self.addControl("received")
            self.addControl("receivedTime")
            self.addControl("receivedFrames")

        self.suppress("output")

        self.callTemplate("dependNode")


arkit_faces = [
    # Arkit 52 Expressions
    "EyeBlinkLeft",
    "EyeLookDownLeft",
    "EyeLookInLeft",
    "EyeLookOutLeft",
    "EyeLookUpLeft",
    "EyeSquintLeft",
    "EyeWideLeft",
    "EyeBlinkRight",
    "EyeLookDownRight",
    "EyeLookInRight",
    "EyeLookOutRight",
    "EyeLookUpRight",
    "EyeSquintRight",
    "EyeWideRight",
    "JawForward",
    "JawLeft",
    "JawRight",
    "JawOpen",
    "MouthClose",
    "MouthFunnel",
    "MouthPucker",
    "MouthLeft",
    "MouthRight",
    "MouthSmileLeft",
    "MouthSmileRight",
    "MouthFrownLeft",
    "MouthFrownRight",
    "MouthDimpleLeft",
    "MouthDimpleRight",
    "MouthStretchLeft",
    "MouthStretchRight",
    "MouthRollLower",
    "MouthRollUpper",
    "MouthShrugLower",
    "MouthShrugUpper",
    "MouthPressLeft",
    "MouthPressRight",
    "MouthLowerDownLeft",
    "MouthLowerDownRight",
    "MouthUpperUpLeft",
    "MouthUpperUpRight",
    "BrowDownLeft",
    "BrowDownRight",
    "BrowInnerUp",
    "BrowOuterUpLeft",
    "BrowOuterUpRight",
    "CheekPuff",
    "CheekSquintLeft",
    "CheekSquintRight",
    "NoseSneerLeft",
    "NoseSneerRight",
    "TongueOut",
]
