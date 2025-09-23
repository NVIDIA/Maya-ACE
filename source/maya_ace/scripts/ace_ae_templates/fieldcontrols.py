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


class PrecisionField(aecustom.CustomControl):
    """A 4-digit float slider control with a reset button"""

    def buildControlUI(self):
        cmds.rowLayout(numberOfColumns=2, ad2=1, columnWidth2=(1, W_BTN))
        self._control = cmds.attrFieldSliderGrp(
            attribute=self.plugName,
            adj=0,
            precision=4,
        )
        self._button = cmds.button(label="Reset", width=W_BTN, height=H_BTN, command=self.onReset)
        cmds.setParent("..")

        # NOTE: force updating the field with correct settings.
        self.replaceControlUI()

    def onReset(self, other):
        defaults = cmds.attributeQuery(
            plugAttr(self.plugName), node=self.nodeName, listDefault=True
        )
        if defaults != None and len(defaults) == 1:
            cmds.setAttr(self.plugName, defaults[0])

    def replaceControlUI(self):
        attrname = plugAttr(self.plugName)
        label = self.build_kwargs.get("label", get_nice_name(attrname))
        annotation = self.build_kwargs.get("annotation", attrname)

        cmds.attrFieldSliderGrp(
            self._control,
            edit=True,
            label=label,
            annotation=annotation,
            attribute=self.plugName,
            precision=4,
        )
