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
import json
import os
from types import MappingProxyType
from typing import Union

from maya import cmds

# requires maya standalone initialized
from maya.api.OpenMaya import MGlobal

from .names import A2F_EMOTION_NAMES, ARKIT_FACE_EXPRESSIONS
from .parameters import (
    A2E_PARAM_MAPPING,
    A2F_PARAM_MAPPING,
    EMOTION_PARAM_MAPPING,
    FACE_PARAM_MAPPING,
)
from .setup import AUDIO_CONNECTIONS, NODE_TYPE_ACE_PLAYER
from .utils import cache_array_aliases


def export_a2f_parameters(node: str, file_path: str, capitalize_array_names: bool = False):
    """Export parameters from an A2F/ACE player node to a json/yaml file.

    Args:
        node (str): an AceAnimationPlayer node path
        file_path (str): a path to the exported json file.
        capitalize_array_names (bool): if True, capitalize the blendshape names. for ACE compatibility.

    Returns:
        (dict) a dictionary of the config data
    """
    if not node:
        raise Exception("Please specify a target ace player node to export the params.")
    if not file_path or not os.access(os.path.dirname(file_path), os.W_OK):
        raise Exception("Please specify a path to export the config file.")

    # read values from the node
    config_data = {}

    # a2e params
    a2e_params = {
        key: data
        for key, data in _collect_config_parameters(node, A2E_PARAM_MAPPING, capitalize_array_names)
    }
    config_data["a2e"] = a2e_params

    # a2f params
    a2f_params = {
        key: data
        for key, data in _collect_config_parameters(node, A2F_PARAM_MAPPING, capitalize_array_names)
    }
    config_data["a2f"] = a2f_params

    try:
        import yaml
    except ImportError:
        yaml = None

    base, ext = os.path.splitext(file_path)
    if yaml and ("yaml" in ext.lower() or "yml" in ext.lower()):  # YAML is preferred
        with open(f"{base}{ext}", "w") as yaml_file:
            yaml.dump(config_data, yaml_file, indent=2)
    else:
        with open(f"{base}{ext}", "w") as json_file:
            json.dump(config_data, json_file, indent=2)

    return config_data


def import_a2f_parameters(node: str, file_path: str):
    """Reads and update attributes of an A2F/ACE player node from a json/yaml file.

    Args:
        node (str): an AceAnimationPlayer node path
        file_path (str): a path to the exported json file.

    Returns:
        (str) a message indicating the success or failure of the operation

    Notes:
        The (new) config file should be in the following format:
            "a2e": {
                ...
            },
            "a2f": {
                "blendshape_params": {
                    "weight_multipliers": {
                    ...
                    },
                    "weight_offsets": {
                        ...
                    }
                },
                "enable_tongue_blendshapes": true,
                "face_params": {
                    ...
                },
            }
        The old config file is in the following format:
            "face_params": {
                ...
            },
            "preferred_emotion": [
                ...
            ],
            "emotion_params": {
                ...
            },
            "blendshape_params": {
                ...
            },
            ...
    """
    if not node:
        raise Exception("Please specify a target ace player node to export the params.")

    if not file_path or not os.access(file_path, os.R_OK):
        raise Exception("Please specify a path to import the config file.")

    # NOTE: yaml or json file can be loaded.
    config_data = _read_config_file(file_path)

    if "a2e" in config_data or "a2f" in config_data:
        # new config file
        blendshape_aliases = {
            "faceMultipliers": cache_array_aliases(node, "faceMultipliers"),
            "faceOffsets": cache_array_aliases(node, "faceOffsets"),
            "tongueMultipliers": cache_array_aliases(node, "tongueMultipliers"),
            "tongueOffsets": cache_array_aliases(node, "tongueOffsets"),
        }
        for attr, val in _populate_config_attributes(config_data.get("a2f", {}), A2F_PARAM_MAPPING):
            _set_config_attributes(node, attr, val, blendshape_aliases)

        emotion_aliases = {
            "preferredEmotions": cache_array_aliases(node, "preferredEmotions"),
        }
        for attr, val in _populate_config_attributes(config_data.get("a2e", {}), A2E_PARAM_MAPPING):
            _set_config_attributes(node, attr, val, emotion_aliases)
    else:
        # old config file
        return ace_import_config_parameters(node, file_path)


