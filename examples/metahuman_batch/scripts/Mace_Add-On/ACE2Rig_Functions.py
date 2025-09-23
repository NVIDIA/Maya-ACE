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

# functions to connect Maya ACE and a target Rig
# example target rig: MetaHuman Rig

import json
import os

import maya.cmds as cmds


def create_selection_set(node: str, SetName: str) -> None:
    """
    Creates a selection set with the given node.

    Args:
        node (str): The name of the node.
        SetName (str): The name of the selection set.
    """

    # Create a selection set with the specified node and name
    set_name = cmds.sets(node, name=SetName)


def create_ACE2Rig_node(Rig_namespace: str) -> str:
    """
    Creates an ACE2Rig node and sets up its inputs, outputs, and expressions.

    Args:
        Rig_namespace (str): The namespace for the Rig.

    Returns:
        str: The name of the created node.
    """

    # Create a new network node with the name 'ACE2Rig'
    node = cmds.createNode("network", name="ACE2Rig")

    print(node)  # Print the name of the created node for debugging purposes

    # Create input attributes for the node all lowercase for better matching
    BS_Inputs = [bs.lower() for bs in blendshapes_from_player()]

    for attr in BS_Inputs:
        # Add a float attribute with the same name as the input
        cmds.addAttr(node, longName=attr, niceName=attr, attributeType="float", defaultValue=0.0)

    # Create output attributes for the node
    BS_Outputs = target_rig_controls()
    for attr in BS_Outputs:
        # Add a float attribute with the same name as the output, replacing '.' with '_'
        cmds.addAttr(
            node,
            longName=attr.replace(".", "_"),
            niceName=attr.replace(".", "_"),
            attributeType="float",
            defaultValue=0.0,
            readable=True,
            writable=True,
        )

    # Create an expression node to map ARKit blend shapes to FaceBuilder
    expression = create_expressions(node)
    cmds.expression(name="ACE2Rig_mapping", string=expression)

    return node


def blendshapes_from_player():

    # blend shapes setup in order of the ACE/A2F player
    # ARKit blend shapes

    blend_shapes = [
        "EyeBlinkLeft",
        "EyeLookDownLeft",
        "EyeLookInLeft",
        "EyeLookOutLeft",
        "EyeLookUpLeft",
        "EyeSquintLeft",
        "EyeWideLeft",
        "EyeBlinkRight",
        "EyeLookDownRight",
        "EyeLookInRight",
        "EyeLookOutRight",
        "EyeLookUpRight",
        "EyeSquintRight",
        "EyeWideRight",
        "JawForward",
        "JawLeft",
        "JawRight",
        "JawOpen",
        "MouthClose",
        "MouthFunnel",
        "MouthPucker",
        "MouthLeft",
        "MouthRight",
        "MouthSmileLeft",
        "MouthSmileRight",
        "MouthFrownLeft",
        "MouthFrownRight",
        "MouthDimpleLeft",
        "MouthDimpleRight",
        "MouthStretchLeft",
        "MouthStretchRight",
        "MouthRollLower",
        "MouthRollUpper",
        "MouthShrugLower",
        "MouthShrugUpper",
        "MouthPressLeft",
        "MouthPressRight",
        "MouthLowerDownLeft",
        "MouthLowerDownRight",
        "MouthUpperUpLeft",
        "MouthUpperUpRight",
        "BrowDownLeft",
        "BrowDownRight",
        "BrowInnerUp",
        "BrowOuterUpLeft",
        "BrowOuterUpRight",
        "CheekPuff",
        "CheekSquintLeft",
        "CheekSquintRight",
        "NoseSneerLeft",
        "NoseSneerRight",
        "TongueOut",
        "TongueTipUp",
        "TongueTipDown",
        "TongueTipLeft",
        "TongueTipRight",
        "TongueRollUp",
        "TongueRollDown",
        "TongueRollLeft",
        "TongueRollRight",
        "TongueUp",
        "TongueDown",
        "TongueLeft",
        "TongueRight",
        "TongueIn",
        "TongueStretch",
        "TongueWide",
        "TongueNarrow",
    ]

    return blend_shapes


