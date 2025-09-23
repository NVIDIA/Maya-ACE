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
import argparse
import glob
import json
import os
import subprocess
import sys
from typing import Any, Dict, Generator, List, Union

from .models import list_local_a2e_models, list_local_a2f_models

TRTEXEC_PATH = "trtexec.exe"


def build_trt_command(
    onnx_path: str,
    output_path: str = None,
    trtexec_path: str = TRTEXEC_PATH,
    user_params: List[str] = None,
    user_defaults: Dict[str, Any] = None,
    optimize_batch_size: bool = False,
):
    """Build a trtexec command for a given ONNX file.

    Args:
        onnx_path (str): path to the ONNX file.
        output_path (str): path to the output file.
        trtexec_path (str): path to the trtexec executable.
        user_params (list[str]): list of additional parameters for trtexec.
        user_defaults (dict[str, Any]): default values for trtexec parameters.
        optimize_batch_size (bool): whether to optimize the batch size.

    Returns:
        (list[str]) list of arguments for the trtexec command.
    """
    onnx_path = os.path.realpath(onnx_path)

    if not output_path:
        output_path = os.path.splitext(onnx_path)[0] + ".trt"

    trtexec_path = os.path.realpath(trtexec_path)

    dirname = os.path.dirname(onnx_path)

    # likely, there will be trt_info.json
    trt_info_path = os.path.join(dirname, "trt_info.json")
    trt_info = {}
    if os.path.isfile(trt_info_path):
        with open(trt_info_path, "r") as f:
            trt_info = json.load(f)

    build_params = trt_info.get("trt_build_param", {})
    batch_params = build_params.get("batch", [])
    default_values = trt_info.get("defaults", {})
    if user_defaults:
        default_values.update(user_defaults)
    if optimize_batch_size:
        default_values["MAX_BATCH_SIZE"] = 1
        default_values["OPT_BATCH_SIZE"] = 1

    trt_params = [opt.format(**default_values) for opt in batch_params]

    cmd = [trtexec_path, f"--onnx={onnx_path}", f"--saveEngine={output_path}"]
    cmd.extend(trt_params)
    if user_params:
        for param in user_params:
            cmd.append(f"--{param}")

    return cmd


def main():
    parser = get_parser()
    args = parser.parse_args()

    # If using default command, ensure model_name exists
    if not hasattr(args, "model_name"):
        args.model_name = "*"

    # Find trtexec path
    trtexec_path = find_trtexec_path(args.trtexec)
    if not os.path.exists(trtexec_path):
        print(f"Error: trtexec not found at {trtexec_path}")
        sys.exit(1)

    # Find model directories
    model_dirs = [args.model_dir] if args.model_dir else []

    command = args.command

    # Get model_name from args safely
    model_name = getattr(args, "model_name", "*")

    # Handle list and build commands (using ONNX files)
    file_pattern = "*.onnx"
    if command == "clean" or command == "list_trt":
        file_pattern = "*.trt"

    # aggressively find paths. include potentially invalid models.
    file_paths = [
        p
        for p in list_local_a2f_models(
            pattern=f"{model_name}/{file_pattern}", model_dirs=model_dirs, skip_invalid=False
        )
    ]
    if not model_dirs:
        file_paths += [
            p
            for p in list_local_a2e_models(
                pattern=f"{model_name}/{file_pattern}", model_dirs=model_dirs, skip_invalid=False
            )
        ]

    print(f"Found {len(file_paths)} files:")
    for path in file_paths:
        print(f"  {path}")

    if command == "build" and file_paths:
        print("\nBuilding TensorRT engines...")
        user_params = args.params.split(",") if args.params else []
        built_engines = build_trt_engines(file_paths, trtexec_path, user_params)
        print("\nTensorRT engines built:")
        for engine in built_engines:
            print(f"  {engine}")

    if command == "clean" and file_paths:
        print("\nCleaning TensorRT engines...")
        clean_trt_files(file_paths)
        print("Done.")


