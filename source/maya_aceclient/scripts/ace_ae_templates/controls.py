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
import os
import re

import maya.cmds as cmds
import maya.internal.common.ae.custom as aecustom
import maya.internal.common.ae.template as aetemplate
from maya.api.OpenMaya import MGlobal
from maya.internal.common.ae.template import Layout, plugAttr

import ace_tools

from ace_tools import preset

SCRIPTS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ICON_DIR = os.path.join(SCRIPTS_DIR, "icon")

H_BTN = 19
W_BTN = 60
# H_OPTION = 20  # default value


class NetworkPreset(aecustom.CustomControl):
    """Provides a quick network setup"""

    def buildControlUI(self):
        """
        Components:
            - a list of network presets
        """
        self._parent = cmds.setParent(q=True)
        self._options = {}

        client_types = cmds.attributeQuery(
            "clientType", node=self.nodeName, listEnum=1) or []
        self._client_type_names = {k: i for i, k in enumerate(client_types[0].split(":"))}

        # align attribute label spacing in the template
        row = cmds.rowLayout(numberOfColumns=2, adjustableColumn=2)
        self.label = cmds.text("Network Preset")
        self._option_menu = cmds.optionMenu(
            annotation="Select an audio to be connected.",
            changeCommand=self.onOptionSelected,
            beforeShowPopup=lambda *_: self.populateOptions(),
        )
        self.populateOptions()

    def populateOptions(self):
        # save current settings
        current_settings = self.getCurrentSettings()

        # clear current items
        cmds.optionMenu(self._option_menu, edit=1, deleteAllItems=1)

        # the default empty option
        selection = 0
        cmds.menuItem(label="Custom", parent=self._option_menu)
        self._options["Custom"] = current_settings

        for i, data in enumerate(preset.read_presets()):
            label = data.get("__label__")
            item = cmds.menuItem(label=label, parent=self._option_menu)
            settings = {}
            for key, val in data.items():
                if key.startswith("_"):
                    continue
                if key == "clientType":
                    # convert enum string to enum index
                    settings[key] = self._client_type_names.get(val, 0)
                else:
                    settings[key] = val
            self._options[label] = settings

            # detect if the current setting is using existing preset
            if self.hasAllValues(settings, current_settings):
                selection = cmds.optionMenu(self._option_menu, query=1, numberOfItems=1)

        # change current selection
        if selection > 0:
            selection = cmds.optionMenu(self._option_menu, edit=1, select=selection)

    def getCurrentSettings(self):
        node = self.nodeName

        attributes = [
            "networkAddress",
            "clientType",
            "functionId",
        ]
        settings = {}
        for attr in attributes:
            settings[attr] = cmds.getAttr(f"{node}.{attr}")

        return settings

    def hasAllValues(self, values: dict, included: dict):
        for key, val1 in values.items():
            if val1 == included.get(key):
                continue
            return False

        return True

    def onOptionSelected(self, item):
        label = str(item)
        if label == "Custom":
            return
        node = self.nodeName
        data = self._options.get(label, {})
        for attr, val in data.items():
            if attr.startswith("__"):
                continue
            path = f"{node}.{attr}"
            if not cmds.objExists(path):
                MGlobal.displayWarning(f"Non-existing attribute: '{path}'")
                continue
            cmds.setAttr(path, lock=0)
            if isinstance(val, str):
                cmds.setAttr(path, val, type="string")
            else:
                cmds.setAttr(path, val)
        MGlobal.displayInfo(f"Changed network preset to '{label}'.")

    def replaceControlUI(self):
        self.populateOptions()


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
        row = cmds.rowLayout(numberOfColumns=3, adjustableColumn=2)
        self._spacer = cmds.text("")
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

        for audio_node, filepath in ace_tools.find_audio_node_from_path("*"):
            item = cmds.menuItem(label=audio_node, parent=self._options)
            if audio_node in audio_input:
                # ssave selection for the current audio node
                selection = cmds.optionMenu(self._options, query=1, numberOfItems=1)

        # change current selection
        if selection > 0:
            cmds.optionMenu(self._options, edit=1, select=selection)
            cmds.button(self._btn_select, edit=1, enable=True)
        else:
            cmds.button(self._btn_select, edit=1, enable=False)

    def onGetCurrent(self, other):
        audio = ace_tools.get_time_slider_audio()
        if not audio:
            MGlobal.displayError(
                "Cannot find an audio. Please set a single audio to the Timeline.")
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
        MGlobal.displayInfo(
            f"Changed audio connection to '{audionode}' of '{self.nodeName}'")

    def replaceControlUI(self):
        self.populateAudios()


class RequestButton(aecustom.CustomControl):
    """A send audio button
        also receives animation on streaming or start session on authoring
    """

    def buildControlUI(self):
        cmds.rowLayout(numberOfColumns=3, adj=2)
        self._spacer = cmds.text("", width=136)  # align left
        self._button = cmds.button(
            label="Connect and Send Audio",
            annotation="Connect to the service and send the audio.",
            command=self.onRequest,
            height=H_BTN,
        )
        self._indicator = cmds.image(
            image=os.path.join(ICON_DIR, "grey.svg"), width=H_BTN, height=H_BTN,
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


class PrecisionField(aecustom.CustomControl):
    """A 4-digit float slider control with a reset button"""

    def buildControlUI(self):
        cmds.rowLayout(numberOfColumns=2, ad2=1, columnWidth2=(1, W_BTN))
        self._control = cmds.attrFieldSliderGrp(
            attribute=self.plugName,
            adj=0,
            precision=4,
        )
        cmds.button(
            label="Reset",
            width=W_BTN,
            height=H_BTN,
            command=self.onReset
        )

        # NOTE: force updating the field with correct settings.
        self.replaceControlUI()

    def onReset(self, other):
        defaults = cmds.attributeQuery(
            plugAttr(self.plugName), node=self.nodeName, listDefault=True)
        if defaults != None and len(defaults) == 1:
            cmds.setAttr(self.plugName, defaults[0])

    def replaceControlUI(self):
        attrname = plugAttr(self.plugName)
        label = self.build_kwargs.get("label", get_nice_name(attrname))

        cmds.attrFieldSliderGrp(
            self._control,
            edit=True,
            label=label,
            annotation=attrname,
            attribute=self.plugName,
            precision=4,
        )


class ArrayListView(aecustom.CustomControl):
    """A dynamic length array viewer"""

    def buildControlUI(self):
        self._controls = []
        self.replaceControlUI()

    def replaceControlUI(self):
        attrname = plugAttr(self.plugName)
        attrlabel = self.build_kwargs.get("label", get_nice_name(attrname))

        length = cmds.getAttr(self.plugName, size=1)
        al_list = cmds.aliasAttr(self.nodeName, q=1)
        aliases = {}
        if al_list:
            aliases = {at: al for al, at in zip(al_list[::2], al_list[1::2])}

        for i in range(min(length, len(self._controls))):
            # update controls with correct information
            ctrl = self._controls[i]
            name = f'{attrname}[{i}]'
            label = aliases.get(name, f'{attrlabel}[{i}]')
            cmds.attrControlGrp(
                ctrl,
                edit=True,
                label=label,
                annotation=name,
                attribute=f'{self.plugName}[{i}]',
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
            name = f'{attrname}[{i}]'
            label = aliases.get(name, f'{attrlabel}[{i}]')
            ctrl = cmds.attrControlGrp(
                label=label,
                annotation=name,
                attribute=f'{self.plugName}[{i}]',
            )
            self._controls.append(ctrl)


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
