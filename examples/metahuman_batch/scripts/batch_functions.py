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

# ------------------------------------------------------------------------------------------------------------------------------------------------------
# this file contains extra functions that are used in the batch_a2f.py script
# the MH controls can be replaced by controls of a different rig and are provided as an working example for a common rig
# note that for FBX export, only X and Y translation are supported but more channels can be added if needed (see FBX SDK documentation for more details)
# ------------------------------------------------------------------------------------------------------------------------------------------------------


import json

import fbx
import numpy as np
import pandas as pd
import scipy.io.wavfile as wavfile
import scipy.signal as signal


# convert audio to supported format if needed
def convert_audio(audio_file_path):
    """
    Converts the input audio file to a temporary mono 16kHz 16-bit PCM WAV file
    compatible with A2F requirements using scipy instead of pydub.

    Args:
        audio_file_path (str): Path to the input WAV audio file.

    Returns:
        str: Path to the converted temporary audio file.
    """

    # Load the input WAV audio file using scipy
    sample_rate, audio_data = wavfile.read(audio_file_path)

    # Convert to mono if stereo (average the channels)
    if len(audio_data.shape) > 1:
        audio_data = np.mean(audio_data, axis=1)

    # Resample to 16kHz if needed
    if sample_rate != 16000:
        # Calculate the resampling ratio
        resample_ratio = 16000 / sample_rate
        # Resample using scipy.signal.resample
        audio_data = signal.resample(audio_data, int(len(audio_data) * resample_ratio))

    # Convert to 16-bit integer
    audio_data = audio_data.astype(np.int16)

    # Save the converted audio to a temporary file
    converted_audio_path = "temp_audio_a2f_compatible.wav"
    wavfile.write(converted_audio_path, 16000, audio_data)

    # Return the path to the converted audio file
    return converted_audio_path


