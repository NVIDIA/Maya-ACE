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

from maya import cmds
from maya.api.OpenMaya import MGlobal

EXPORT_PLAYER_DISCLAIMER = """This export may not fully reflect recent updates from the server or ACE services.

The following limitations may result in potential discrepancies:
- Parameters not set and controlled by the Maya plugin will be exported with default values.
- Default values in the exported configuration files may differ if the A2F server is deployed with custom settings.
- To ensure consistency between the client and server, carefully compare the server's configuration with the exported parameters.
- For the most accurate and up-to-date information, please refer to the official website or the server configuration."""

EXPORT_SERVICE_DISCLAIMER = """This export may not fully reflect recent updates from the server or ACE services.

The following limitations may result in potential discrepancies:
- Exported files are provided by the A2F service, suitable for service deployment.
- The format and content vary depending on the service and the service version.
- Note that some services may not support this export."""


class AceExportConfigWindow:
    """Display a window to export the params of a Ace Player node to json file.

    The combobox will be set to the current selected Ace Player node by default.
    """

    export_consent_text = EXPORT_PLAYER_DISCLAIMER

    def __init__(self, title="Export Ace Player Parameters"):
        self._title = title
        self._window = ""

        self._field_path = ""
        self._btn_export = ""
        self._btn_browse = ""
        self._option_node = ""

    @classmethod
    def pop_window(cls):
        win = cls()
        win.build()
        win.show()

    def show(self):
        cmds.showWindow(self._window)

    def _on_export(self, *args):
        export_params(self.get_selected_node(), self.get_file_path())

    def get_selected_node(self):
        return cmds.optionMenu(self._option_node, query=True, value=True)

    def get_file_path(self):
        return cmds.textField(self._field_path, query=True, text=True)

    def browse_file_path(self, *args):
        file_path = cmds.fileDialog2(
            fileMode=0,
            dialogStyle=2,
            caption="Select a File",
            fileFilter="JSON Files (*.json);;All Files (*.*)",
        )
        if not file_path:
            return
        file_path = file_path[0]
        cmds.textField(self._field_path, edit=True, text=file_path, ann=file_path)
        cmds.button(self._btn_export, e=True, enable=True)
        return file_path

    def build(self):
        if not self._window:
            self._build_layout()
            populate_anim_player_nodes(self._option_node)
            # browse button
            cmds.button(
                self._btn_browse,
                e=True,
                command=self.browse_file_path,
            )
            # Export button
            cmds.button(
                self._btn_export,
                edit=True,
                command=self._on_export,
            )
        # Attempt to set the option in option_menu to the selected node
        select_node_from_editor(self._option_node, display_errors=False)

    def _build_layout(self):
        """
        Display a window to export the params of a Ace Player node to json file.

        The combobox will be set to the current selected Ace Player node by default.
        """
        externalMargin = 8
        internalMargin = 4

        self._window = cmds.window(title=self._title)
        cmds.columnLayout(
            adjustableColumn=True,
            columnAlign="center",
            columnAttach=("both", externalMargin)
        )
        cmds.separator(height=externalMargin, style="none")

        # File path row layout
        cmds.rowLayout(
            nc=3,
            adjustableColumn=2,
            columnWidth3=(120, 200, 70),
            columnAttach=[(1, "right", 0), (2, "both", 5), (3, "left", 5)],
        )
        cmds.text(label="Location")
        self._field_path = cmds.textField(
            text="Select a location...", editable=False)
        self._btn_browse = cmds.button(label="Browse")
        cmds.setParent("..")  # Set the layout back to column layout

        # Node selection row layout
        cmds.separator(height=internalMargin, style="none")
        cmds.rowLayout(
            nc=2,
            adjustableColumn=2,
            columnWidth2=(120, 280),
            columnAttach=[(1, "right", 0), (2, "left", 5)]
        )
        cmds.text(label="AcePlayer Node")
        self._option_node = cmds.optionMenu()
        cmds.setParent("..")  # Set the layout back to column layout

        # Disclaimer
        link = """<a href="https://docs.nvidia.com/ace/latest/index.html">NVIDIA ACE</a>"""
        cmds.separator(height=internalMargin, style="none")
        cmds.rowLayout(
            "row_heading",
            numberOfColumns=1,
            adjustableColumn=1,
            columnAttach=[(1, "right", 0)]
        )
        note_heading = cmds.text(
            l="DISCLAIMER",
        )
        cmds.setParent("..")  # Set the layout back to column layout
        cmds.rowLayout(
            "row_note",
            numberOfColumns=1,
            adjustableColumn=1,
            columnAttach=[(1, "right", 0)]
        )
        note_text = cmds.text(
            l=self.export_consent_text,
            align="left",
        )
        cmds.setParent("..")  # Set the layout back to column layout
        cmds.rowLayout(
            "row_link",
            numberOfColumns=1,
            adjustableColumn=1,
            columnAttach=[(1, "right", 0)]
        )
        link_text = cmds.text(
            l=link,
            hl=1,
        )
        cmds.setParent("..")  # Set the layout back to column layout

        # Export button
        cmds.separator(height=internalMargin, style="none")
        cmds.rowLayout(
            numberOfColumns=1,
            adjustableColumn=1,
            columnAttach=[(1, "right", 0)]
        )
        self._btn_export = cmds.button(
            label="Export File(s)",
            enable=False,
        )
        cmds.setParent("..")  # Set the layout back to column layout

        # Bottom margin
        cmds.separator(height=externalMargin, style="none")
        cmds.setParent("..")  # Set the layout back to column layout


