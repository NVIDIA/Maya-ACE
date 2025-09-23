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
import os
import re

import maya.cmds as cmds
import maya.internal.common.ae.custom as aecustom
import maya.internal.common.ae.template as aetemplate
from maya.api.OpenMaya import MGlobal
from maya.internal.common.ae.template import Layout, plugAttr

from .fixed import H_BTN, ICON_DIR, SCRIPTS_DIR, W_BTN, get_nice_name, get_snake_case


class RequestButton(aecustom.CustomControl):
    """A request animation button
    also receives animation from the audio2face-3d service
    """

    def buildControlUI(self):
        cmds.rowLayout(numberOfColumns=3, adj=2)
        self._spacer = cmds.text(label="", width=143)  # align left
        self._button = cmds.button(
            label="Request New Animation",
            annotation="Connect to the service, send audio, and receive new animation.",
            command=self.onRequest,
            height=H_BTN,
        )
        self._indicator = cmds.image(
            image=os.path.join(ICON_DIR, "grey.svg"),
            width=H_BTN,
            height=H_BTN,
            annotation="Idle",
        )
        self.replaceControlUI()

    def onRequest(self, other):
        cmds.AceRequestSendAudio(self.nodeName)
        self.replaceControlUI()

    def replaceControlUI(self):
        if cmds.getAttr(f"{self.nodeName}.receivedTime") == 0:
            cmds.image(
                self._indicator,
                edit=True,
                image=os.path.join(ICON_DIR, "grey.svg"),
                annotation="Idle",
            )
        elif cmds.getAttr(f"{self.nodeName}.received"):
            if cmds.getAttr(f"{self.nodeName}.needsUpdate"):
                cmds.image(
                    self._indicator,
                    edit=True,
                    image=os.path.join(ICON_DIR, "yellow.svg"),
                    annotation="Needs new animation",
                )
            else:
                cmds.image(
                    self._indicator,
                    edit=True,
                    image=os.path.join(ICON_DIR, "green.svg"),
                    annotation="Connected",
                )
        else:
            cmds.image(
                self._indicator,
                edit=True,
                image=os.path.join(ICON_DIR, "red.svg"),
                annotation="Error - Please check output window for messages",
            )
        cmds.scriptJob(
            attributeChange=[f"{self.nodeName}.received", lambda *_: self.replaceControlUI()],
            parent=self._indicator,
            # replacePrevious=True,
            runOnce=True,
        )


class ResetAllElementsButton(aecustom.CustomControl):
    """A reset all button for array elements"""

    def buildControlUI(self):
        cmds.rowLayout(numberOfColumns=2, adj=2)
        self._spacer1 = cmds.text(label="", width=143)  # align left
        self._button = cmds.button(label="Reset All", height=H_BTN, command=self.onReset)
        cmds.setParent("..")

    def onReset(self, other):
        attr = plugAttr(self.plugName)
        node = self.nodeName
        for member in cmds.listAttr(self.plugName, multi=True) or []:
            defaults = cmds.attributeQuery(member, node=node, listDefault=True)
            if defaults != None and len(defaults) == 1:
                cmds.setAttr(f"{node}.{member}", defaults[0])
