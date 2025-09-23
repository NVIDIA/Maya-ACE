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
import fnmatch
import json
import os
import re
from typing import Union

from maya import api, cmds, mel

# requires maya standalone initialized
from maya.api.OpenMaya import MGlobal

from .audio import (
    get_time_slider_audio,
)
from .names import A2F_EMOTION_NAMES, A2F_TONGUE_POSES, ARKIT_FACE_EXPRESSIONS
from .utils import (
    create_animation_players_set,
    create_original_mesh,
    filter_mesh_shapes,
    get_blendshapes_from_node,
    get_original_mesh,
)

NODE_TYPE_ACE_PLAYER = "AceAnimationPlayer"
NODE_TYPE_A2F_PLAYER = "A2FAnimationPlayer"
PLAYER_TYPES = (NODE_TYPE_ACE_PLAYER, NODE_TYPE_A2F_PLAYER)
ENV_DEFAULT_URL = "ACE_ANIMATION_CONTROLLER_DEFAULT_URL"
ENV_API_KEY = "NVCF_API_KEY"
AUDIO_CONNECTIONS = [
    # audio attribute, player attribute
    ("filename", "audiofile"),
    ("offset", "audioOffset"),
    ("sourceStart", "audioStart"),
    ("sourceEnd", "audioEnd"),
]


def attach_ace_player_to_blendshapes(
    blendshapes: Union[str, list] = None,
    set_name: str = "AnimationPlayersSet",
) -> str:
    """Create and connect a AceAnimationPlayer to the blendshape node

    Args:
        blendshapes (str or list): one or multiple blendshape nodes

    Returns:
        (str) An AceAnimationPlayer node name

    Raises:
        RuntimeError: when the input node is not a blendshape node nor connected to a blendshape
    """
    blendshapes = get_blendshapes_from_node(blendshapes)
    if not blendshapes:
        raise RuntimeError(
            f"Requires a blendshape or a mesh with a blendshape. (given: {blendshapes})"
        )

    player_node = create_ace_animation_player(set_name=set_name)
    # force initialize output aliases for ace player
    # TODO: get this list from server when it is available
    initialize_output_aliases(player_node)

    audio_node = get_time_slider_audio()
    if audio_node:
        connect_audio_to_animation_player(audio_node, player_node)

    connect_animation_player_to_blendshapes(player_node, blendshapes, force=True)

    return player_node


def attach_a2f_player_to_blendshapes(
    blendshapes: Union[str, list] = None,
    set_name: str = "AnimationPlayersSet",
) -> str:
    """Create and connect a A2FAnimationPlayer to the blendshape node

    Args:
        blendshapes (str or list): one or multiple blendshape nodes

    Returns:
        (str) An A2FAnimationPlayer node name

    Raises:
        RuntimeError: when the input node is not a blendshape node nor connected to a blendshape
    """
    blendshapes = get_blendshapes_from_node(blendshapes)
    if not blendshapes:
        raise RuntimeError(
            f"Requires a blendshape or a mesh with a blendshape. (given: {blendshapes})"
        )

    player_node = create_a2f_animation_player(set_name=set_name)
    # initialize_output_aliases(player_node)  # NOTE: not required for local a2x

    audio_node = get_time_slider_audio()
    if audio_node:
        connect_audio_to_animation_player(audio_node, player_node)

    connect_animation_player_to_blendshapes(player_node, blendshapes, force=True)

    return player_node