def _collect_config_parameters(node: str, mapping: dict, capitalize_array_names: bool = False):
    """Get parameters from an A2F/ACE player node.

    Args:
        node (str): an AceAnimationPlayer node path
        mapping (dict): a dictionary of the mapping from the config data to the attribute names
        capitalize_blendshape_names (bool):
            if True, capitalize the blendshape names. for ACE compatibility.
                - e.g. "blinkLeft" -> "BlinkLeft"

    Yields:
        (str, Any) a tuple of the config key and the value or a nested dict
    """
    for key, attributes in mapping.items():
        if isinstance(attributes, str):
            if cmds.objExists(f"{node}.{attributes}"):
                yield key, cmds.getAttr(f"{node}.{attributes}")
            else:
                MGlobal.displayInfo(f"Attribute {attributes} does not exist on node {node}")
        elif isinstance(attributes, (list, tuple)):
            # array attributes
            array_params = {}
            for attr in attributes:
                if not cmds.objExists(f"{node}.{attr}"):
                    MGlobal.displayInfo(f"Attribute {attr} does not exist on node {node}")
                array_params.update(_get_array_parameter_values(node, attr, capitalize_array_names))
            yield key, array_params
        elif isinstance(attributes, (dict, MappingProxyType)):
            # get the nested parameters - recursively call _collect_config_parameters
            sub_params = {
                sub_key: sub_data
                for sub_key, sub_data in _collect_config_parameters(
                    node, attributes, capitalize_array_names
                )
            }
            yield key, sub_params

    return  # terminate generator


def _get_array_parameter_values(node: str, attr: str, capitalize_array_names: bool = False):
    """Get parameters from an array attribute.

    Args:
        node (str): an AceAnimationPlayer node path
        attr (str): an array attribute name
        capitalize_array_names (bool):
            if True, capitalize the blendshape names. for ACE compatibility.
                - e.g. "blinkLeft" -> "BlinkLeft"

    Yields:
        (str, Any) a tuple of the config key(array label without prefix) and the value
    """
    aliases = cache_array_aliases(node, attr)
    for alias_key, (alias, aliased_attr) in aliases.items():
        name = alias.split("_", 1)[-1]  # e.g. "faceMultiplier_Blink_X" -> "Blink_X"
        label = name
        if capitalize_array_names:
            # capitalize the first letter; e.g. "blinkLeft" -> "BlinkLeft"
            label = name[0].upper() + name[1:]
        yield label, cmds.getAttr(f"{node}.{aliased_attr}")
    return  # terminate generator


def _set_config_attributes(
    node: str, attr: str, data: Union[float, int, bool, dict], cached_aliases: dict
):
    """Set attributes from a config value.
        This function deals with array attributes with aliases and prefixes.
            e.g. "BlinkLeft" -> "faceMultiplier_BlinkLeft" -> "faceMultipliers[0]"

    Args:
        node (str): an AceAnimationPlayer node path
        attr (str): an attribute name
        data (float | int | bool | dict): a value to set
        cached_aliases (dict): a dictionary of cached aliases
    """
    if isinstance(data, dict):
        # array attributes with aliases and prefixes. deal this very specially.
        for name, val in data.items():
            alias_and_attr = cached_aliases[attr].get(name.lower())
            if not alias_and_attr:
                continue
            alias, aliased_attr = alias_and_attr
            _set_attr(node, alias, val)
    elif cmds.objExists(f"{node}.{attr}"):
        _set_attr(node, attr, data)
    else:
        MGlobal.displayInfo(f"Attribute {attr} does not exist on node {node}")


def _set_attr(node: str, attr: str, val: Union[float, int, bool]):
    if not attr or not cmds.objExists(f"{node}.{attr}"):
        return
    cmds.setAttr(f"{node}.{attr}", val)
    if val in (True, False):
        val = int(val)
    MGlobal.displayInfo(f'setAttr "{node}.{attr}" {val};')


