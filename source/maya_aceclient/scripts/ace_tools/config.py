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
import json
import os
import re

from maya import api, cmds, mel

# requires maya standalone initialized
from maya.api.OpenMaya import MGlobal

from .names import ARKIT_FACE_EXPRESSIONS, EMOTION_NAMES
from .setup import NODE_TYPE_PLAYER

FACE_PARAM_MAPPING = {
    # attribute name: config key
    "lowerFaceSmoothing": "lower_face_smoothing",
    "upperFaceSmoothing": "upper_face_smoothing",
    "lowerFaceStrength": "lower_face_strength",
    "upperFaceStrength": "upper_face_strength",
    "faceMaskLevel": "face_mask_level",
    "faceMaskSoftness": "face_mask_softness",
    "skinStrength": "skin_strength",
    "eyelidOpenOffset": "eyelid_offset",
    "lipOpenOffset": "lip_close_offset",
}
FACE_PARAM_MAPPING_REV = {v: k for k, v in FACE_PARAM_MAPPING.items()}
EMOTION_PARAM_MAPPING = {
    "emotionStrength": "emotion_strength",
    "emotionContrast": "emotion_contrast",
    "maxEmotion": "max_emotions",
    "liveBlendCoef": "live_blend_coef",
    "enablePreferredEmotion": "enable_preferred_emotion",
    "preferredEmotionStrength": "preferred_emotion_strength",
}
EMOTION_PARAM_MAPPING_REV = {v: k for k, v in EMOTION_PARAM_MAPPING.items()}
AUDIO_CONNECTIONS = [
    # audio attribute, player attribute
    ("filename", "audiofile"),
    ("offset", "audioOffset"),
    ("sourceStart", "audioStart"),
    ("sourceEnd", "audioEnd"),
]


def ace_import_config_parameters(node, file_path):
    """
    Args:
        node (str): an AceAnimationPlayer node path
        file_path (str): a path to the exported json file.
    """
    if not file_path:
        raise Exception("Please specify a path for the imported JSON file.")
    if not node:
        raise Exception("Please specify a target ace player node to export the params.")

    if not os.path.exists(file_path):
        raise Exception(f"{file_path} doesn't exists")

    with open(file_path, "r") as json_file:
        json_data = json.load(json_file)

    # validations
    emotion_names = json_data.get("emotion_names") or json_data.get("preferred_emotion_names") or []
    emotion_weights = json_data.get("preferred_emotion") or []
    if len(emotion_names) != len(emotion_weights):
        msg = (
            "The number of elements in preferred_emotion_names and preferred_emotion must match."
            " Please ensure that both lists have the same length."
        )
        raise Exception(msg)

    bs_names = json_data.get("blendshape_names") or []
    bs_multipliers = json_data.get("blendshape_params", {}).get("bsWeightMultipliers") or []
    bs_offsets = json_data.get("blendshape_params", {}).get("bsWeightOffsets") or []
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

    # set configs
    for jsonKey, val in json_data.get("face_params", {}).items():
        cmds.setAttr(f"{node}.{FACE_PARAM_MAPPING_REV[jsonKey]}", val)

    for jsonKey, val in json_data.get("emotion_params", {}).items():
        cmds.setAttr(f"{node}.{EMOTION_PARAM_MAPPING_REV[jsonKey]}", val)

    for attr, val in zip(emotion_names, emotion_weights):
        cmds.setAttr(f"{node}.{attr}", val)

    for expression, val in zip(bs_names, bs_multipliers):
        cmds.setAttr(f"{node}.multiply_{expression}", val)

    for expression, val in zip(bs_names, bs_offsets):
        cmds.setAttr(f"{node}.offset_{expression}", val)

    return "import successfully"


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

    # read from json template
    current_directory = os.path.dirname(os.path.realpath(__file__))

    path_a2f_config = os.path.join(current_directory, "a2f_ms_config_template.json")
    with open(path_a2f_config, "r") as a2f_template:
        template_a2f_config = json.load(a2f_template)

    path_a2e_config = os.path.join(current_directory, "a2e_ms_config_template.json")
    with open(path_a2e_config, "r") as a2e_template:
        template_a2e_config = json.load(a2e_template)

    # read values from the node
    json_data = get_config_parameters(selected_node)

    # patch a2f params
    template_a2f_config["face_params"].update(json_data.get("face_params", {}))
    template_a2f_config["face_params"]["emotion"] = json_data.get("preferred_emotion", [])
    template_a2f_config["blendshape_params"].update(json_data.get("blendshape_params", {}))

    # patch a2e params
    template_a2e_config["post_processing_config"].update(json_data.get("emotion_params", {}))

    base, ext = os.path.splitext(file_path)
    # save to file
    with open(f"{base}{ext}", "w") as json_file:
        json.dump(json_data, json_file, indent=2)

    # # save server side config
    # # save a2f config
    # with open(f"{base}.a2f_config{ext}", "w") as a2f_config:
    #     json.dump(template_a2f_config, a2f_config, indent=2)

    # # save a2e config
    # with open(f"{base}.a2e_config{ext}", "w") as a2e_config:
    #     json.dump(template_a2e_config, a2e_config, indent=2)

    return json_data


def get_config_parameters(node):
    """Collect parameters for the a2f config

    Args:
        node (str): an AceAnimationPlayer node path

    Returns:
        (dict) controlled parameters in the a2f config format
    """
    json_data = {}

    # face params
    face_params = {}
    for attr, key in FACE_PARAM_MAPPING.items():
        face_params[key] = cmds.getAttr(f"{node}.{attr}")
    json_data["face_params"] = face_params

    # emotion vector
    # NOTE: currently, mix-used with preferred emotion for a2e
    emotion = []
    for attr in EMOTION_NAMES:
        emotion.append(cmds.getAttr(f"{node}.{attr}"))
    json_data["preferred_emotion"] = emotion
    json_data["emotion_names"] = EMOTION_NAMES

    # blendshape weight multipliers and offsets
    bs_params = {
        "bsWeightMultipliers": [1.0] * len(ARKIT_FACE_EXPRESSIONS),
        "bsWeightOffsets": [0.0] * len(ARKIT_FACE_EXPRESSIONS),
    }
    for i, expression in enumerate(ARKIT_FACE_EXPRESSIONS):
        bs_params["bsWeightMultipliers"][i] = cmds.getAttr(f"{node}.multiply_{expression}")
        bs_params["bsWeightOffsets"][i] = cmds.getAttr(f"{node}.offset_{expression}")
    json_data["blendshape_params"] = bs_params
    json_data["blendshape_names"] = ARKIT_FACE_EXPRESSIONS

    # emotion params
    emotion_params = {}
    for attr, key in EMOTION_PARAM_MAPPING.items():
        emotion_params[key] = cmds.getAttr(f"{node}.{attr}")
    json_data["emotion_params"] = emotion_params

    return json_data
