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
import os
from typing import Union

from maya import cmds, mel

# requires maya standalone initialized
from maya.api.OpenMaya import (
    MFnDependencyNode,
    MFnNumericAttribute,
    MGlobal,
    MObject,
    MPlug,
    MSelectionList,
)


def create_original_mesh(mesh_selection: str):
    """Create an original mesh shape for deformer input.

    Args:
        mesh_selection (str): a mesh node name

    Returns:
        (str) a new mesh node's outMesh attribute
    """
    mesh_shapes = filter_mesh_shapes(mesh_selection)
    if not mesh_shapes:
        return ""

    # CAUTION: this command may connect worldMesh to inMesh, while returning outMesh
    orig_mesh_out = cmds.deformableShape(mesh_shapes[0], createOriginalGeometry=1)

    return orig_mesh_out[0]


def get_original_mesh(mesh_selection: str):
    """Create an original mesh shape for deformer input.

    Args:
        mesh_selection (str): a mesh node name

    Returns:
        (str) an original mesh's outMesh attribute or ['']
    """
    mesh_shapes = filter_mesh_shapes(mesh_selection)
    if not mesh_shapes:
        return ""

    orig_mesh_out = cmds.deformableShape(mesh_shapes[0], originalGeometry=1)

    if not orig_mesh_out:
        return ""

    return orig_mesh_out[0]


def get_blendshapes_from_node(nodes: Union[str, list] = None):
    """Find blendshape nodes from upstream connections.

    Args:
        nodes (str or list): shape(s) or parent transform node(s)

    Returns:
        (list): a list of blendshape nodes
    """
    # clean up input. make a list of node paths, or get from selection
    if nodes is None:
        # get user selection
        nodes = cmds.ls(sl=1, long=1)
    else:
        nodes = cmds.ls(nodes, long=1)

    blendshapes = cmds.ls(nodes, type="blendShape")
    if blendshapes:
        return blendshapes

    # get shape nodes
    shapes = cmds.ls(nodes, shapes=1, long=1)
    if not shapes:
        shapes = cmds.listRelatives(nodes, shapes=1, noIntermediate=1, fullPath=1)
        if not shapes:
            MGlobal.displayError(f"Cannot find any shapes from the given: {nodes}")
            return []

    # filter or gather blendshape nodes
    blendshapes = cmds.ls(cmds.listHistory(shapes), type="blendShape")
    if not blendshapes:  # or len(blendshapes) < 1
        return []

    return blendshapes


def filter_mesh_shapes(nodes: Union[str, list]):
    """Filter mesh nodes from a nodes list.

    Args:
        nodes (str or list): a mesh node or a parent transform node

    Returns:
        (list): a list of mesh shape nodes
    """
    # conform str/list to a list of long names
    nodes = cmds.ls(nodes, long=1)

    meshes = cmds.ls(nodes, type="mesh")
    if meshes:
        return meshes

    meshes = cmds.ls(
        cmds.listRelatives(nodes, shapes=1, noIntermediate=1, fullPath=1),
        type="mesh",
    )

    return meshes


def get_meshes_from_history(nodes: Union[str, list]):
    """Find mesh nodes from a node history(upstream connections).

    Args:
        nodes (str or list): shape(s) or parent transform node(s)

    Returns:
        (list): a list of mesh shape nodes
    """
    # get shape nodes
    shapes = cmds.ls(nodes, shapes=1, long=1)
    if not shapes:
        # get meshes from children nodes
        shapes = cmds.listRelatives(nodes, shapes=1, noIntermediate=1, fullPath=1)
        if not shapes:
            MGlobal.displayError(f"Please enter at least a shape")
            return []

    # filter or gather mesh nodes
    meshes = cmds.ls(cmds.listHistory(shapes), type="mesh")

    return meshes or []


def create_animation_players_set(node: str, name: str = "AnimationPlayersSet"):
    """Create a new animation player set."""

    # create a set for the player node
    existings = cmds.ls(name)
    new_set = name
    if existings:
        try:
            cmds.sets(node, add=f"{existings[0]}")
        except RuntimeError:
            # the one is not a set
            new_set = cmds.sets(node, n=f"{name}")
    else:
        new_set = cmds.sets(node, n=f"{name}")
    return new_set


def remove_attr_aliases(node: str, prefix: str = "", force: bool = False):
    """Remove existing attribute aliases"""
    aliases = cmds.aliasAttr(node, q=1)
    if not aliases:
        return

    for i in range(0, len(aliases), 2):
        alias, attr = aliases[i], aliases[i + 1]
        if not force and not cmds.objExists(f"{node}.{alias}"):
            continue
        if prefix and not alias.startswith(prefix):
            continue
        cmds.aliasAttr(f"{node}.{alias}", rm=1)

    return


def list_attr_aliases(node: str, attr: str = ""):
    """List existing attribute aliases

    Args:
        node (str): a node name
        attr (str): an attribute name. if not provided, all aliases will be listed

    Yields:
        (str, str): an alias and an attribute name
    """
    aliases = cmds.aliasAttr(node, q=1)
    if not aliases:
        return

    for i in range(0, len(aliases), 2):
        alias, attr_name = aliases[i], aliases[i + 1]
        if not attr:
            yield alias, attr_name
        elif attr_name == attr:
            yield alias, attr_name
        elif attr_name.startswith(f"{attr}["):
            # array attribute elements
            yield alias, attr_name
    return


def cache_array_aliases(node: str, array_attribute: str):
    """Cache aliases for an array attribute.
        key is the name without prefix of each array attribute.

    Args:
        node (str): an AceAnimationPlayer node path
        array_attribute (str): an array attribute name

    Returns:
        (dict) a dictionary of lower case name to (alias, aliased_attr)
    """
    aliases = {}

    for alias, aliased_attr in list_attr_aliases(node, array_attribute):
        # Remove the prefix. There's an assumption that the prefix is the first part of the alias.
        name = alias.split("_", 1)[-1]  # e.g. "faceMultiplier_Blink_X" -> "Blink_X"
        # Save keys as lower case. we compare them case insensitively to maximize the compatibility.
        #    e.g. leftMouth == LeftMouth == leftmouth != left_mouth
        # NOTE: Warn users that blendshape names should not duplicate at lower case.
        aliases[name.lower()] = (alias, aliased_attr)

    return aliases


def get_attribute_object(node_and_attr: Union[str, tuple]):
    """Get the MObject for an attribute.

    Args:
        node_and_attr (str or tuple): Either a string "node.attribute" or tuple (node, attribute_name)

    Returns:
        (MObject, MObject) the attribute object and the node object

    Examples:
        get_attribute_object("myNode.myAttribute")
        get_attribute_object(("myNode", "myAttribute"))
    """
    # Parse input to get node and attribute names
    if isinstance(node_and_attr, str):
        if "." not in node_and_attr:
            MGlobal.displayError(f"Expected 'node.attribute' format, got '{node_and_attr}'")
            return None, None
        node_name, attr_name = node_and_attr.split(".", 1)
    elif isinstance(node_and_attr, (tuple, list)) and len(node_and_attr) == 2:
        node_name, attr_name = node_and_attr
    else:
        MGlobal.displayError(f"Invalid input type: {type(node_and_attr)}")
        return None, None

    # Get the MObject for the node
    sel = MSelectionList()
    sel.add(node_name)
    node_obj = sel.getDependNode(0)

    # Get the attribute MObject
    fn_node = MFnDependencyNode(node_obj)
    attr_obj = fn_node.attribute(attr_name)

    return attr_obj, node_obj