def _populate_config_attributes(config_data: dict, mapping: dict = A2F_PARAM_MAPPING):
    """Populates attributes recursively from a config data.

    The mapping is a dictionary of a config data key to an attribute name.
    The mapping can be nested (e.g. face_params).
    The mapping can be an array (e.g. preferredEmotions).
    The mapping can be a float, int, or bool (e.g. liveBlendCoef).

    This function skips config data if the keys are not in the mapping.

    Args:
        config_data (dict): a dictionary of the config data
        mapping (dict): a dictionary of the mapping from the config data to the attribute names

    Yields:
        (str, Any) a tuple of the attribute name and the value
    """
    for key, value in config_data.items():
        mapping_value = mapping.get(key)  # str, dict, or list
        if not mapping_value:
            continue

        if isinstance(mapping_value, (dict, MappingProxyType)) and isinstance(value, dict):
            # the value is a nested dictionary. goes recursively.
            #   e.g. "face_params": { "eye_params": { "eye_size": 1.0 } }
            for sub_key, sub_value in _populate_config_attributes(value, mapping_value):
                yield f"{sub_key}", sub_value
        elif isinstance(mapping_value, (list, tuple)):
            # if the mapping value is a list or tuple, the attribute is an array (e.g. preferredEmotions)
            for attr_name in mapping_value:
                yield f"{attr_name}", value  # value will be a dictionary. pop this multiple times.
        elif isinstance(mapping_value, str):
            yield f"{mapping_value}", value  # value is float, int, or bool
        # else: skip the config data

    return  # terminate generator


def _read_config_file(file_path):
    """Reads a json or yaml file and returns a dictionary.

    Args:
        file_path (str): a path to the exported json file.

    Returns:
        (dict) a dictionary of the config data
    """
    config_data = None
    try:
        import yaml
    except ImportError:
        print("Warning: PyYAML is not installed. Only JSON config files can be loaded.")
        try:
            with open(file_path, "r") as json_file:
                config_data = json.load(json_file)
        except json.JSONDecodeError:
            raise Exception("The file is not a valid JSON file.")
    else:
        # NOTE: PyYAML can read json files.
        try:
            with open(file_path, "r") as yaml_file:
                config_data = yaml.load(yaml_file, Loader=yaml.FullLoader)
        except yaml.YAMLError:
            raise Exception("The file is not a valid YAML file.")

    return config_data


def ace_import_config_parameters(node, file_path):
    """Import parameters from an old config file. (deprecated)

    Args:
        node (str): an AceAnimationPlayer node path
        file_path (str): a path to the exported json file.

    Returns:
        (dict) a dictionary of the config data
    """
    if not file_path:
        raise Exception("Please specify a path for the imported JSON file.")
    if not node:
        raise Exception("Please specify a target ace player node to export the params.")

    if not os.path.exists(file_path):
        raise Exception(f"{file_path} doesn't exists")

    with open(file_path, "r") as json_file:
        config_data = json.load(json_file)

    # validations
    emotion_names = (
        config_data.get("emotion_names") or config_data.get("preferred_emotion_names") or []
    )
    emotion_weights = config_data.get("preferred_emotion") or []
    if len(emotion_names) != len(emotion_weights):
        msg = (
            "The number of elements in preferred_emotion_names and preferred_emotion must match."
            " Please ensure that both lists have the same length."
        )
        raise Exception(msg)

    bs_names = config_data.get("blendshape_names") or []
    bs_multipliers = config_data.get("blendshape_params", {}).get("bsWeightMultipliers") or []
    bs_offsets = config_data.get("blendshape_params", {}).get("bsWeightOffsets") or []
    if len(bs_names) != len(bs_multipliers):
        msg = (
            "The number of elements in blendshape_names and blendshape_params.bsWeightMultipliers must match."
            " Please ensure that both lists have the same length."
        )
        raise Exception(msg)
    if len(bs_names) != len(bs_offsets):
        msg = (
            "The number of elements in blendshape_names and blendshape_params.bsWeightOffsets must match."
            " Please ensure that both lists have the same length."
        )
        raise Exception(msg)

    # reverse mapping
    FACE_PARAM_MAPPING_REV = {v: k for k, v in FACE_PARAM_MAPPING.items()}
    EMOTION_PARAM_MAPPING_REV = {v: k for k, v in EMOTION_PARAM_MAPPING.items()}

    # set configs
    for jsonKey, val in config_data.get("face_params", {}).items():
        attr = FACE_PARAM_MAPPING_REV.get(jsonKey)
        _set_attr(node, attr, val)

    for jsonKey, val in config_data.get("emotion_params", {}).items():
        attr = EMOTION_PARAM_MAPPING_REV.get(jsonKey)
        _set_attr(node, attr, val)

    for i, val in enumerate(emotion_weights):
        attr = f"preferredEmotions[{i}]"
        _set_attr(node, attr, val)

    for expression, val in zip(bs_names, bs_multipliers):
        attr = f"faceMultiplier_{expression}"
        if not cmds.objExists(f"{node}.{attr}"):
            # deal with a common error case
            camel_case = expression[0].lower() + expression[1:]
            attr2 = f"faceMultiplier_{camel_case}"
            if cmds.objExists(f"{node}.{attr2}"):
                attr = attr2
            else:
                continue
        _set_attr(node, attr, val)

    for expression, val in zip(bs_names, bs_offsets):
        attr = f"faceOffset_{expression}"
        if not cmds.objExists(f"{node}.{attr}"):
            # deal with a common error case
            camel_case = expression[0].lower() + expression[1:]
            attr2 = f"faceOffset_{camel_case}"
            if cmds.objExists(f"{node}.{attr2}"):
                attr = attr2
            else:
                continue
        _set_attr(node, attr, val)

    return config_data