def attach_a2f_player_to_mesh(
    skin_mesh: str = None,
    tongue_mesh: str = None,
    set_name: str = "AnimationPlayersSet",
) -> str:
    """Create and connect a A2FAnimationPlayer to a mesh node

    Args:
        skin_mesh (str): a mesh node for skin geometry
        tongue_mesh (str): (Optional) a mesh node for tongue geometry
        set_name (str): (Optional) a set name to add the player to

    Returns:
        (str) An A2FAnimationPlayer node name

    Raises:
        RuntimeError: when there's no input mesh nor a selected mesh
    """
    selections = cmds.ls(sl=1, long=1) or []
    if not skin_mesh:
        # get the selected nodes
        if len(selections) < 1:
            MGlobal.displayError("Please select a mesh for skin geometry.")
            return
        skin_mesh = selections[0]

    if not tongue_mesh and len(selections) > 1:
        # get the second selected node if available
        tongue_mesh = selections[1]

    skin_meshes = filter_mesh_shapes(skin_mesh)
    if not skin_meshes or len(skin_meshes) < 1:
        raise RuntimeError(f"Requires one mesh. (given: {skin_mesh})")

    # tongue mesh is optional
    tongue_meshes = filter_mesh_shapes(tongue_mesh)
    if not tongue_meshes or len(tongue_meshes) < 1:
        MGlobal.displayInfo(f"No tongue mesh is given. skipping connect to tongue.")

    skin = skin_meshes[0] if skin_meshes else None
    tongue = tongue_meshes[0] if tongue_meshes else None

    return _attach_a2f_player_to_mesh(skin, tongue, set_name=set_name)


def _attach_a2f_player_to_mesh(
    skin_mesh: str = None,
    tongue_mesh: str = None,
    set_name: str = "AnimationPlayersSet",
) -> str:
    player_node = create_a2f_animation_player(set_name=set_name)

    audio_node = get_time_slider_audio()
    if audio_node:
        connect_audio_to_animation_player(audio_node, player_node)

    _connect_a2f_player_to_mesh(player_node, skin_mesh, to_tongue=False)
    if tongue_mesh:
        _connect_a2f_player_to_mesh(player_node, tongue_mesh, to_tongue=True)

    return player_node


def connect_a2f_player_to_mesh(
    player_node: str = None,
    mesh_node: str = None,
    to_tongue: bool = False,
) -> None:
    """Connect an animation player to a mesh node

    Args:
        player_node (str): the name of an A2FAnimationPlayer node
        mesh_node (str): the name of a mesh node
        to_tongue (bool): if True, connect to the tongue mesh

    Returns:
        (None)

    Raises:
        RuntimeError: when the input node is not a mesh node
    """
    if not player_node and not mesh_node:
        # get from selections
        mesh_selections = cmds.ls(sl=1, long=1, type="mesh") or []
        mesh_selections += cmds.ls(sl=1, long=1, type="transform", shapes=1) or []
        player_selections = cmds.ls(sl=1, long=1, type=NODE_TYPE_A2F_PLAYER) or []

        if len(mesh_selections) != 1 or len(player_selections) != 1:
            MGlobal.displayError("Please select a A2FAnimationPlayer and a mesh.")
            return

        player_node = player_selections[0]
        mesh_node = mesh_selections[0]

    if not player_node or cmds.nodeType(player_node) != NODE_TYPE_A2F_PLAYER:
        raise RuntimeError(f"Requires one A2FAnimationPlayer. (given: {player_node})")

    meshes = filter_mesh_shapes(mesh_node)
    if not meshes or len(meshes) != 1:
        raise RuntimeError(f"Requires one mesh. (given: {mesh_node})")

    return _connect_a2f_player_to_mesh(player_node, meshes[0], to_tongue)


