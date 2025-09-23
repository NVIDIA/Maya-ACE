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
import shutil
import tempfile

try:
    import yaml
except ImportError:
    yaml = None

from ace_tools.setup import NODE_TYPE_ACE_PLAYER
from ace_tools.ui.config_export import AceExportConfigWindow
from maya import cmds
from maya.api.OpenMaya import MGlobal

EXPORT_SERVICE_DISCLAIMER = """This export may not fully reflect recent updates from the server or ACE services.

The following limitations may result in potential discrepancies:
- Exported files are provided by the A2F service, suitable for service deployment.
- The format and content vary depending on the service and the service version.
- Note that some services may not support this export."""


class AceExportServiceConfigWindow(AceExportConfigWindow):

    export_consent_text = EXPORT_SERVICE_DISCLAIMER

    def __init__(self, title="Export A2F Service Configs"):
        super().__init__(title)

    def _on_export(self, *args):
        export_service_configs(self.get_selected_node(), self.get_file_path())

    def _populate_nodes(self):
        nodes = cmds.ls(type=NODE_TYPE_ACE_PLAYER) or []
        for node in nodes:
            cmds.menuItem(label=node, parent=self._option_node)

    def browse_file_path(self, *args):
        data_dir = cmds.workspace(expandName="data")
        if not os.path.exists(data_dir):
            data_dir = cmds.workspace(q=1, rootDirectory=1)

        file_path = cmds.fileDialog2(
            fileMode=3,
            dialogStyle=2,
            caption="Select a Directory",
            fileFilter="All Files (*.*)",
            startingDirectory=data_dir,
        )
        if not file_path:
            return
        file_path = file_path[0]
        cmds.textField(self._field_path, edit=True, text=file_path, ann=file_path)
        cmds.button(self._btn_export, e=True, enable=True)
        return file_path


def export_service_configs(node, filepath, config_type="YAML"):
    file_prefix = filepath
    if os.path.isdir(filepath) and filepath[-1] not in ("/", "\\"):
        file_prefix += "/"

    try:
        address = cmds.getAttr(f"{node}.networkAddress")
        apikey = cmds.getAttr(f"{node}.apiKey")
        functionid = cmds.getAttr(f"{node}.functionId")
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_prefix = temp_dir + "/"
            result = cmds.AceExportServiceConfig(
                filePrefix=temp_prefix,
                configType=config_type,
                address=address,
                apiKey=apikey,
                functionId=functionid,
            )
            exported_files = [os.path.join(temp_dir, f) for f in os.listdir(temp_dir)]
            output_files = []
            for temp_file in exported_files:
                # Determine output file path
                out_file = (
                    os.path.join(file_prefix, os.path.basename(temp_file))
                    if os.path.isdir(filepath)
                    else filepath
                )
                _rewrite_config_file(temp_file, out_file, config_type)
                output_files.append(out_file)
    except Exception as exc:
        MGlobal.displayError(f"{exc}")
        if not (cmds.about(batch=1)):
            cmds.confirmDialog(
                title="Error",
                message=f"{exc}",
                button=["OK"],
                defaultButton="OK",
                dismissString="OK",
            )
    else:
        MGlobal.displayInfo(f"Successfully exported files to {file_prefix}")
        MGlobal.displayInfo("\n".join(result))
        if not (cmds.about(batch=1)):
            cmds.confirmDialog(
                title="Export Success",
                message="\n".join([f"{w:30s}" for w in result]),
                button=["OK"],
                defaultButton="OK",
            )
    pass


def _rewrite_config_file(temp_file, out_file, config_type):
    with open(temp_file, "r", encoding="utf-8") as f:
        content = f.read()
    # Reformat content
    try:
        if config_type.upper() == "JSON":
            obj = json.loads(content)
            pretty = json.dumps(obj, indent=2, ensure_ascii=False)
        elif config_type.upper() == "YAML":
            if yaml is None:
                raise RuntimeError("PyYAML is not installed.")
            obj = yaml.safe_load(content)
            pretty = yaml.dump(obj, default_flow_style=False, allow_unicode=True)
        else:
            pretty = content
    except Exception as e:
        pretty = content  # fallback to raw if error
        MGlobal.displayInfo(f"Error rewriting config file: {e}. Writing raw content to file.")
    with open(out_file, "w", encoding="utf-8") as f:
        f.write(pretty)
