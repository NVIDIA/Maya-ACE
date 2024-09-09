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
import fnmatch
import json
import os
import re

from maya import api, cmds, mel

# requires maya standalone initialized
from maya.api.OpenMaya import MGlobal

NODE_TYPE_PLAYER = "AceAnimationPlayer"
ENV_DEFAULT_URL = "ACE_ANIMATION_CONTROLLER_DEFAULT_URL"
ENV_API_KEY = "NVCF_API_KEY"

AUDIO_CONNECTIONS = [
    # audio attribute, player attribute
    ("filename", "audiofile"),
    ("offset", "audioOffset"),
    ("sourceStart", "audioStart"),
    ("sourceEnd", "audioEnd"),
]


def attach_animation_player_to_blendshape(node=None):
    """Create and connect a AceAnimationPlayer to the blendshape node

    Args:
        node (str): the name of a blendshape node or a shape node with blendshape history

    Returns:
        (str) An AceAnimationPlayer node name
    """
    blendshapes = get_blendshapes_from_node(node)
    player_node = create_animation_player()
    audio_node = get_time_slider_audio()
    if audio_node:
        connect_audio_to_animation_player(audio_node, player_node)

    connect_animation_player_to_blendshape(player_node, blendshapes[0], force=True)

    return player_node


def connect_current_audio_to_animation_player(player_node=None):
    """Connect the current audio node from the timeline control.

    Args:
        player_node (str): An AceAnimationPlayer node

    Returns:
        (str) An Audio node if connected. "" if not applicable.
    """
    if player_node is None:
        selections = cmds.ls(sl=1, type="AceAnimationPlayer")
        if len(selections) < 1:
            MGlobal.displayError("Please select one or more AceAnimationPlayer node")
            return ""

    audio_node = get_time_slider_audio()
    if not audio_node:
        return ""

    for player_node in selections:
        connect_audio_to_animation_player(audio_node, player_node)

    return audio_node


def connect_animation_player_to_blendshape(player_node=None, blendshape_node=None, force=False):
    """Connect an animation player to a blendshape node.

    Args:
        player_node (str): An AceAnimationPlayer node
        blendshape_node (str): An Blendshape node

    Keywords:
        force (bool): Remove existing input connections from blendshape node

    Returns:
        (None)
    """
    if player_node is None and blendshape_node is None:
        selections = cmds.ls(sl=1, long=1)
        if len(selections) != 2:
            MGlobal.displayError("Please select one AceAnimationPlayer node and one Blendshape node.")
            return
        player_node, blendshape_node = selections[0], selections[1]

    if not player_node or cmds.nodeType(player_node) != NODE_TYPE_PLAYER:
        MGlobal.displayError("Please enter one AceAnimationPlayer node and one Blendshape node.")
        return

    blendshape_nodes = get_blendshapes_from_node(blendshape_node)
    if not blendshape_nodes:
        MGlobal.displayError("Please enter one AceAnimationPlayer node and one Blendshape node.")
        return

    blendshape_node = blendshape_nodes[0]
    MGlobal.displayInfo(f"Connecting blendshape weights from {player_node} to {blendshape_node}.")

    in_bs_size = cmds.getAttr(f"{blendshape_node}.weight", size=1)
    if not in_bs_size:
        MGlobal.displayError("No blendshape to connect. Aborting.")
        return

    out_bs_attr = f"{player_node}.outputWeights"
    out_bs_aliases = _get_output_weights_aliases(out_bs_attr)

    def _find_alias_matching(target: str):
        result = ""
        target_alias = cmds.aliasAttr(target, q=1)
        if target_alias:
            result = out_bs_aliases.get(target_alias.lower(), "")
        else:
            result = out_bs_aliases.get(target.lower(), "")
        return result

    # connect blendshape weight attributes
    for i in range(in_bs_size):
        dest_attr = f"{blendshape_node}.weight[{i}]"
        existing = cmds.listConnections(dest_attr, plugs=1)
        if existing:
            if force:
                cmds.disconnectAttr(existing[0], dest_attr)
            else:
                MGlobal.displayWarning(f"Skipping {dest_attr} because of the existing connection to {existing[0]}")
                continue

        src_attr = _find_alias_matching(dest_attr)
        if not src_attr:
            # if outputWeights.size() is not big enough, Maya will automatically increase the size
            src_attr = f"{player_node}.outputWeights[{i}]"

        cmds.connectAttr(src_attr, dest_attr)

    return


def _get_output_weights_aliases(array_attr: str):
    """Returns a map of cleaned and lower-cased aliases for attribute names"""
    aliases = {}
    for i in range(cmds.getAttr(array_attr, size=1)):
        attr_name = f"{array_attr}[{i}]"
        attr_alias = cmds.aliasAttr(attr_name, q=1)
        if not attr_alias:
            continue
        bs_name = re.sub(r"^out_", "", attr_alias)
        aliases[bs_name.lower()] = attr_name
    return aliases


