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

from maya import cmds, mel

# requires maya standalone initialized
from maya.api.OpenMaya import MGlobal


def import_audio(filepath: str, set_time_slider: bool = False, set_time_range: bool = False):
    """Create a new Audio node with the filepath.

    Args:
        filepath (str): Path to the audio file
        set_time_slider (bool): If True, set the audio on the time slider
        set_time_range (bool): If True and set_time_slider is True, also set the global time range

    Returns:
        (str) The audio node name
    """
    existings = list(find_audio_node_from_path(filepath))

    if existings:
        node, path = existings[0]  # get the first item
        return node

    ns, _ = os.path.splitext(os.path.basename(filepath))
    nodes = cmds.file(
        filepath, type="audio", i=1, ignoreVersion=1, renameAll=1, namespace=ns, returnNewNodes=1
    )
    if not nodes:
        # NOTE: should not happen
        return ""

    if set_time_slider and not cmds.about(batch=True):
        time_slider = set_time_slider_audio(nodes[0], set_range=set_time_range)

    return nodes[0]


def find_audio_node_from_path(match_filter: str = "*"):
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


def set_time_slider_audio(audio_node: str, fit_range: bool = False):
    """Set audio on the time slider and optionally set the time range.

    Args:
        audio_node (str): An Audio node
        set_range (bool): If True, set the global time range to match audio duration

    Returns:
        (str) time slider ui name
    """
    time_slider = _get_time_slider()
    result = cmds.timeControl(time_slider, e=True, sound=audio_node, displaySound=True)

    if fit_range:
        fit_time_range_to_audio(audio_node)

    return result  # time slider name


def fit_time_range_to_audio(audio_node: str):
    """Fit the time range to the audio duration.

    Args:
        audio_node (str): an audio node name

    Returns:
        (tuple): (0, start_frame, end_frame, end_frame)
    """
    # Get audio duration info (the values are frames)
    # maya frame[offset - source_start, offset - source_start + source_end]
    #       -> audio[source_start, source_end]
    offset = cmds.getAttr(f"{audio_node}.offset") or 0
    source_start = cmds.getAttr(f"{audio_node}.sourceStart") or 0
    source_end = cmds.getAttr(f"{audio_node}.sourceEnd") or 1  # +1 to the ui visible value
    duration = cmds.getAttr(f"{audio_node}.duration") or 0

    if source_end == 0:
        source_end = duration - source_start

    start_frame = offset
    # if source_end on ui is 10, the attribute value is 11. range of 0..10 is 11 frames.
    end_frame = offset + (source_end - 1) - source_start

    cmds.playbackOptions(
        animationStartTime=0,
        animationEndTime=int(end_frame),
        min=int(start_frame),
        max=int(end_frame),
    )

    return 0, int(start_frame), int(end_frame), int(end_frame)


def get_time_slider_audio():
    """
    Returns:
        (str) audio node name
    """
    time_slider = _get_time_slider()
    return cmds.timeControl(time_slider, q=True, sound=True)


def _get_time_slider():
    if cmds.about(batch=True):
        return ""
    time_slider = mel.eval("$tmpVar=$gPlayBackSlider")
    return time_slider