def target_rig_controls():

    # controls to map to on the target rig
    # Metahuman controls

    controls = [
        "CTRL_C_jaw.translateX",
        "CTRL_C_jaw.translateY",
        "CTRL_C_mouth.translateX",
        "CTRL_C_mouth.translateY",
        "CTRL_R_mouth_upperLipRaise.translateY",
        "CTRL_L_mouth_upperLipRaise.translateY",
        "CTRL_R_mouth_sharpCornerPull.translateY",
        "CTRL_L_mouth_sharpCornerPull.translateY",
        "CTRL_R_mouth_cornerPull.translateY",
        "CTRL_L_mouth_cornerPull.translateY",
        "CTRL_R_mouth_dimple.translateY",
        "CTRL_L_mouth_dimple.translateY",
        "CTRL_R_mouth_cornerDepress.translateY",
        "CTRL_L_mouth_cornerDepress.translateY",
        "CTRL_R_mouth_stretch.translateY",
        "CTRL_L_mouth_stretch.translateY",
        "CTRL_R_mouth_lowerLipDepress.translateY",
        "CTRL_L_mouth_lowerLipDepress.translateY",
        "CTRL_R_nose.translateX",
        "CTRL_R_nose.translateY",
        "CTRL_L_nose.translateX",
        "CTRL_L_nose.translateY",
        "CTRL_C_eye.translateX",
        "CTRL_C_eye.translateY",
        "CTRL_R_eye.translateX",
        "CTRL_R_eye.translateY",
        "CTRL_L_eye.translateX",
        "CTRL_L_eye.translateY",
        "CTRL_R_eye_squintInner.translateY",
        "CTRL_L_eye_squintInner.translateY",
        "CTRL_C_eye_parallelLook.translateY",
        "CTRL_R_eye_blink.translateY",
        "CTRL_L_eye_blink.translateY",
        "CTRL_R_brow_down.translateY",
        "CTRL_L_brow_down.translateY",
        "CTRL_R_brow_lateral.translateY",
        "CTRL_L_brow_lateral.translateY",
        "CTRL_R_brow_raiseIn.translateY",
        "CTRL_L_brow_raiseIn.translateY",
        "CTRL_R_brow_raiseOut.translateY",
        "CTRL_L_brow_raiseOut.translateY",
        "CTRL_R_eye_cheekRaise.translateY",
        "CTRL_L_eye_cheekRaise.translateY",
        "CTRL_R_ear_up.translateY",
        "CTRL_L_ear_up.translateY",
        "CTRL_R_mouth_suckBlow.translateY",
        "CTRL_L_mouth_suckBlow.translateY",
        "CTRL_R_jaw_chinCompress.translateY",
        "CTRL_L_jaw_chinCompress.translateY",
        "CTRL_R_jaw_ChinRaiseU.translateY",
        "CTRL_L_jaw_ChinRaiseU.translateY",
        "CTRL_R_jaw_ChinRaiseD.translateY",
        "CTRL_L_jaw_ChinRaiseD.translateY",
        "CTRL_C_jaw_fwdBack.translateY",
        "CTRL_R_jaw_clench.translateY",
        "CTRL_L_jaw_clench.translateY",
        "CTRL_C_mouth_stickyU.translateY",
        "CTRL_C_mouth_stickyD.translateY",
        "CTRL_R_mouth_stickyInnerU.translateY",
        "CTRL_L_mouth_stickyInnerU.translateY",
        "CTRL_R_mouth_stickyInnerD.translateY",
        "CTRL_L_mouth_stickyInnerD.translateY",
        "CTRL_R_mouth_stickyOuterU.translateY",
        "CTRL_L_mouth_stickyOuterU.translateY",
        "CTRL_R_mouth_stickyOuterD.translateY",
        "CTRL_L_mouth_stickyOuterD.translateY",
        "CTRL_R_mouth_lipSticky.translateY",
        "CTRL_L_mouth_lipSticky.translateY",
        "CTRL_R_mouth_lipsBlow.translateY",
        "CTRL_L_mouth_lipsBlow.translateY",
        "CTRL_R_mouth_towardsU.translateY",
        "CTRL_L_mouth_towardsU.translateY",
        "CTRL_R_mouth_towardsD.translateY",
        "CTRL_L_mouth_towardsD.translateY",
        "CTRL_R_mouth_purseU.translateY",
        "CTRL_L_mouth_purseU.translateY",
        "CTRL_R_mouth_purseD.translateY",
        "CTRL_L_mouth_purseD.translateY",
        "CTRL_R_mouth_funnelU.translateY",
        "CTRL_L_mouth_funnelU.translateY",
        "CTRL_R_mouth_funnelD.translateY",
        "CTRL_L_mouth_funnelD.translateY",
        "CTRL_R_mouth_pressU.translateY",
        "CTRL_L_mouth_pressU.translateY",
        "CTRL_R_mouth_pressD.translateY",
        "CTRL_L_mouth_pressD.translateY",
        "CTRL_R_mouth_lipBiteU.translateY",
        "CTRL_L_mouth_lipBiteU.translateY",
        "CTRL_R_mouth_lipBiteD.translateY",
        "CTRL_L_mouth_lipBiteD.translateY",
        "CTRL_R_mouth_lipsPressU.translateY",
        "CTRL_L_mouth_lipsPressU.translateY",
        "CTRL_R_mouth_tightenU.translateY",
        "CTRL_L_mouth_tightenU.translateY",
        "CTRL_R_mouth_tightenD.translateY",
        "CTRL_L_mouth_tightenD.translateY",
        "CTRL_C_neck_swallow.translateY",
        "CTRL_neck_digastricUpDown.translateY",
        "CTRL_neck_throatExhaleInhale.translateY",
        "CTRL_neck_throatUpDown.translateY",
        "CTRL_R_neck_stretch.translateY",
        "CTRL_L_neck_stretch.translateY",
        "CTRL_R_neck_mastoidContract.translateY",
        "CTRL_L_neck_mastoidContract.translateY",
        "CTRL_R_nose_nasolabialDeepen.translateY",
        "CTRL_L_nose_nasolabialDeepen.translateY",
        "CTRL_R_mouth_lipsTowardsTeethU.translateY",
        "CTRL_L_mouth_lipsTowardsTeethU.translateY",
        "CTRL_R_mouth_lipsTowardsTeethD.translateY",
        "CTRL_L_mouth_lipsTowardsTeethD.translateY",
        "CTRL_R_mouth_corner.translateX",
        "CTRL_R_mouth_corner.translateY",
        "CTRL_L_mouth_corner.translateX",
        "CTRL_L_mouth_corner.translateY",
        "CTRL_C_mouth_lipShiftU.translateY",
        "CTRL_C_mouth_lipShiftD.translateY",
        "CTRL_R_mouth_pushPullU.translateY",
        "CTRL_L_mouth_pushPullU.translateY",
        "CTRL_R_mouth_pushPullD.translateY",
        "CTRL_L_mouth_pushPullD.translateY",
        "CTRL_R_mouth_cornerSharpnessU.translateY",
        "CTRL_L_mouth_cornerSharpnessU.translateY",
        "CTRL_R_mouth_cornerSharpnessD.translateY",
        "CTRL_L_mouth_cornerSharpnessD.translateY",
        "CTRL_R_mouth_lipsRollU.translateY",
        "CTRL_L_mouth_lipsRollU.translateY",
        "CTRL_R_mouth_lipsRollD.translateY",
        "CTRL_L_mouth_lipsRollD.translateY",
        "CTRL_R_mouth_thicknessU.translateY",
        "CTRL_L_mouth_thicknessU.translateY",
        "CTRL_R_mouth_thicknessD.translateY",
        "CTRL_L_mouth_thicknessD.translateY",
        "CTRL_R_eye_faceScrunch.translateY",
        "CTRL_L_eye_faceScrunch.translateY",
        "CTRL_R_eye_eyelidU.translateY",
        "CTRL_L_eye_eyelidU.translateY",
        "CTRL_R_eye_eyelidD.translateY",
        "CTRL_L_eye_eyelidD.translateY",
        "CTRL_L_mouth_lipsTogetherD.translateY",
        "CTRL_R_mouth_lipsTogetherD.translateY",
        "CTRL_C_eyesAim.translateX",
        "CTRL_C_eyesAim.translateY",
        "CTRL_C_eyesAim.translateZ",
        "CTRL_C_eyesAim.rotateX",
        "CTRL_C_eyesAim.rotateY",
        "CTRL_C_eyesAim.rotateZ",
        "CTRL_convergenceSwitch.translateY",
        "CTRL_L_eyeAim.translateX",
        "CTRL_L_eyeAim.translateY",
        "CTRL_L_eyeAim.translateZ",
        "CTRL_L_eyeAim.rotateX",
        "CTRL_L_eyeAim.rotateY",
        "CTRL_L_eyeAim.rotateZ",
        "CTRL_R_eyeAim.translateX",
        "CTRL_R_eyeAim.translateY",
        "CTRL_R_eyeAim.translateZ",
        "CTRL_R_eyeAim.rotateX",
        "CTRL_R_eyeAim.rotateY",
        "CTRL_R_eyeAim.rotateZ",
        "CTRL_L_eye_pupil.translateY",
        "CTRL_R_eye_pupil.translateY",
        "CTRL_L_eye_lidPress.translateY",
        "CTRL_R_eye_lidPress.translateY",
        "CTRL_L_nose_wrinkleUpper.translateY",
        "CTRL_R_nose_wrinkleUpper.translateY",
        "CTRL_L_mouth_stretchLipsClose.translateY",
        "CTRL_R_mouth_stretchLipsClose.translateY",
        "CTRL_L_mouth_lipsTogetherU.translateY",
        "CTRL_R_mouth_lipsTogetherU.translateY",
        "CTRL_C_jaw_openExtreme.translateY",
        "CTRL_C_teethU.translateX",
        "CTRL_C_teethU.translateY",
        "CTRL_C_teeth_fwdBackU.translateY",
        "CTRL_C_teethD.translateX",
        "CTRL_C_teethD.translateY",
        "CTRL_C_teeth_fwdBackD.translateY",
        "CTRL_L_eyelashes_tweakerIn.translateY",
        "CTRL_R_eyelashes_tweakerIn.translateY",
        "CTRL_L_eyelashes_tweakerOut.translateY",
        "CTRL_R_eyelashes_tweakerOut.translateY",
        "CTRL_neckCorrectivesMultiplyerU.translateY",
        "CTRL_neckCorrectivesMultiplyerM.translateY",
        "CTRL_neckCorrectivesMultiplyerD.translateY",
        "CTRL_faceGUIfollowHead.translateY",
        "CTRL_eyesAimFollowHead.translateY",
        "CTRL_C_tongue_move.translateX",
        "CTRL_C_tongue_move.translateY",
        "CTRL_C_tongue_wideNarrow.translateY",
        "CTRL_C_tongue_bendTwist.translateX",
        "CTRL_C_tongue_bendTwist.translateY",
        "CTRL_C_tongue_tipMove.translateX",
        "CTRL_C_tongue_tipMove.translateY",
        "CTRL_C_tongue_inOut.translateY",
        "CTRL_C_tongue_press.translateY",
        "CTRL_C_tongue_roll.translateY",
        "CTRL_C_tongue_thickThin.translateY",
    ]

    return controls