def get_blendshapes_from_node(node=None):
    """Find blendshape nodes from a node history(upstream connections).

    Args:
        node (str): a mesh or a parent transform node

    Returns:
        (list): a list of blendshape nodes
    """
    if node is None:
        nodes = cmds.ls(sl=1, long=1)
    else:
        nodes = cmds.ls(node, long=1)

    blendshapes = cmds.ls(nodes, type="blendShape")
    if blendshapes:
        return blendshapes

    # get shape nodes
    shapes = cmds.ls(nodes, shapes=1, long=1)
    if not shapes:
        shapes = cmds.listRelatives(nodes, shapes=1, noIntermediate=1, fullPath=1)
        if not shapes:
            MGlobal.displayError(f"Please enter at least a shape")
            return []

    # filter or gather blendshape nodes
    blendshapes = cmds.ls(cmds.listHistory(shapes), type="blendShape")
    if len(blendshapes) < 1:
        MGlobal.displayError(f"Please enter a shape with blendshape node(s).")
        return []

    return blendshapes


def connect_audio_to_animation_player(audio_node, player_node):
    """Connect an audio node to an animation player

    Args:
        audio_node (str): An Audio node
        player_node (str): An AceAnimationPlayer node

    Returns:
        None
    """
    for src, dst in AUDIO_CONNECTIONS:
        cur_conns = cmds.listConnections(f"{player_node}.{dst}", plugs=1)
        if cur_conns:
            cmds.disconnectAttr(cur_conns[0], f"{player_node}.{dst}")
        cmds.connectAttr(f"{audio_node}.{src}", f"{player_node}.{dst}")
    return


def disconnect_audio(player_node):
    """Disonnect audio node from an animation player

    Args:
        player_node (str): An AceAnimationPlayer node

    Returns:
        None
    """
    for src, dst in AUDIO_CONNECTIONS:
        cur_conns = cmds.listConnections(f"{player_node}.{dst}", plugs=1)
        if cur_conns:
            cmds.disconnectAttr(cur_conns[0], f"{player_node}.{dst}")
    return


def create_animation_player(name="AceAnimationPlayer1"):
    """Create an animation player with default connections and parameters.

    Keywords:
        name (str): A name of the new node; defaults to "AceAnimationPlayer1"

    Environment Variable Input:
        ACE_ANIMATION_CONTROLLER_DEFAULT_URL: define the default url

    Returns:
        (str) the name of the new AceAnimationPlayer node
    """
    cmds.loadPlugin("maya_aceclient", qt=1)
    node = cmds.createNode(NODE_TYPE_PLAYER, name=name)

    # connect time input
    time_node = cmds.ls(type="time")
    if "time1" in time_node:
        cmds.connectAttr("time1.outTime", f"{node}.time")

    default_url = os.getenv(ENV_DEFAULT_URL)
    if default_url:
        cmds.setAttr(f"{node}.networkAddress", default_url, type="string")

    return node


def import_audio(filepath, set_time_slider=False):
    """Create a new Audio node with the filepath."""
    existings = list(find_audio_node_from_path(filepath))

    if existings:
        node, path = existings[0]  # get the first item
        return node

    ns, _ = os.path.splitext(os.path.basename(filepath))
    nodes = cmds.file(filepath, type="audio", i=1, ignoreVersion=1, renameAll=1, namespace=ns, returnNewNodes=1)
    if not nodes:
        # NOTE: should not happen
        return ""

    if set_time_slider:
        time_slider = set_time_slider_audio(nodes[0])

    return nodes[0]


def find_audio_node_from_path(match_filter="*"):
    """Iterate audio node and file path matching the given path

    Args:
        match_filter (str): a unix-style file path filter

    Yields:
        (str, str) node name and file path
    """
    for node in cmds.ls(type="audio"):
        filepath = cmds.getAttr(f"{node}.filename")
        if fnmatch.fnmatch(filepath, match_filter):
            yield node, filepath
    return


def set_time_slider_audio(audio_node):
    """
    Args:
        audio_node (str): An Audio node

    Returns:
        (str) time slider ui name
    """
    time_slider = _get_time_slider()
    result = cmds.timeControl(time_slider, e=True, sound=audio_node, displaySound=True)
    return result  # time slider name


def get_time_slider_audio():
    """
    Returns:
        (str) audio node name
    """
    time_slider = _get_time_slider()
    return cmds.timeControl(time_slider, q=True, sound=True)


def _get_time_slider():
    time_slider = mel.eval("$tmpVar=$gPlayBackSlider")
    return time_slider


def remove_attr_aliases(node: str):
    """Remove existing attribute aliases"""
    aliases = cmds.aliasAttr(node, q=1)
    if not aliases:
        return

    for i in range(0, len(aliases), 2):
        alias, attr = aliases[i], aliases[i + 1]
        cmds.aliasAttr(f"{node}.{alias}", rm=1)

    return
