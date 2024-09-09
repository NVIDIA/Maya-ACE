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
import re

import ace_tools
import maya.cmds as cmds
import maya.internal.common.ae.custom as aecustom
import maya.internal.common.ae.template as aetemplate
from maya.api.OpenMaya import MGlobal
from maya.internal.common.ae.template import Layout, plugAttr


class AudioSelection(aecustom.CustomControl):
    """Audiofile field ui with audio node selection"""

    def buildControlUI(self):
        """
        Components:
            - select different audio in the scene
            - (future) import a new audio through a file browser
        """
        self._parent = cmds.setParent(q=True)

        # align attribute label spacing in the template
        row = cmds.rowLayout(numberOfColumns=1, adjustableColumn=1)
        self.optionControl = cmds.optionMenu(
            annotation="Select an audio to be connected.",
            changeCommand=self.onOptionSelected,
        )
        self.populateAudios()
        # self.btnSync = cmds.button(label="Get Current", width=100, command=self.onGetCurrent)

    def populateAudios(self):
        # cleanup
        cmds.optionMenu(self.optionControl, edit=1, deleteAllItems=1)

        # check the current connection
        audio_input = cmds.ls(cmds.listConnections(self.plugName), type="audio")

        # the default empty option
        selection = 0
        cmds.menuItem(label="", parent=self.optionControl)

        for audio_node, filepath in ace_tools.find_audio_node_from_path("*"):
            item = cmds.menuItem(label=audio_node, parent=self.optionControl)
            if audio_node in audio_input:
                # ssave selection for the current audio node
                selection = cmds.optionMenu(self.optionControl, query=1, numberOfItems=1)

        # change current selection
        if selection > 0:
            selection = cmds.optionMenu(self.optionControl, edit=1, select=selection)

    def onGetCurrent(self, other):
        audio = ace_tools.get_time_slider_audio()
        if not audio:
            MGlobal.displayError("Cannot find an audio. Please set a single audio to the Timeline.")
            return
        ace_tools.connect_audio_to_animation_player(audio, self.nodeName)
        MGlobal.displayInfo(f"Connected '{audio}' to '{self.nodeName}'")

    def onOptionSelected(self, item):
        audionode = str(item)
        if not audionode:
            # empty option is selected. disconnect the current audio
            ace_tools.disconnect_audio(self.nodeName)
            return
        ace_tools.connect_audio_to_animation_player(audionode, self.nodeName)
        cmds.dgeval(f"{self.nodeName}.triggerLoad")
        MGlobal.displayInfo(f"Changed audio connection to '{audionode}' of '{self.nodeName}'")

    def replaceControlUI(self):
        self.populateAudios()


class RequestButton(aecustom.CustomControl):
    """A request new animation button"""

    def buildControlUI(self):
        cmds.rowLayout(numberOfColumns=1, adjustableColumn1=1)
        cmds.button(label="Request New Animation", command=self.onRequest)

    def onRequest(self, other):
        cmds.AceRequestAnimation(self.nodeName)

    def replaceControlUI(self):
        pass


class PrecisionField(aecustom.CustomControl):
    """A 4-digit float slider control with a reset button"""

    def buildControlUI(self):
        attrname = plugAttr(self.plugName)
        label = self.build_kwargs.get("label", get_nice_name(attrname))

        cmds.rowLayout(numberOfColumns=2, ad2=1, columnWidth2=(200, 50))
        self.createdControl = cmds.attrFieldSliderGrp(
            label=label,
            annotation=attrname,
            attribute=self.plugName,
            adj=0,
            precision=4,
        )
        cmds.button(label="Reset", width=100, command=self.onReset)

    def onReset(self, other):
        defaults = cmds.attributeQuery(plugAttr(self.plugName), node=self.nodeName, listDefault=True)
        if defaults != None and len(defaults) == 1:
            cmds.setAttr(self.plugName, defaults[0])

    def replaceControlUI(self):
        attrname = plugAttr(self.plugName)
        label = self.build_kwargs.get("label", get_nice_name(attrname))

        cmds.attrFieldSliderGrp(
            self.createdControl,
            edit=True,
            label=label,
            annotation=attrname,
            attribute=self.plugName,
        )


def get_nice_name(name):
    """Convert a name to a nice ui name.

    Example:
        camelCase -> Camel Case
        PascalCase -> Pascal Case
        snake_case -> Snake Case

    Args:
        name (str): a name to convert

    Returns:
        (str) a nicely formatted name
    """
    # camel case to snake
    s1 = get_snake_case(name)
    return " ".join(word.title() for word in s1.split("_"))


def get_snake_case(name):
    """Convert a name to snake case

    Args:
        name (str): a name to convert

    Returns:
        (str) a snake_case name
    """
    # 0Aa -> 0_Aa
    s1 = re.sub("(.)([A-Z][a-z]+)", r"\1_\2", name)
    # __A -> _A
    s2 = re.sub("__([A-Z])", r"_\1", s1)
    # 0A -> 0_A
    s3 = re.sub("([a-z0-9])([A-Z])", r"\1_\2", s2)
    return s3.lower()
