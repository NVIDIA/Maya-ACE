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
from maya.api.OpenMaya import MGlobal


class PresetControl(aecustom.CustomControl):
    """Provides a quick preset options"""

    affecting_attributes = []
    add_custom_option = True

    def buildControlUI(self):
        """
        Components:
            - a list of presets
        """
        self._parent = cmds.setParent(q=True)
        self._options = {}

        # align attribute label spacing in the template
        row = cmds.rowLayout(numberOfColumns=2, adjustableColumn=2)
        label = self.build_kwargs.get("label", "")
        annotation = self.build_kwargs.get("annotation", "")
        self._label = cmds.text(label=label)
        self._option_menu = cmds.optionMenu(
            annotation=annotation,
            changeCommand=self.onOptionSelected,
            beforeShowPopup=lambda *_: self.populateOptions(),
        )
        self.populateOptions()
        cmds.setParent(self._parent)

    def populateOptions(self):
        # save current settings
        current_settings = self.getCurrentSettings()

        # clear current items
        cmds.optionMenu(self._option_menu, edit=1, deleteAllItems=1)

        # the default empty option
        selection = 0
        if self.add_custom_option:
            cmds.menuItem(label="Custom", parent=self._option_menu)
            self._options["Custom"] = current_settings

        available_options = list(self.listOptions())

        # disable if there are no options
        if len(available_options) == 0:
            cmds.text(self._label, edit=1, enable=0)
            cmds.optionMenu(self._option_menu, edit=1, enable=0)
        else:
            cmds.text(self._label, edit=1, enable=1)
            cmds.optionMenu(self._option_menu, edit=1, enable=1)

        # populate the options
        for item_count, data in enumerate(available_options):
            label = data.get("__label__", f"{item_count}")
            item = cmds.menuItem(label=label, parent=self._option_menu)
            settings = {}
            for key, val in data.items():
                if key == "__label__":
                    continue
                else:
                    settings[key] = val
            self._options[label] = settings

            # detect if the current setting is using existing preset, and select it
            if self.hasAllValues(settings, current_settings):
                selection = cmds.optionMenu(self._option_menu, query=1, numberOfItems=1)

        # change current selection
        if selection > 0:
            selection = cmds.optionMenu(self._option_menu, edit=1, select=selection)

    def listOptions(self):
        return []

    def getCurrentSettings(self):
        node = self.nodeName
        settings = {}
        for attr in self.affecting_attributes:
            settings[attr] = cmds.getAttr(f"{node}.{attr}")
        return settings

    def hasAllValues(self, values: dict, compare_to: dict):
        for key, val1 in values.items():
            if key.startswith("__"):
                continue
            if val1 == compare_to.get(key):
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
            self.setAttribute(path, val)

        # apply the default values if provided
        if "__defaults__" in data:
            for attr, val in data["__defaults__"].items():
                path = f"{node}.{attr}"
                if not cmds.objExists(path):
                    MGlobal.displayWarning(f"Non-existing attribute: '{path}'")
                    continue
                self.setAttribute(path, val)

    def setAttribute(self, path, val):
        cmds.setAttr(path, lock=0)
        if isinstance(val, str):
            cmds.setAttr(path, val, type="string")
            MGlobal.displayInfo(f"Changed '{path}' to '{val}'.")
        else:
            cmds.setAttr(path, val)
            MGlobal.displayInfo(f"Changed '{path}' to '{val}'.")

    def setDefault(self, path, val):
        cmds.setAttr(path, lock=0)
        if isinstance(val, str):
            cmds.addAttr(path, e=1, defaultValue=val)
            MGlobal.displayInfo(f"Set '{path}' default value to '{val}'.")
        else:
            cmds.addAttr(path, e=1, defaultValue=val)
            MGlobal.displayInfo(f"Set '{path}' default value to '{val}'.")

    def replaceControlUI(self):
        self.populateOptions()
