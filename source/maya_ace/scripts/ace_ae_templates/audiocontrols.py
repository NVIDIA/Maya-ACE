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
import maya.cmds as cmds
import maya.internal.common.ae.custom as aecustom
import maya.internal.common.ae.template as aetemplate
from ace_tools.audio import (
    find_audio_node_from_path,
    get_time_slider_audio,
    set_time_slider_audio,
)
from ace_tools.setup import connect_audio_to_animation_player, disconnect_audio
from maya.api.OpenMaya import MGlobal
from maya.internal.common.ae.template import Layout, plugAttr

from .fixed import H_BTN, W_BTN


class AudioSelection(aecustom.CustomControl):
    """Audiofile field ui with audio node selection"""

    def buildControlUI(self):
        """
        Components:
            - select different audio in the scene
            - (future) import a new audio through a file browser
        """
        self._parent = cmds.setParent(q=True)

        # First row: align attribute label spacing in the template
        maya_version = cmds.about(q=1, version=1)
        if maya_version < "2024":
            col = cmds.columnLayout(
                adjustableColumn=True,
            )
            row1 = cmds.rowLayout(numberOfColumns=3, adjustableColumn=2)
        else:
            col = cmds.columnLayout(
                adjustableColumn=True,
                generalSpacing=2,  # to match to the spacing of AE layout
            )
            row1 = cmds.rowLayout(numberOfColumns=3, adjustableColumn=2, margins=0)

        self._spacer1 = cmds.text(label="")
        self._options = cmds.optionMenu(
            annotation="Select an audio to be connected.",
            changeCommand=self.onOptionSelected,
            beforeShowPopup=lambda *_: self.populateAudios(),
        )
        self._btn_select = cmds.button(
            label="Select",
            annotation="Select the currently connected audio node.",
            width=W_BTN,
            height=H_BTN,
            command=self.selectAudio,
        )
        cmds.setParent(col)

        row2 = cmds.rowLayout(numberOfColumns=2, adj=2)
        self._spacer2 = cmds.text(label="")  # align left
        self._btn_set_time_slider = cmds.button(
            label="Update Time Slider",
            annotation="Set the global time slider with the current audio and fit the time range",
            height=H_BTN,
            command=self.onSetTimeSlider,
        )
        cmds.setParent(col)

        cmds.setParent("..")  # go back to parent layout
        self.populateAudios()

    def selectAudio(self, *args):
        audio_input = cmds.ls(cmds.listConnections(self.plugName), type="audio")
        cmds.select(audio_input, replace=1)

    def populateAudios(self):
        # cleanup
        cmds.optionMenu(self._options, edit=1, deleteAllItems=1)

        # check the current connection
        audio_input = cmds.ls(cmds.listConnections(self.plugName), type="audio")

        # the default empty option
        selection = 0
        cmds.menuItem(label="", parent=self._options)

        for audio_node, filepath in find_audio_node_from_path("*"):
            item = cmds.menuItem(label=audio_node, parent=self._options)
            if audio_node in audio_input:
                # ssave selection for the current audio node
                selection = cmds.optionMenu(self._options, query=1, numberOfItems=1)

        # change current selection
        if selection > 0:
            cmds.optionMenu(self._options, edit=1, select=selection)
            cmds.button(self._btn_select, edit=1, enable=True)
            cmds.button(self._btn_set_time_slider, edit=1, enable=True)
        else:
            cmds.button(self._btn_select, edit=1, enable=False)
            cmds.button(self._btn_set_time_slider, edit=1, enable=False)

    def onGetCurrent(self, other):
        audio = get_time_slider_audio()
        if not audio:
            MGlobal.displayError("Cannot find an audio. Please set a single audio to the Timeline.")
            return
        connect_audio_to_animation_player(audio, self.nodeName)
        MGlobal.displayInfo(f"Connected '{audio}' to '{self.nodeName}'")

    def onOptionSelected(self, item):
        audionode = str(item)
        if not audionode:
            # empty option is selected. disconnect the current audio
            disconnect_audio(self.nodeName)
            return

        connect_audio_to_animation_player(audionode, self.nodeName)
        if cmds.objExists(f"{self.nodeName}.triggerLoad"):
            cmds.dgeval(f"{self.nodeName}.triggerLoad")

        MGlobal.displayInfo(f"Changed audio connection to '{audionode}' of '{self.nodeName}'")

    def replaceControlUI(self):
        self.populateAudios()

    def onSetTimeSlider(self, *args):
        """Set the global time slider with the current audio and fit the time range"""
        # Get the currently connected audio
        audio_connections = cmds.listConnections(self.plugName, type="audio")
        if not audio_connections:
            MGlobal.displayError("No audio connected to set on the time slider.")
            return

        audio_node = audio_connections[0]
        try:
            set_time_slider_audio(audio_node, fit_range=True)
            MGlobal.displayInfo(f"Set time slider audio to '{audio_node}' with fitted time range.")
        except Exception as e:
            MGlobal.displayError(f"Failed to set time slider audio: {e}")