def _connect_a2f_player_to_mesh(
    player_node: str = None,
    mesh_node: str = None,
    to_tongue: bool = False,
) -> None:
    orig_mesh_in = "faceGeometryOrig"
    mesh_out = "faceGeometry"
    if to_tongue:
        orig_mesh_in = "tongueGeometryOrig"
        mesh_out = "tongueGeometry"

    # prepare or find an original mesh
    player_orig_in_connections = (
        cmds.listConnections(f"{player_node}.{orig_mesh_in}", plugs=1, source=True) or []
    )
    if not player_orig_in_connections:
        orig_mesh_out = create_original_mesh(mesh_node)  # 'meshShapeOrig.outMesh'
        # connect the original mesh to the player
        cmds.connectAttr(orig_mesh_out, f"{player_node}.{orig_mesh_in}")
    else:
        orig_mesh_out = player_orig_in_connections[0]

    # disconnect existing inMesh connections
    mesh_in_connections = cmds.listConnections(f"{mesh_node}.inMesh", plugs=1) or []
    for src_to_mesh in mesh_in_connections:
        # disconnect existing inMesh connections and the original mesh
        cmds.disconnectAttr(src_to_mesh, f"{mesh_node}.inMesh")

    # connect the player to the mesh
    cmds.connectAttr(f"{player_node}.{mesh_out}", f"{mesh_node}.inMesh")
    return


def connect_current_audio_to_animation_players(
    player_nodes: Union[str, list] = None,
) -> str:
    """Connect the current audio node from the timeline control.

    Args:
        player_nodes (str or list):
            a name or a list of names of Ace/A2F AnimationPlayer nodes
            if not provided, the selected nodes will be used

    Returns:
        (str) An Audio node if connected. "" if not applicable.
    """
    selections = []
    if player_nodes is None:
        # get the selected node
        for node_type in PLAYER_TYPES:
            selections += cmds.ls(sl=1, type=node_type) or []
    else:
        for node_type in PLAYER_TYPES:
            selections += cmds.ls(player_nodes, type=node_type) or []

    if len(selections) != 1:
        MGlobal.displayError("Please select or enter at least one Ace or A2F AnimationPlayer node")
        return ""

    audio_node = get_time_slider_audio()
    if not audio_node:
        return ""

    for player_node in selections:
        connect_audio_to_animation_player(audio_node, player_node)

    return audio_node


def connect_animation_player_to_blendshapes(
    player_node: str = None,
    blendshape_nodes: Union[str, list] = None,
    force: bool = False,
) -> None:
    """Connect an animation player to a blendshape node.

    Args:
        player_node (str): An Ace or Local AnimationPlayer node
        blendshape_nodes (str or list): one or multiple blendshape nodes

    Keywords:
        force (bool): Remove existing input connections from blendshape node

    Returns:
        (None)
    """
    if player_node is None and blendshape_nodes is None:
        selections = cmds.ls(sl=1, long=1)
        if len(selections) < 2:
            MGlobal.displayError(
                "Please select one Ace or Local AnimationPlayer node and Blendshape nodes."
            )
            return
        player_node, blendshape_nodes = selections[0], selections[1:]

    if not player_node or cmds.nodeType(player_node) not in PLAYER_TYPES:
        MGlobal.displayError(
            "Please enter one Ace or Local AnimationPlayer node and Blendshape nodes."
        )
        return

    blendshape_nodes = get_blendshapes_from_node(blendshape_nodes)
    if not blendshape_nodes:
        MGlobal.displayError(
            "Please enter one Ace or Local AnimationPlayer node and Blendshape nodes."
        )
        return

    for blendshape_node in blendshape_nodes:
        MGlobal.displayInfo(
            f"Connecting blendshape weights from {player_node} to {blendshape_node}."
        )
        connected = _connect_animation_player_to_blendshapes(player_node, blendshape_node, force)
        if not connected:
            MGlobal.displayError(f"No blendshape is connected to '{blendshape_node}'")

    return


