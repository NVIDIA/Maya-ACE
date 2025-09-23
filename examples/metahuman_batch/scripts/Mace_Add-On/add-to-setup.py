# SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
"""

Add the python code below into the Maya Ace setup.py script file below the import statements.

This script can be found where the Maya Ace is installed and then navigate to the following path:
/mace/scripts/ace_tools/setup.py


"""

# ------------------------------------------------------------------------------------------
# add-on section to setup.py file for target rig Mapping (MetaHuman Facial Rig in this case)
# ------------------------------------------------------------------------------------------


from .ACE2Rig_Functions import *
from .Batch_List_MayaDialog import Create_Batch_Dialog


def attach_ace_player_to_MH(node=None):
    """
    Attaches an animation player to a Metahuman (MH) facial rignode.

    Args:
        node (str, optional): The name of the node. Defaults to None.

    Returns:
        str: The name of the created player node.
    """

    # Get the currently selected objects
    selected = cmds.ls(selection=True, long=True)

    # Check if any objects are selected
    if not selected:
        MGlobal.displayError("Please select a MetaHuman Facial Rig.")
        return
    else:
        # Get the full name of the first selected object
        MH_NameSplit = selected[0].split("|")[1].split(":")

        # Extract the namespace from the selected object's name
        if len(MH_NameSplit) > 1:
            MH_namespace = MH_NameSplit[0]
            # Verify that the selected object is a Metahuman Facial Rig
            if not cmds.objExists(f"{MH_namespace}:CTRL_faceGUIShape"):
                MGlobal.displayError("Please select a MetaHuman Facial Rig.")
                return
        else:
            MH_namespace = ""
            # Verify that the selected object is a Metahuman Facial Rig (no namespace)
            if not cmds.objExists(f"CTRL_faceGUIShape"):
                MGlobal.displayError("Please select a MetaHuman Facial Rig.")
                return

    # Create an animation player node and set its outputs to ARKit blend shape names
    # Create a selection set with the ACE player node
    player_node = create_ace_animation_player(set_name="Maya_ACE_Player")
    initialize_output_aliases(player_node)

    # Connect the current audio to the ACE player node
    audio_node = get_time_slider_audio()
    if audio_node:
        connect_audio_to_animation_player(audio_node, player_node)

    # Create an ACE2MH node to map between ACE and Metahuman controls
    ACE2MH_node = create_ACE2Rig_node(MH_namespace)

    # Connect the nodes: ACE2MH node to player node and Metahuman controls
    connect_ACE2Rig_to_player_and_Rig(ACE2MH_node, player_node, MH_namespace)

    # Show the ACE interface in the attribute editor
    cmds.select(player_node)
    cmds.AttributeEditor()

    return player_node


def attach_a2f_player_to_MH(node=None):
    """Create and connect a A2FAnimationPlayer to Metahuman Facial Rig

    Args:
        node (str, optional): The name of the node. Defaults to None.

    Returns:
        (str) An A2FAnimationPlayer node name

    Raises:
        RuntimeError: when the input node is not a Metahuman Facial Rig
    """

    # Get the currently selected objects
    selected = cmds.ls(selection=True, long=True)

    # Check if any objects are selected
    if not selected:
        MGlobal.displayError("Please select a MetaHuman Facial Rig.")
        return
    else:
        # Get the full name of the first selected object
        MH_NameSplit = selected[0].split("|")[1].split(":")

        # Extract the namespace from the selected object's name
        if len(MH_NameSplit) > 1:
            MH_namespace = MH_NameSplit[0]
            # Verify that the selected object is a Metahuman Facial Rig
            if not cmds.objExists(f"{MH_namespace}:CTRL_faceGUIShape"):
                MGlobal.displayError("Please select a MetaHuman Facial Rig.")
                return
        else:
            MH_namespace = ""
            # Verify that the selected object is a Metahuman Facial Rig (no namespace)
            if not cmds.objExists(f"CTRL_faceGUIShape"):
                MGlobal.displayError("Please select a MetaHuman Facial Rig.")
                return

    player_node = create_a2f_animation_player(set_name="Maya_A2F_Player")
    initialize_output_aliases(player_node)

    ACE2MH_node = create_ACE2Rig_node(MH_namespace)

    audio_node = get_time_slider_audio()
    if audio_node:
        connect_audio_to_animation_player(audio_node, player_node)

    connect_ACE2Rig_to_player_and_Rig(ACE2MH_node, player_node, MH_namespace)

    # Show the A2F interface in the attribute editor
    cmds.select(player_node)
    cmds.AttributeEditor()

    return player_node


def connect_ACE2Rig_to_player_and_Rig(ACE2Rig_node, player_node, Rig_namespace):
    """
    Connects the ACE2Rig node to the player node and rig controls.

    Args:
        ACE2Rig_node (str): The name of the ACE2Rig node.
        player_node (str): The name of the player node.
        Rig_namespace (str): The namespace for the rig controls.
    """

    # Connect the ACE2Rig node to the player node
    out_bs_attr = f"{player_node}.outputWeights"
    out_bs_aliases = get_blendshapes_output_indices_dict(out_bs_attr)

    # Connect each output from the player node to the corresponding input on the ACE2rig node
    for key, value in out_bs_aliases.items():
        src_attr = value
        dest_attr = f"{ACE2Rig_node}.{key}"
        cmds.connectAttr(src_attr, dest_attr)

    # Get the base names of the Rig controls
    Rig_Controls_BaseNames = target_rig_controls()

    # If a namespace is provided, add it to the control names
    if Rig_namespace != "":
        Rig_Controls = [f"{Rig_namespace}:{a}" for a in Rig_Controls_BaseNames]
    else:
        Rig_Controls = [f"{a}" for a in Rig_Controls_BaseNames]

    # Get the output names for the ACE2Rig node (replacing '.' with '_')
    ACE2Rig_Outputs = [a.replace(".", "_") for a in Rig_Controls_BaseNames]

    # Connect each output from the ACE2rig node to the corresponding rig control
    for i in range(len(Rig_Controls)):
        src_attr = f"{ACE2Rig_node}.{ACE2Rig_Outputs[i]}"
        dest_attr = f"{Rig_Controls[i]}"
        cmds.connectAttr(src_attr, dest_attr)

    return


def get_blendshapes_output_indices_dict(out_bs_attr):
    """
    Returns a dictionary mapping blendshape output names (lowercased)
    to their corresponding output attribute index on the player node.
    The key is the blendshape name (as used in the rig), and the value is the attribute string for the indexed output, e.g., "playerNode.outputWeights[0]".
    This is used to connect the outputs of the player node to the ACE2Rig node.

    Args:
        out_bs_attr (str): The name of the output blendshape parent attribute.

    Returns:
        dict: A dictionary mapping blendshape output names to their corresponding output attribute indices under the parent attribute.
    """

    # all_outputs = ARKIT_FACE_EXPRESSIONS + A2F_TONGUE_POSES
    out_bs_aliases = {}
    for index, output in enumerate(blendshapes_from_player()):
        out_bs_aliases[output.lower()] = f"{out_bs_attr}[{index}]"
    return out_bs_aliases


def open_batch_dialog(node=None):
    """
    Opens a batch dialog.

    Args:
        node (str, optional): The name of the node. Defaults to None.
    """

    # Create a batch dialog
    BatchUIDialog = Create_Batch_Dialog()

    return


# -------------------------------------
# End of Add-on section for Rig Mapping
# -------------------------------------
