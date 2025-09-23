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
from typing import List, Tuple

import maya.cmds as cmds
import maya.internal.common.ae.custom as aecustom
import maya.internal.common.ae.template as aetemplate
from maya.api.OpenMaya import MGlobal
from maya.internal.common.ae.template import Layout, plugAttr

from .fixed import H_BTN, ICON_DIR, SCRIPTS_DIR, W_BTN, get_nice_name, get_snake_case


# Inspired by AEblendShapeAttrControl from maya/internal/nodes/blendshape/ae_template.py
class BaseArrayControl(aecustom.CustomControl):
    def __init__(self, fieldClass, *args, **kwargs):
        super(BaseArrayControl, self).__init__(*args, **kwargs)
        self.fieldClass = fieldClass

    @property
    def element_aliases(self) -> List[Tuple[str, str]]:
        elements = list(self._get_element_aliases())
        if not elements:
            return []

        elements.sort(key=lambda x: x[0])
        return elements

    def _get_element_aliases(self):
        attrName = plugAttr(self.plugName)
        aliases = cmds.aliasAttr(self.nodeName, query=True)

        if not aliases:
            return

        for i, alias in enumerate(aliases[::2]):
            attr = aliases[i * 2 + 1]
            if not attr.startswith(attrName + "["):
                continue
            idx = int(attr.split("[")[1].rstrip("]"))
            yield (idx, alias, attr)
        return

    def _build_element_fields(self):
        cmds.dgeval(self.plugName)  # force evaluation
        for _, alias, attr in self.element_aliases:
            field = self.fieldClass(label=self.nice_alias(alias), annotation=attr)
            field.plugName = f"{self.nodeName}.{alias}"
            field.nodeName = self.nodeName
            field.buildControlUI()

    def buildControlUI(self):
        attrName = plugAttr(self.plugName)
        layoutName = f"{attrName}Column"

        cmds.setUITemplate("attributeEditorTemplate", pst=True)
        self.columnLayout = cmds.columnLayout(layoutName)

        self._build_element_fields()

        cmds.setParent("..")
        cmds.setUITemplate(ppt=True)

    def replaceControlUI(self):
        # We could try to reuse the widgets as an optimization, but why bother.
        for child in cmds.layout(self.columnLayout, query=True, childArray=True) or []:
            cmds.deleteUI(child)

        cmds.setUITemplate("attributeEditorTemplate", pst=True)
        cmds.setParent(self.columnLayout)

        self._build_element_fields()

        cmds.setParent("..")
        cmds.setUITemplate(ppt=True)

    def nice_alias(self, alias):
        return get_nice_name(alias)


class ArrayListView(aecustom.CustomControl):
    """A dynamic length array viewer"""

    def buildControlUI(self):
        self._controls = []
        self.replaceControlUI()

    def replaceControlUI(self):
        attrname = plugAttr(self.plugName)
        attrlabel = self.build_kwargs.get("label", get_nice_name(attrname))

        cmds.dgeval(self.plugName)  # force evaluation
        length = cmds.getAttr(self.plugName, size=1)
        alias_list = cmds.aliasAttr(self.nodeName, q=1)
        aliases = {}
        if alias_list:
            # a dict of {attribute: alias}
            aliases = {alias_list[idx * 2 + 1]: al for idx, al in enumerate(alias_list[::2])}

        for i in range(min(length, len(self._controls))):
            # update controls with correct information
            ctrl = self._controls[i]
            name = f"{attrname}[{i}]"
            label = aliases.get(name, f"{attrlabel}[{i}]")
            cmds.attrControlGrp(
                ctrl,
                edit=True,
                label=label,
                annotation=name,
                attribute=f"{self.plugName}[{i}]",
                enable=True,
            )

        for i in range(length, len(self._controls)):
            # disable old entries (controls > length)
            ctrl = self._controls[i]
            cmds.attrControlGrp(
                ctrl,
                edit=True,
                enable=False,
            )

        for i in range(len(self._controls), length):
            # add new entries (length > controls)
            name = f"{attrname}[{i}]"
            label = aliases.get(name, f"{attrlabel}[{i}]")
            ctrl = cmds.attrControlGrp(
                label=label,
                annotation=name,
                attribute=f"{self.plugName}[{i}]",
            )
            self._controls.append(ctrl)
