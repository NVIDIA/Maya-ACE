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

from ace_tools.config import import_a2f_parameters  # fmt: off
from ace_tools.setup import NODE_TYPE_A2F_PLAYER, NODE_TYPE_ACE_PLAYER, PLAYER_TYPES
from maya import cmds
from maya.api.OpenMaya import MGlobal

from .winutils import center_to_main_window


def import_params(selected_node, file_path):
    try:
        import_a2f_parameters(selected_node, file_path)
    except Exception as e:
        errorMessage = str(e)
        MGlobal.displayError(errorMessage)
        cmds.confirmDialog(
            title="Error",
            message=f"{errorMessage}",
            button=["OK"],
            defaultButton="OK",
            dismissString="OK",
        )
    else:
        MGlobal.displayInfo(f"Successfully imported config from {file_path}")
        cmds.confirmDialog(
            title="Export Success",
            message="Parameters imported successfully.",
            button=["OK"],
            defaultButton="OK",
        )


def browse_file_path(text_field, button):
    data_dir = cmds.workspace(expandName="data")
    if not os.path.exists(data_dir):
        data_dir = cmds.workspace(q=1, rootDirectory=1)

    file_path = cmds.fileDialog2(
        fileMode=1,
        dialogStyle=2,
        caption="Select File",
        fileFilter="YAML & JSON Files (*.yaml *.json);;YAML Files (*.yaml);;JSON Files (*.json);;All Files (*.*)",
        startingDirectory=data_dir,
    )
    if file_path:
        file_path = file_path[0]
        cmds.textField(text_field, edit=True, text=file_path, ann=file_path)
        cmds.button(button, e=True, enable=True)


def display_import_animation_player_params_window():
    """
    Display a window to import the params of a Ace Player node to a config file.

    The combobox will be set to the current selected Ace Player node by default.
    """
    externalMargin = 8
    internalMargin = 4

    window = cmds.window(title="Import A2F/ACE Parameters")
    cmds.columnLayout(
        adjustableColumn=True,
        columnAlign="center",
        columnAttach=("both", externalMargin),
    )
    cmds.separator(height=externalMargin, style="none")

    # File path row layout
    cmds.rowLayout(
        nc=3,
        adjustableColumn=2,
        columnWidth3=(120, 200, 70),
        columnAttach=[(1, "right", 0), (2, "both", 5), (3, "left", 5)],
    )
    cmds.text(label="Import Config File")
    file_path_field = cmds.textField(text="Select a file...", editable=False)
    browse_button = cmds.button(label="Browse")
    cmds.setParent("..")  # Set the layout back to column layout

    # Node selection row layout
    cmds.separator(height=internalMargin, style="none")
    cmds.rowLayout(
        nc=2,
        adjustableColumn=2,
        columnWidth2=(120, 280),
        columnAttach=[(1, "right", 0), (2, "left", 5)],
    )
    cmds.text(label="A2F/ACE Player Node")
    option_menu = cmds.optionMenu()
    populate_nodes(option_menu)
    cmds.setParent("..")  # Set the layout back to column layout

    # Export button
    cmds.separator(height=internalMargin, style="none")
    cmds.rowLayout(
        numberOfColumns=1,
        adjustableColumn=1,
        columnAttach=[(1, "right", 0)],
    )
    import_button = cmds.button(
        label="Import A2F/ACE Parameters",
        enable=False,
        command=lambda x: import_params(
            get_selected_node(option_menu),
            cmds.textField(file_path_field, query=True, text=True),
        ),
    )
    cmds.setParent("..")  # Set the layout back to column layout

    # attach command to the browse button
    cmds.button(
        browse_button,
        e=True,
        command=lambda x: browse_file_path(file_path_field, import_button),
    )

    # Bottom margin
    cmds.separator(height=externalMargin, style="none")

    # Attempt to set the option in option_menu to the selected node
    select_node_from_editor(option_menu, display_errors=False)
    cmds.showWindow(window)
    center_to_main_window(window)


def populate_nodes(option_menu):
    nodes = []
    for node_type in PLAYER_TYPES:
        nodes += cmds.ls(type=node_type) or []

    for node in nodes:
        cmds.menuItem(label=node, parent=option_menu)


def get_selected_node(option_menu):
    return cmds.optionMenu(option_menu, query=True, value=True)


def select_node_from_editor(option_menu, display_errors=True):
    selected_nodes = []
    for node_type in PLAYER_TYPES:
        selected_nodes += cmds.ls(selection=True, type=node_type) or []

    if selected_nodes:
        # Check if the selected node exists in the option menu
        menu_items = cmds.optionMenu(option_menu, query=True, itemListLong=True) or []
        for selection in selected_nodes:
            if selection in [cmds.menuItem(item, query=True, label=True) for item in menu_items]:
                cmds.optionMenu(option_menu, edit=True, value=selection)
                break
    else:
        if display_errors:
            message = "No Ace/A2F Animation Player is selected."
            cmds.confirmDialog(
                title="Message",
                message=message,
                button=["OK"],
                defaultButton="OK",
            )
    pass