class AceExportServiceConfigWindow(AceExportConfigWindow):

    export_consent_text = EXPORT_SERVICE_DISCLAIMER

    def __init__(self, title="Export Ace Service Configs"):
        super().__init__(title)

    def _on_export(self, *args):
        export_service_configs(self.get_selected_node(), self.get_file_path())

    def browse_file_path(self, *args):
        file_path = cmds.fileDialog2(
            fileMode=3,
            dialogStyle=2,
            caption="Select a Directory",
            fileFilter="All Files (*.*)",
        )
        if not file_path:
            return
        file_path = file_path[0]
        cmds.textField(self._field_path, edit=True, text=file_path, ann=file_path)
        cmds.button(self._btn_export, e=True, enable=True)
        return file_path


def export_service_configs(node, filepath, config_type="JSON"):
    file_prefix = filepath
    if os.path.isdir(filepath) and filepath[-1] not in ("/", "\\"):
        file_prefix += '/'

    try:
        client_type = cmds.getAttr(f"{node}.clientType")
        if client_type not in (0, ):
            raise Exception("The service does not provide config export.")

        address = cmds.getAttr(f"{node}.networkAddress")
        apikey = cmds.getAttr(f"{node}.apiKey")
        functionid = cmds.getAttr(f"{node}.functionId")
        result = cmds.AceExportServiceConfig(
            filePrefix=file_prefix,
            configType=config_type,
            address=address,
            apiKey=apikey,
            functionId=functionid,
        )
    except Exception as exc:
        MGlobal.displayError(f'{exc}')
        if not (cmds.about(batch=1)):
            cmds.confirmDialog(
                title="Error",
                message=f'{exc}',
                button=["OK"],
                defaultButton="OK",
                dismissString="OK"
            )
    else:
        MGlobal.displayInfo(f"Successfully exported files to {file_prefix}")
        MGlobal.displayInfo("\n".join(result))
        if not (cmds.about(batch=1)):
            cmds.confirmDialog(
                title="Export Success",
                message="\n".join([f'{w:30s}' for w in result]),
                button=["OK"],
                defaultButton="OK"
            )
    pass


def export_params(selected_node, file_path):
    try:
        from ace_tools.config import ace_export_config_parameters
        ace_export_config_parameters(selected_node, file_path)
    except Exception as e:
        errorMessage = str(e)
        MGlobal.displayError(errorMessage)
        if not (cmds.about(batch=1)):
            cmds.confirmDialog(
                title="Error",
                message=f"{errorMessage}",
                button=["OK"],
                defaultButton="OK",
                dismissString="OK"
            )
    MGlobal.displayInfo(f"Successfully exported config to {file_path}")
    if not (cmds.about(batch=1)):
        cmds.confirmDialog(
            title="Export Success",
            message="Parameters exported successfully.",
            button=["OK"],
            defaultButton="OK"
        )


def populate_anim_player_nodes(option_menu):
    nodes = cmds.ls(type="AceAnimationPlayer")
    for node in nodes:
        cmds.menuItem(label=node, parent=option_menu)


def select_node_from_editor(option_menu, display_errors=True):
    selected_nodes = cmds.ls(selection=True, type="AceAnimationPlayer")
    if selected_nodes:
        cmds.optionMenu(option_menu, edit=True, value=selected_nodes[0])
    else:
        if display_errors:
            message = "No AceAnimationPlayer is selected."
            cmds.confirmDialog(
                title="Message",
                message=message,
                button=["OK"],
                defaultButton="OK"
            )