def _connect_animation_player_to_blendshapes(
    player_node: str,
    blendshape_node: str,
    force: bool = False,
) -> list:
    """Connect an animation player to a blendshape node

    Args:
        player_node (str): An Ace or Local AnimationPlayer node
        blendshape_node (str): A blendshape node
        force (bool): If True, remove existing input connections from blendshape node

    Returns:
        (list) A list of connected attributes
    """
    connected = []
    in_bs_size = cmds.getAttr(f"{blendshape_node}.weight", size=1)
    if not in_bs_size:
        return []

    out_bs_attr = f"{player_node}.outputWeights"
    out_bs_aliases = _get_output_weights_aliases(out_bs_attr)

    # connect blendshape weight attributes
    for i in range(in_bs_size):
        dest_attr = f"{blendshape_node}.weight[{i}]"
        existing = cmds.listConnections(dest_attr, plugs=1)
        if existing:
            if force:
                cmds.disconnectAttr(existing[0], dest_attr)
            else:
                MGlobal.displayWarning(
                    f"Skipping {dest_attr} because of the existing connection to {existing[0]}"
                )
                continue

        src_attr = _find_matching_alias(dest_attr, aliases=out_bs_aliases)
        if not src_attr:
            # if outputWeights.size() is not big enough, Maya will automatically increase the size
            src_attr = f"{player_node}.outputWeights[{i}]"

        cmds.connectAttr(src_attr, dest_attr)
        connected.append(dest_attr)

    return connected


def _find_matching_alias(target: str, aliases: dict):
    """Find a matching alias for an attribute

    Args:
        target (str): An attribute name
        aliases (dict): A map of cleaned and lower-cased aliases for attribute names

    Returns:
        (str) A matching alias for the attribute
    """
    result = ""
    target_alias = cmds.aliasAttr(target, q=1)
    if target_alias:
        result = aliases.get(target_alias.lower(), "")
    else:
        result = aliases.get(target.lower(), "")
    return result


def _get_output_weights_aliases(array_attr: str):
    """Returns a map of cleaned and lower-cased aliases for attribute names

    Args:
        array_attr (str): An array attribute

    Returns:
        (dict) A map of cleaned and lower-cased aliases for attribute names
    """
    aliases = {}
    for i in range(cmds.getAttr(array_attr, size=1)):
        attr_name = f"{array_attr}[{i}]"
        attr_alias = cmds.aliasAttr(attr_name, q=1)
        if not attr_alias:
            continue
        bs_name = re.sub(r"^out(_[a-zA-Z]+)?_", "", attr_alias)
        aliases[bs_name.lower()] = attr_name
    return aliases


def connect_audio_to_animation_player(audio_node, player_node):
    """Connect an audio node to an animation player

    Args:
        audio_node (str): An Audio node
        player_node (str): An AceAnimationPlayer or a A2FAnimationPlayer node

    Returns:
        None
    """
    for src, dst in AUDIO_CONNECTIONS:
        cur_conns = cmds.listConnections(
            f"{player_node}.{dst}", plugs=1, source=True, destination=False
        )
        if cur_conns:
            cmds.disconnectAttr(cur_conns[0], f"{player_node}.{dst}")
        cmds.connectAttr(f"{audio_node}.{src}", f"{player_node}.{dst}")
    return


def connect_audio_to_all_players(audio_node: str) -> list:
    """Connect an audio node to all animation player nodes

    Args:
        audio_node (str): An Audio node

    Returns:
        (list) A list of updated player nodes
    """
    all_player_nodes = []
    for nodetype in PLAYER_TYPES:
        all_player_nodes += cmds.ls(type=nodetype)

    updated = []
    for player_node in all_player_nodes:
        try:
            connect_audio_to_animation_player(audio_node, player_node)
            if cmds.objExists(f"{player_node}.triggerLoad"):
                cmds.dgeval(f"{player_node}.triggerLoad")
            updated.append(player_node)
        except Exception as e:
            MGlobal.displayWarning(f"Failed to connect audio to '{player_node}': {e}")

    if updated:
        MGlobal.displayInfo(f"Connected '{audio_node}' to {len(updated)} other player node(s).")

    return updated


def disconnect_audio(player_node: str) -> None:
    """Disonnect audio node from an animation player

    Args:
        player_node (str): An AceAnimationPlayer or a A2FAnimationPlayer node

    Returns:
        None
    """
    for src, dst in AUDIO_CONNECTIONS:
        cur_conns = cmds.listConnections(f"{player_node}.{dst}", plugs=1)
        if cur_conns:
            cmds.disconnectAttr(cur_conns[0], f"{player_node}.{dst}")
    return