def create_expressions(node: str) -> str:
    """
    Creates a string of expressions based on the mapping from the mapping json file.

    Args:
        node (str): The name of the node.

    Returns:
        str: A string of expressions.
    """

    # mapping file must be in the same directory as the script if not path is provided
    # ARKit to MetaHuman Mapping
    Mapping_file = "MH_Mapping.json"

    # Load the mapping from the target_rig_mapping.json file
    script_dir = os.path.dirname(os.path.abspath(__file__))
    json_file_path = os.path.join(script_dir, Mapping_file)
    with open(json_file_path, "r") as json_file:
        # Load the mapping data from the JSON file
        target_rig_mapping = json.load(json_file)

    # Initialize a dictionary to store the output expressions
    OutputExpressions = {}

    # Create initial expressions with default values
    for control in target_rig_controls():
        OutputExpressions[f'{node}.{control.replace(".", "_")}'] = "0.0"

    # Build expressions based on the mapping data
    for ARKit_key, ARKit_map in target_rig_mapping.items():
        for target_rig_key, target_rig_value in ARKit_map.items():
            OutputExpressions[
                f'{node}.{target_rig_key.replace(".", "_")}'
            ] += f"+({target_rig_value}*{node}.{ARKit_key.lower()})"

    # Build the final expression string
    expressions = ""

    # Iterate over the output expressions and build the final string
    for key, value in OutputExpressions.items():
        expressions += f"""{key} = {value};\n"""

    return expressions