def ace_export_config_parameters(selected_node, file_path):
    """
    This function exports the AceAnimationPlayer configs to a json file which can be reimport again.
    """
    if not file_path:
        MGlobal.displayError("Please specify a path for the exported JSON file.")
        return
    if not selected_node:
        MGlobal.displayError("Please specify a target ace player node to export the params.")
        return

    # read values from the node
    config_data = get_config_parameters(selected_node)

    base, ext = os.path.splitext(file_path)
    # save to file
    with open(f"{base}{ext}", "w") as json_file:
        json.dump(config_data, json_file, indent=2)

    return config_data


def get_config_parameters(node):
    """Collect parameters for the a2f config

    Args:
        node (str): an AceAnimationPlayer node path

    Returns:
        (dict) controlled parameters in the a2f config format
    """
    config_data = {}

    # face params
    face_params = {}
    for attr, key in FACE_PARAM_MAPPING.items():
        if cmds.objExists(f"{node}.{attr}"):
            face_params[key] = cmds.getAttr(f"{node}.{attr}")
        else:
            MGlobal.displayError(f"Attribute {attr} does not exist on node {node}")
    config_data["face_params"] = face_params

    # emotion vector
    # NOTE: currently, mix-used with preferred emotion for a2e
    if cmds.objExists(f"{node}.preferredEmotions"):
        emotion = cmds.getAttr(f"{node}.preferredEmotions")
        config_data["preferred_emotion"] = emotion[0] if emotion else []
    else:
        MGlobal.displayError(f"Attribute preferredEmotions does not exist on node {node}")
    config_data["emotion_names"] = A2F_EMOTION_NAMES

    # blendshape weight multipliers and offsets
    bs_params = {
        "bsWeightMultipliers": [1.0] * len(ARKIT_FACE_EXPRESSIONS),
        "bsWeightOffsets": [0.0] * len(ARKIT_FACE_EXPRESSIONS),
    }
    for i, expression in enumerate(ARKIT_FACE_EXPRESSIONS):
        if cmds.objExists(f"{node}.faceMultiplier_{expression}"):
            bs_params["bsWeightMultipliers"][i] = cmds.getAttr(
                f"{node}.faceMultiplier_{expression}"
            )
        else:
            MGlobal.displayError(
                f"Attribute faceMultiplier_{expression} does not exist on node {node}"
            )
        if cmds.objExists(f"{node}.faceOffset_{expression}"):
            bs_params["bsWeightOffsets"][i] = cmds.getAttr(f"{node}.faceOffset_{expression}")
        else:
            MGlobal.displayError(f"Attribute faceOffset_{expression} does not exist on node {node}")
    config_data["blendshape_params"] = bs_params
    config_data["blendshape_names"] = ARKIT_FACE_EXPRESSIONS

    # emotion params
    emotion_params = {}
    for attr, key in EMOTION_PARAM_MAPPING.items():
        if cmds.objExists(f"{node}.{attr}"):
            emotion_params[key] = cmds.getAttr(f"{node}.{attr}")
        else:
            MGlobal.displayError(f"Attribute {attr} does not exist on node {node}")
    config_data["emotion_params"] = emotion_params

    return config_data