def get_parser():
    parser = argparse.ArgumentParser(
        prog="trtgen", description="TensorRT engine management for Audio2Face models"
    )

    # Common arguments for all subcommands
    parser.add_argument("--trtexec", "-t", type=str, help="Path to trtexec executable")
    parser.add_argument("--model-dir", "-m", type=str, help="Directory containing models")

    # Create a parent parser for subcommands with common arguments
    parent_parser = argparse.ArgumentParser(add_help=False)
    parent_parser.add_argument(
        "model_name",
        type=str,
        default="*",
        nargs="?",
        help="Model name pattern (default: * for all models)",
    )

    # Create subparsers for different commands
    subparsers = parser.add_subparsers(dest="command", help="Command to execute")
    parser.set_defaults(command="list")
    # 'build' command to build TRT engines
    build_parser = subparsers.add_parser(
        "build", parents=[parent_parser], help="Build TensorRT engines from ONNX models"
    )
    build_parser.add_argument(
        "--params",
        "-p",
        type=str,
        help="Additional parameters for trtexec. use ',' to separate multiple parameters.",
    )

    # 'list' command to list ONNX models
    list_parser = subparsers.add_parser("list", parents=[parent_parser], help="List ONNX models")

    # 'clean' command to delete TRT files
    clean_parser = subparsers.add_parser(
        "clean", parents=[parent_parser], help="Delete TensorRT engine files"
    )

    # 'list_trt' command to list TRT engines
    list_trt_parser = subparsers.add_parser(
        "list_trt", parents=[parent_parser], help="List TensorRT engine files"
    )

    return parser


def list_file_paths(model_dirs: List[str], model_name: str, file_pattern: str):
    """Find files matching the pattern in the specified directories.

    Assuming models are organized as:
        - model_dir/
            - model_name/
                - file_pattern

    Args:
        model_dirs (list[str]): list of directories to search.
        model_name (str): name of the subdirectory to search for.
        file_pattern (str): pattern to match for files.

    Yields:
        (str) paths to the files.
    """
    for model_dir in model_dirs:
        for model_subdir in glob.glob(os.path.join(model_dir, model_name)):
            if not os.path.isdir(model_subdir):
                continue
            for file_path in glob.glob(os.path.join(model_subdir, file_pattern)):
                yield os.path.realpath(file_path)
    return  # terminate generator


def build_trt_engines(model_paths: List[str], trtexec_path: str, user_params: List[str] = None):
    """Build TensorRT engines for the given ONNX model paths.

    Args:
        model_paths (list[str]): list of paths to the ONNX model files.
        trtexec_path (str): path to the trtexec executable.
        user_params (list[str]): list of additional parameters for trtexec.

    Returns:
        (list[str]) list of paths to the built trt files.
    """
    built_engines = []
    for onnx_path in model_paths:
        print(f"Building TensorRT engine for: {onnx_path}")
        output_path = os.path.splitext(onnx_path)[0] + ".trt"
        cmd = build_trt_command(
            onnx_path, output_path, trtexec_path, user_params, optimize_batch_size=True
        )
        print(" ".join(cmd))
        result = subprocess.run(cmd)
        if result.returncode != 0:
            print(f"Error: TensorRT engine build failed with exit code {result.returncode}")
            continue
        if os.path.exists(output_path):
            built_engines.append(output_path)
    return built_engines


def clean_trt_files(trt_paths: List[str]):
    """Delete TensorRT engine files.

    Args:
        trt_paths (list[str]): list of paths to the TensorRT engine files.
    """
    for trt_path in trt_paths:
        print(f"Deleting: {trt_path}")
        try:
            os.remove(trt_path)
        except OSError as e:
            print(f"Error deleting {trt_path}: {e}")
    return


def find_trtexec_path(exec_name: str = None) -> str:
    """Find the path to the trtexec executable.

    Args:
        exec_name (str): name of the trtexec executable.

    Returns:
        (str) path to the trtexec executable.
    """
    trtexec_path = exec_name or os.getenv("TRTEXEC_PATH") or TRTEXEC_PATH
    trtexec_path = trtexec_path.split(";")[0].strip()  # pick the first one

    # search for trtexec_path in the PATH
    for path in os.getenv("PATH").split(";"):
        fullpath = os.path.realpath(os.path.join(path, trtexec_path))
        if os.path.exists(fullpath):
            return fullpath

    return trtexec_path


if __name__ == "__main__":
    main()
