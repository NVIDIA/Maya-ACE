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
from maya import cmds
from maya.api.OpenMaya import MGlobal

from ace_tools import ace_import_config_parameters


def import_params(selected_node, file_path):
    try:
        ace_import_config_parameters(selected_node, file_path)
    except Exception as e:
        errorMessage = str(e)
        MGlobal.displayError(errorMessage)
        cmds.confirmDialog(
            title="Error", message=f"{errorMessage}", button=["OK"], defaultButton="OK", dismissString="OK"
        )
    else:
        MGlobal.displayInfo(f"Successfully imported config from {file_path}")
        cmds.confirmDialog(
            title="Export Success", message="Parameters imported successfully.", button=["OK"], defaultButton="OK"
        )


def browse_file_path(text_field, button):
    file_path = cmds.fileDialog2(
        fileMode=1, dialogStyle=2, caption="Select File", fileFilter="JSON Files (*.json);;All Files (*.*)"
    )
    if file_path:
        file_path = file_path[0]
        cmds.textField(text_field, edit=True, text=file_path, ann=file_path)
        cmds.button(button, e=True, enable=True)


def populate_nodes(option_menu):
    nodes = cmds.ls(type="AceAnimationPlayer")
    for node in nodes:
        cmds.menuItem(label=node, parent=option_menu)


def get_selected_node(option_menu):
    return cmds.optionMenu(option_menu, query=True, value=True)


def select_node_from_editor(option_menu, display_errors=True):
    selected_nodes = cmds.ls(selection=True, type="AceAnimationPlayer")
    if selected_nodes:
        cmds.optionMenu(option_menu, edit=True, value=selected_nodes[0])
    else:
        if display_errors:
            message = "No AceAnimationPlayer is selected."
            cmds.confirmDialog(title="Message", message=message, button=["OK"], defaultButton="OK")


def display_import_animation_player_params_window():
    """
    Display a window to import the params of a Ace Player node to json file.

    The combobox will be set to the current selected Ace Player node by default.
    """
    externalMargin = 8
    internalMargin = 4

    window = cmds.window(title="Import Ace Player Parameters")
    cmds.columnLayout(adjustableColumn=True, columnAlign="center", columnAttach=("both", externalMargin))
    cmds.separator(height=externalMargin, style="none")

    # File path row layout
    cmds.rowLayout(
        nc=3,
        adjustableColumn=2,
        columnWidth3=(120, 200, 70),
        columnAttach=[(1, "right", 0), (2, "both", 5), (3, "left", 5)],
    )
    cmds.text(label="Import JSON File")
    file_path_field = cmds.textField(text="Select a file...", editable=False)
    browse_button = cmds.button(label="Browse")
    cmds.setParent("..")  # Set the layout back to column layout

    # Node selection row layout
    cmds.separator(height=internalMargin, style="none")
    cmds.rowLayout(nc=2, adjustableColumn=2, columnWidth2=(120, 280), columnAttach=[(1, "right", 0), (2, "left", 5)])
    cmds.text(label="AcePlayer Node")
    option_menu = cmds.optionMenu()
    populate_nodes(option_menu)
    cmds.setParent("..")  # Set the layout back to column layout

    # Export button
    cmds.separator(height=internalMargin, style="none")
    cmds.rowLayout(numberOfColumns=1, adjustableColumn=1, columnAttach=[(1, "right", 0)])
    import_button = cmds.button(
        label="Import Ace Player Parameters",
        enable=False,
        command=lambda x: import_params(
            get_selected_node(option_menu), cmds.textField(file_path_field, query=True, text=True)
        ),
    )
    cmds.setParent("..")  # Set the layout back to column layout

    # attach command to the browse button
    cmds.button(browse_button, e=True, command=lambda x: browse_file_path(file_path_field, import_button))

    # Bottom margin
    cmds.separator(height=externalMargin, style="none")

    # Attempt to set the option in option_menu to the selected node
    select_node_from_editor(option_menu, display_errors=False)
    cmds.showWindow(window)
