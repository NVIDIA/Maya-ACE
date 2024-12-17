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
import os
import re
import glob
import json

from maya import cmds, mel
from maya.api.OpenMaya import MGlobal

CURRENT_DIR = os.path.dirname(__file__)


def read_presets(pattern: str = "*ace_presets.json"):
    """Search and read presets files

    Keywords:
        pattern (str): file name glob pattern. defaults to "*ace_presets.json"

    Returns:
        (list) a list of dictionary, with preset contents
    """
    data_list = []

    for filepath in search_presets(pattern=pattern):
        with open(filepath, 'r') as fr:
            try:
                data = json.load(fr)
            except json.JSONDecodeError:
                MGlobal.displayWarning(f"Cannot read a preset file: {filepath}")
                continue

        if isinstance(data, dict):
            data_list.append(data)
        if isinstance(data, list):
            data_list.extend(data)

    return data_list


def search_presets(pattern: str = "*ace_presets.json", directory: str = ""):
    """Search files from presets directory and maya's app directory

    Keywords:
        pattern (str): file name glob pattern. defaults to "*ace_presets.json"
        directory (str): directory path to serach. defaults to mace/presets

    Yields:
        (str) a full filepath that matches the pattern
    """
    presets_dir = re.sub(r"scripts(/|\\)ace_tools(/|\\).*", "presets", __file__)
    if not directory:
        directory = presets_dir

    for filename in glob.glob(os.path.join(directory, pattern)):
        filepath = os.path.join(directory, filename)
        yield filepath

    maya_dir = get_maya_appdir()
    for filename in glob.glob(os.path.join(maya_dir, pattern)):
        filepath = os.path.join(maya_dir, filename)
        yield filepath

    return


def get_maya_appdir():
    """
    Returns:
        (str) maya's app directory. i.e. ~/maya
    """
    return cmds.internalVar(userAppDir=True)
