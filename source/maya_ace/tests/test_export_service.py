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
import unittest
from unittest import mock


class TestExportServiceConfig(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def setUp(self):
        # Patch cmds and MGlobal
        from maya import cmds

        self._original_cmds = cmds

        patcher_cmds = mock.patch("ace_tools.ui.service_export.cmds", autospec=True)
        patcher_mglobal = mock.patch("ace_tools.ui.service_export.MGlobal", autospec=True)
        self.mock_cmds = patcher_cmds.start()
        self.mock_mglobal = patcher_mglobal.start()
        self.addCleanup(patcher_cmds.stop)
        self.addCleanup(patcher_mglobal.stop)
        # Set up fake node attributes
        self.mock_cmds.getAttr.side_effect = lambda attr: {
            "testNode.networkAddress": "localhost",
            "testNode.apiKey": "dummy",
            "testNode.functionId": "funcid",
        }[attr]
        self.mock_cmds.about.return_value = True
        self.mock_cmds.confirmDialog.return_value = None
        self.mock_cmds.AceExportServiceConfig = mock.Mock()

    def _write_fake_export(self, prefix, config_type, content, fname):
        os.makedirs(prefix, exist_ok=True)
        path = os.path.join(prefix, fname)
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
        return [fname]

    def test_export_json_formatting(self):
        from ace_tools.ui.service_export import export_service_configs

        with mock.patch("ace_tools.ui.service_export._rewrite_config_file") as mock_rewrite:

            def fake_export(**kwargs):
                return self._write_fake_export(
                    kwargs["filePrefix"],
                    "JSON",
                    '{"foo":    [1,2,3],   "bar": {"baz":1}}',
                    "test.json",
                )

            self.mock_cmds.AceExportServiceConfig.side_effect = fake_export
            with tempfile.TemporaryDirectory() as out_dir:
                export_service_configs("testNode", out_dir, config_type="JSON")
                mock_rewrite.assert_called_once()
                args, kwargs = mock_rewrite.call_args
                self.assertTrue(args[1].endswith("test.json"))
                self.assertEqual(args[2], "JSON")

    def test_export_yaml_formatting(self):
        from ace_tools.ui.service_export import export_service_configs

        with mock.patch("ace_tools.ui.service_export._rewrite_config_file") as mock_rewrite:

            def fake_export(**kwargs):
                return self._write_fake_export(
                    kwargs["filePrefix"], "YAML", "foo: [1,2,3]\nbar: {baz: 1}", "test.yaml"
                )

            self.mock_cmds.AceExportServiceConfig.side_effect = fake_export
            with tempfile.TemporaryDirectory() as out_dir:
                export_service_configs("testNode", out_dir, config_type="YAML")
                mock_rewrite.assert_called_once()
                args, kwargs = mock_rewrite.call_args
                self.assertTrue(args[1].endswith("test.yaml"))
                self.assertEqual(args[2], "YAML")

    def test_export_handles_invalid_json(self):
        from ace_tools.ui.service_export import export_service_configs

        with mock.patch("ace_tools.ui.service_export._rewrite_config_file") as mock_rewrite:

            def fake_export(**kwargs):
                return self._write_fake_export(
                    kwargs["filePrefix"], "JSON", "{not valid json", "broken.json"
                )

            self.mock_cmds.AceExportServiceConfig.side_effect = fake_export
            with tempfile.TemporaryDirectory() as out_dir:
                export_service_configs("testNode", out_dir, config_type="JSON")
                mock_rewrite.assert_called_once()
                args, kwargs = mock_rewrite.call_args
                self.assertTrue(args[1].endswith("broken.json"))
                self.assertEqual(args[2], "JSON")

    def test_rewrite_config_file_json(self):
        import json

        from ace_tools.ui.service_export import _rewrite_config_file

        with tempfile.TemporaryDirectory() as temp_dir:
            temp_file = os.path.join(temp_dir, "input.json")
            out_file = os.path.join(temp_dir, "output.json")
            with open(temp_file, "w", encoding="utf-8") as f:
                f.write('{"foo":    [1,2,3],   "bar": {"baz":1}}')
            _rewrite_config_file(temp_file, out_file, "JSON")
            with open(out_file, "r", encoding="utf-8") as f:
                data = f.read()
                obj = json.loads(data)
                self.assertEqual(obj, {"foo": [1, 2, 3], "bar": {"baz": 1}})
                self.assertTrue("\n  " in data or "\n    " in data)

    def test_rewrite_config_file_yaml(self):
        import yaml
        from ace_tools.ui.service_export import _rewrite_config_file

        with tempfile.TemporaryDirectory() as temp_dir:
            temp_file = os.path.join(temp_dir, "input.yaml")
            out_file = os.path.join(temp_dir, "output.yaml")
            with open(temp_file, "w", encoding="utf-8") as f:
                f.write("foo: [1,2,3]\nbar: {baz: 1}")
            _rewrite_config_file(temp_file, out_file, "YAML")
            with open(out_file, "r", encoding="utf-8") as f:
                data = f.read()
                self.assertIn("foo:", data)
                self.assertIn("bar:", data)
                self.assertIn("\n", data)
                self.assertTrue("[1, 2, 3]" in data or "- 1" in data)

    def test_rewrite_config_file_invalid_json(self):
        from ace_tools.ui.service_export import _rewrite_config_file

        with tempfile.TemporaryDirectory() as temp_dir:
            temp_file = os.path.join(temp_dir, "broken.json")
            out_file = os.path.join(temp_dir, "output.json")
            with open(temp_file, "w", encoding="utf-8") as f:
                f.write("{not valid json")
            _rewrite_config_file(temp_file, out_file, "JSON")
            with open(out_file, "r", encoding="utf-8") as f:
                data = f.read()
                self.assertEqual(data, "{not valid json")