def create_ace_animation_player(
    name: str = "AceAnimationPlayer1",
    set_name: str = "AnimationPlayersSet",
) -> str:
    """Create an animation player with default connections and parameters.

    Keywords:
        name (str): A name of the new node; defaults to "AceAnimationPlayer1"

    Environment Variable Input:
        ACE_ANIMATION_CONTROLLER_DEFAULT_URL: define the default url

    Returns:
        (str) the name of the new AceAnimationPlayer node
    """
    cmds.loadPlugin("maya_ace", qt=1)
    node = cmds.createNode(NODE_TYPE_ACE_PLAYER, name=name)

    # connect time input
    time_node = cmds.ls(type="time")
    if "time1" in time_node:
        cmds.connectAttr("time1.outTime", f"{node}.time")

    default_url = os.getenv(ENV_DEFAULT_URL)
    if default_url:
        cmds.setAttr(f"{node}.networkAddress", default_url, type="string")

    new_set = create_animation_players_set(node, set_name)
    if new_set != set_name:
        print(f'Could not add "{node}" to a set, "{set_name}", created a new one, "{new_set}"')
    else:
        print(f'Added "{node}" to the set, "{set_name}"')

    return node


def create_a2f_animation_player(
    name: str = "A2FAnimationPlayer1",
    set_name: str = "AnimationPlayersSet",
) -> str:
    """Create a local animation player with default connections and parameters.

    Keywords:
        name (str): A name of the new node; defaults to "A2FAnimationPlayer1"

    Returns:
        (str) the name of the new A2FAnimationPlayer node
    """
    cmds.loadPlugin("maya_ace", qt=1)
    node = cmds.createNode(NODE_TYPE_A2F_PLAYER, name=name)

    # connect time input
    time_node = cmds.ls(type="time")
    if "time1" in time_node:
        cmds.connectAttr("time1.outTime", f"{node}.time")

    new_set = create_animation_players_set(node, set_name)
    if new_set != set_name:
        print(f'Could not add "{node}" to a set, "{set_name}", created a new one, "{new_set}"')
    else:
        print(f'Added "{node}" to the set, "{set_name}"')

    return node


def initialize_output_aliases(node: str) -> None:
    """Set default aliases for an AceAnimationPlayer node

    NOTE: This function assumes the default arkit blendshapes are used for the output.
        Arkit is the only option at the moment, but it can be changed in the future.
    """
    existing_pairs = cmds.aliasAttr(node, q=1) or ()  # [alias, attr, alias, attr, ...]

    if cmds.objExists(f"{node}.outputWeights"):
        for i, name in enumerate(ARKIT_FACE_EXPRESSIONS):
            alias = f"out_{name}"
            attr = f"{node}.outputWeights[{i}]"
            if alias in existing_pairs:
                cmds.aliasAttr(f"node.{alias}", remove=1)
            cmds.aliasAttr(alias, attr)
            cmds.getAttr(attr)  # resize array by pulling value

        for i, name in enumerate(A2F_TONGUE_POSES, start=len(ARKIT_FACE_EXPRESSIONS)):
            alias = f"out_{name}"
            attr = f"{node}.outputWeights[{i}]"
            if alias in existing_pairs:
                cmds.aliasAttr(f"node.{alias}", remove=1)
            cmds.aliasAttr(alias, attr)
            cmds.getAttr(attr)  # resize array by pulling value

    if cmds.objExists(f"{node}.outputEmotions"):
        for i, name in enumerate(A2F_EMOTION_NAMES):
            alias = f"out_{name}"
            attr = f"{node}.outputEmotions[{i}]"
            if alias in existing_pairs:
                cmds.aliasAttr(f"node.{alias}", remove=1)
            cmds.aliasAttr(alias, attr)
            cmds.getAttr(attr)  # resize array by pulling value

    return
