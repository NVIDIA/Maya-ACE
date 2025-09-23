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
import glob
import os
from typing import Generator, List

A2F_MODEL_DIRS = "A2F_MODEL_DIRS"
A2E_MODEL_DIRS = "A2E_MODEL_DIRS"


def list_local_a2f_models(
    pattern="*/*.onnx", model_dirs: List[str] = None, skip_invalid: bool = True
):
    """Lists all available local Audio2Face models.

    Args:
        pattern (str): pattern to match for model files.
        model_dirs (list[str]): list of model directories to search.
        skip_invalid (bool): whether to skip invalid models.

    Yields:
        (str) paths to network.onnx files for valid Audio2Face models.
    """
    if not model_dirs:
        model_dirs = os.getenv(A2F_MODEL_DIRS, "").split(";")

    for path in find_all_files(pattern, *model_dirs):
        if skip_invalid and not is_valid_model(path):
            continue
        yield os.path.normpath(path)

    return


def list_local_a2e_models(
    pattern="*/*.onnx", model_dirs: List[str] = None, skip_invalid: bool = True
):
    """Lists all available local Audio2Emotion models.

    Args:
        pattern (str): pattern to match for model files.
        model_dirs (list[str]): list of model directories to search.
        skip_invalid (bool): whether to skip invalid models.

    Yields:
        (str) paths to network.onnx files for valid Audio2Emotion models.
    """
    if not model_dirs:
        model_dirs = os.getenv(A2E_MODEL_DIRS, "").split(";")

    for path in find_all_files(pattern, *model_dirs):
        if skip_invalid and not is_valid_model(path):
            continue
        yield os.path.normpath(path)

    return


def is_valid_model(path: str) -> bool:
    """Check if the given directory has a valid model.

    Args:
        path (str): path to the model directory.

    Returns:
        (bool) True if the model has valid trt file in it. False otherwise.
    """
    # TODO: generalize finding .trt file.
    if os.path.isfile(path):
        path = os.path.dirname(path)

    trt_path = os.path.join(path, "network.trt")
    if os.path.exists(trt_path):
        return True

    return False


def find_all_files(pattern: str = "*", *directories: str):
    """Find all files in the given directories.

    Args:
        pattern (str): pattern to match for model files.
        directories (list[str]): list of model directories to search.

    Yields:
        (str) paths to all files in the given directories.
    """
    _searched = set()
    for model_dir in directories:
        if model_dir is None:
            continue
        model_dir_norm = os.path.normpath(model_dir)
        if model_dir_norm in _searched:
            # skip duplicated directories
            continue
        _searched.add(model_dir_norm)
        for path in glob.glob(os.path.join(model_dir, pattern)):
            yield path
    return