def list_MH_controls():
    """
    Returns a list of all target rig (MetaHuman in this example) controls and their animation channels.
    Each control is represented as a string in the format 'NODE_NAME.channelName'.
    """

    # list of all MetaHuman controls and their animation channels

    MH_controls = [
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
        "CTRL_convergenceSwitch.translateY",
        "CTRL_L_mouth_lipsTogetherD.translateY",
        "CTRL_R_mouth_lipsTogetherD.translateY",
        "CTRL_R_eye_squintInner.translateY",
        "CTRL_L_eye_squintInner.translateY",
        "CTRL_C_eye_parallelLook.translateY",
        "CTRL_L_eye_pupil.translateY",
        "CTRL_R_eye_pupil.translateY",
        "CTRL_L_eye_lidPress.translateY",
        "CTRL_R_eye_lidPress.translateY",
        "CTRL_L_mouth_stretchLipsClose.translateY",
        "CTRL_R_mouth_stretchLipsClose.translateY",
        "CTRL_L_mouth_lipsTogetherU.translateY",
        "CTRL_R_mouth_lipsTogetherU.translateY",
        "CTRL_L_nose_wrinkleUpper.translateY",
        "CTRL_R_nose_wrinkleUpper.translateY",
        "CTRL_C_jaw_openExtreme.translateY",
        "CTRL_C_teethU.translateX",
        "CTRL_C_teethU.translateY",
        "CTRL_C_teeth_fwdBackU.translateY",
        "CTRL_C_teethD.translateX",
        "CTRL_C_teethD.translateY",
        "CTRL_C_teeth_fwdBackD.translateY",
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
        "CTRL_L_eyelashes_tweakerIn.translateY",
        "CTRL_R_eyelashes_tweakerIn.translateY",
        "CTRL_L_eyelashes_tweakerOut.translateY",
        "CTRL_R_eyelashes_tweakerOut.translateY",
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
    return MH_controls


def list_MH_nodes():
    """
    Returns a list of unique MetaHuman node names (without their animation channels).

    This function extracts the node names from the full control names returned by
    list_MH_controls(), which are in the format 'NODE_NAME.channelName'. Only the
    unique node names are kept in the returned list.
    """

    MH_nodes = []  # List to store unique MetaHuman node names

    # Iterate through all MetaHuman controls
    for a in list_MH_controls():
        # Split the control name at the '.' to separate node name from channel
        a_node = a.split(".")[0]
        # Add the node name to the list if it's not already present
        if a_node not in MH_nodes:
            MH_nodes.append(a_node)

    # Return the list of unique node names
    return MH_nodes


def save_keyframes_to_MH_animation(mapping, animation_key_frames, destinationpath, FBX_axis_system):
    """
    Saves the animation keyframes to a MetaHuman FBX file.

    This function takes the animation keyframes, mapping file, and target rig axis system,
    and exports the keyframes to a MetaHuman FBX file. The file is saved in the specified
    destination path.

    Args:
        mapping (str): Path to the mapping file.
        animation_key_frames (list): List of animation keyframes.
        destinationpath (str): Path to save the FBX file.
        FBX_axis_system (str): Axis system of the target rig (Maya or Unreal).
    """
    with open(mapping, "r") as jsonMappingFile:
        MappingToMH = json.load(jsonMappingFile)

    # get list of MH controls
    MH_Controls = list_MH_controls()

    # map ARKit Keyframes to MH controls keyframes

    # animation_key_frames structure
    # list of keyframes []
    #   keyframe is a dictionnary {}
    #       each keyframe has 2 main keys:
    #           timeCode -> time float value
    #           blendShapes
    #               blendshapes is a dictionnary where:
    #                   key is the ARKit blend shape name
    #                   value is the blend shape weight float value

    # create empty list for MH_animation_key_frames
    # this list will follow the same structure as animation_key_frames but will match MH controls

    # start with empt list
    MH_animation_key_frames = []

    # iterate through all the frames received from A2F
    for ARKit_keyframe in animation_key_frames:

        # create empty keyframe
        newKeyframe = {"timeCode": ARKit_keyframe["timeCode"], "MH_Controls": {}}

        # iterate through all the blendshapes received from A2F
        # set the start value to 0.0 before adding mapping contributions later
        for MH_Control in MH_Controls:
            newKeyframe["MH_Controls"][MH_Control] = 0.0

        # add mapped values from the received keyframes, multiplying the ARKit values by the MH mapping multipliers in the Mapping file
        for ARKit_blendshape, MH_mapped_controls in MappingToMH.items():
            for MH_Mapping, MH_Value in MH_mapped_controls.items():
                UpperCaseARKit_blendshape = ARKit_blendshape[:1].upper() + ARKit_blendshape[1:]
                newKeyframe["MH_Controls"][MH_Mapping] += (
                    ARKit_keyframe["blendShapes"][UpperCaseARKit_blendshape]
                ) * MH_Value

        # append MH_Keyframe
        MH_animation_key_frames.append(newKeyframe)

    # uncomment next 2 lines to save as CSV file (useful for debugging)
    # this will save the keyframes to a CSV file in the same directory as the destinationpath
    # format_animation = pd.json_normalize(MH_animation_key_frames)
    # format_animation.to_csv(f"{destinationpath}.csv")

    # save as FBX file
    # FBX SDK needs to be installed (download link below) with Python SDK
    # https://aps.autodesk.com/developer/overview/fbx-sdk

    # initialize and empty FBX scene container
    manager = fbx.FbxManager.Create()
    scene = fbx.FbxScene.Create(manager, "FBX_Scene")

    # intialize animation stack and layer
    animation_stack = fbx.FbxAnimStack.Create(scene, "AnimStack")
    animation_layer = fbx.FbxAnimLayer.Create(scene, "AnimLayer")
    animation_stack.AddMember(animation_layer)
    scene.SetCurrentAnimationStack(animation_stack)

    # define axis system

    # to load FBX in MH in Maya you need Z up
    # to load in MH in Unreal, you need to use Max
    # more axis systems are available if needed (see FBX SDK documentation for details)

    if FBX_axis_system == "Maya Yup":
        axis_system = fbx.FbxAxisSystem.MayaYUp
    elif FBX_axis_system == "Maya Zup":
        axis_system = fbx.FbxAxisSystem.MayaZUp
    elif FBX_axis_system == "Unreal":
        axis_system = fbx.FbxAxisSystem.Max
    else:
        print("Unsupported FBX axis system")
        return

    # make sure the scene is set to the target axis system
    axis_system.ConvertScene(scene)

    # initialize fbx time
    key_time = fbx.FbxTime()

    # define root node and its translation curves
    root_node = scene.GetRootNode()
    root_x_curve = root_node.LclTranslation.GetCurve(animation_layer, "X", True)
    root_y_curve = root_node.LclTranslation.GetCurve(animation_layer, "Y", True)
    key_time.SetFrame(0)
    key_index = root_x_curve.KeyAdd(key_time)[0]
    root_x_curve.KeySetValue(key_index, 0.0)
    key_index = root_y_curve.KeyAdd(key_time)[0]
    root_y_curve.KeySetValue(key_index, 0.0)

    # define an empty dictionnary to store the nodes and their curves
    FBX_nodes = {}

    # create name based nodes that will represent MH Controls and create translation curves (X and Y)
    for MH_node in list_MH_nodes():
        new_node = fbx.FbxNode.Create(scene, MH_node)
        root_node.AddChild(new_node)
        new_x_curve = new_node.LclTranslation.GetCurve(animation_layer, "X", True)
        new_y_curve = new_node.LclTranslation.GetCurve(animation_layer, "Y", True)
        new_node_entry = {"translateX": new_x_curve, "translateY": new_y_curve}
        FBX_nodes[MH_node] = new_node_entry

    # open the curves for animation keyframes
    # this is required to be able to add keyframes to the curves
    for control in MH_animation_key_frames[0]["MH_Controls"]:
        curve = FBX_nodes[control.split(".")[0]][control.split(".")[1]]
        curve.KeyModifyBegin()

    # add keyframes to the curves
    for keyframe in MH_animation_key_frames:
        current_frame = int(keyframe["timeCode"] * 30.0)  # convert to frame number
        key_time.SetFrame(current_frame)
        for control in keyframe["MH_Controls"]:
            curve = FBX_nodes[control.split(".")[0]][control.split(".")[1]]
            key_index = curve.KeyAdd(key_time)[0]
            key_value = keyframe["MH_Controls"][control]
            curve.KeySetValue(key_index, key_value)

    # close the curves for animation keyframes
    # this is required to be able to add keyframes to the curves
    for control in MH_animation_key_frames[0]["MH_Controls"]:
        curve = FBX_nodes[control.split(".")[0]][control.split(".")[1]]
        curve.KeyModifyEnd()

    # define the FBX file name and path
    exporter = fbx.FbxExporter.Create(manager, "")
    output_file = f"{destinationpath}.fbx"

    # export (use line 1 for binary or line 2 for ASCII export) -> ASCII is good for debugging!
    if exporter.Initialize(output_file):  # binary export
        # if exporter.Initialize(output_file, 1, manager.GetIOSettings()):    # ascii export
        exporter.Export(scene)
    else:
        print("Failed to initialize exporter.")
    exporter.Destroy()

    return


def process_a2f_params(a2f_params_data):
    """
    Processes the input parameter data from a2f_params_data and returns a dictionary
    formatted for use with the A2F Microservice.

    Args:
        a2f_params_data (dict): The input parameters, typically loaded from a .json or .yaml config.

    Returns:
        dict: A dictionary containing processed face, emotion, and blendshape parameters.
    """

    a2f_params = {}

    # Extract face parameters from the 'a2f' section
    a2f_params["face_params"] = a2f_params_data["a2f"]["face_params"]

    # Prepare lists for preferred emotions and their names
    a2f_params["preferred_emotion"] = []
    a2f_params["emotion_names"] = []
    # Iterate through preferred emotions and store their names and values
    for emotion, value in a2f_params_data["a2e"]["preferred_emotions"].items():
        a2f_params["emotion_names"].append(emotion)
        a2f_params["preferred_emotion"].append(value)

    # Initialize blendshape parameters dictionary
    a2f_params["blendshape_params"] = {}
    a2f_params["blendshape_params"]["bsWeightMultipliers"] = []
    a2f_params["blendshape_params"]["bsWeightOffsets"] = []
    a2f_params["blendshape_names"] = []

    # Extract blendshape weight multipliers and their names
    for blendshape, value in a2f_params_data["a2f"]["blendshape_params"][
        "weight_multipliers"
    ].items():
        a2f_params["blendshape_names"].append(blendshape)
        a2f_params["blendshape_params"]["bsWeightMultipliers"].append(value)

    # Extract blendshape weight offsets (order should match blendshape_names)
    for blendshape, value in a2f_params_data["a2f"]["blendshape_params"]["weight_offsets"].items():
        a2f_params["blendshape_params"]["bsWeightOffsets"].append(value)

    # Extract emotion post-processing parameters
    a2f_params["emotion_params"] = a2f_params_data["a2e"]["post_processing_params"]

    return a2f_params
