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

import maya.cmds as cmds
from maya.api.OpenMaya import MGlobal
from trtgen import models

from .presetcontrol import PresetControl


class LocalA2FModelPath(PresetControl):
    """Provides a quick a2f model setup"""

    affecting_attributes = ["a2fModelPath"]

    def listOptions(self):
        project_dir = cmds.workspace(query=True, rootDirectory=True)
        a2f_model_dirs = os.getenv("A2F_MODEL_DIRS", "").split(";") or []
        project_a2f_dir = os.path.join(project_dir, "models", "audio2face-models")
        if project_a2f_dir not in a2f_model_dirs:
            a2f_model_dirs += [project_a2f_dir]
        for i, model_path in enumerate(models.list_local_a2f_models(model_dirs=a2f_model_dirs)):
            dirname = os.path.dirname(model_path)
            vername = dirname.split(os.path.sep)[-1]

            settings = {
                "__label__": vername,
                "a2fModelPath": os.path.join(dirname, "model.json"),
            }
            yield settings
        return

    def hasAllValues(self, values: dict, compare_to: dict):
        project_root = cmds.workspace(query=True, rootDirectory=True)
        for key, val1 in values.items():
            if key.startswith("__"):
                continue
            if val1 == compare_to.get(key):
                continue
            if key in ("a2fModelPath", "a2eModelPath"):
                # If the path is relative, check if it's in the project root
                if _eq_path(val1, compare_to.get(key), project_root):
                    continue
                if _eq_path(compare_to.get(key), val1, project_root):
                    continue
            return False
        return True


def _eq_path(path: str, relative_path: str, project_root: str):
    path1 = os.path.normpath(path)
    path2 = os.path.normpath(os.path.join(project_root, relative_path))
    path3 = os.path.normpath(relative_path)
    return path1 in (path2, path3)


class LocalA2EModelPath(PresetControl):
    """Provides a quick a2e model setup"""

    affecting_attributes = ["a2eModelPath"]

    def listOptions(self):
        # pop option for disable a2e inference
        disable_a2e = {
            "__label__": "Disable",
            "a2eModelPath": "",
        }
        yield disable_a2e

        project_dir = cmds.workspace(query=True, rootDirectory=True)
        a2e_model_dirs = os.getenv("A2E_MODEL_DIRS", "").split(";") or []
        project_a2e_dir = os.path.join(project_dir, "models", "audio2emotion-models")
        if project_a2e_dir not in a2e_model_dirs:
            a2e_model_dirs += [project_a2e_dir]
        for i, model_path in enumerate(models.list_local_a2e_models(model_dirs=a2e_model_dirs)):
            dirname = os.path.dirname(model_path)
            vername = dirname.split(os.path.sep)[-1]

            settings = {
                "__label__": vername,
                "a2eModelPath": os.path.join(dirname, "model.json"),
            }
            yield settings
        return

    def hasAllValues(self, values: dict, compare_to: dict):
        project_root = cmds.workspace(query=True, rootDirectory=True)
        for key, val1 in values.items():
            if key.startswith("__"):
                continue
            if val1 == compare_to.get(key):
                continue
            if key in ("a2fModelPath", "a2eModelPath"):
                # If the path is relative, check if it's in the project root
                if _eq_path(val1, compare_to.get(key), project_root):
                    continue
                if _eq_path(compare_to.get(key), val1, project_root):
                    continue
            return False
        return True


class IdentityIndex(PresetControl):
    """Provides a quick identity index setup"""

    affecting_attributes = ["identityIndex"]
    add_custom_option = False

    def listOptions(self):
        model_path = cmds.getAttr(f"{self.nodeName}.a2fModelPath")
        model_dir = os.path.dirname(model_path)
        netinfo_path = os.path.join(model_dir, "network_info.json")
        project_root = cmds.workspace(query=True, rootDirectory=True)
        project_path = os.path.join(project_root, netinfo_path)

        if not os.path.exists(netinfo_path):
            if os.path.exists(project_path):
                netinfo_path = project_path
            else:
                MGlobal.displayWarning(f"Non-existing file: '{netinfo_path}'")
                return

        netinfo = json.load(open(netinfo_path))
        identities = netinfo.get("params", {}).get("identities", [])

        for i, identity in enumerate(identities):
            settings = {
                "__label__": identity,
                "identityIndex": i,
            }
            yield settings
        return
