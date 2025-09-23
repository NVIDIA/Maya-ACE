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
import tempfile
import unittest

from common import ENV_API_KEY, ENV_DEFAULT_URL, TEST_AUDIO_FILEPATH, TEST_DIR
from maya import cmds, standalone


class TestMayaClientBase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Setup scene and request animation data.
        Do not change any audio/network info during tests in this class.
        """
        test_url = os.getenv(ENV_DEFAULT_URL, "")
        test_api_key = os.getenv(ENV_API_KEY, "")

        if not test_url:
            raise Exception(f"Environment variable {ENV_DEFAULT_URL} is required.")

        cls._address = test_url
        # cls._address = "10.63.183.57:52000"
        cls._api_key = test_api_key
        cls._function_id = "462f7853-60e8-474a-9728-7b598e58472c"

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_ace")

        nodename = cmds.createNode("AceAnimationPlayer", name="_test_node_01")

        cmds.setAttr(f"{nodename}.time", 0.0)
        cmds.setAttr(f"{nodename}.audiofile", TEST_AUDIO_FILEPATH, type="string")
        cmds.setAttr(f"{nodename}.networkAddress", test_url, type="string")
        cmds.setAttr(f"{nodename}.apiKey", test_api_key, type="string")

        cls._aceplayer = nodename

    @classmethod
    def tearDownClass(cls):
        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_ace", force=True)
        # standalone.uninitialize()

        if hasattr(cls, "_mock_ace_server"):
            cls._mock_ace_server.stop(0)
            print(f"Mock server stopped")


class TestCommandExportServiceConfig(TestMayaClientBase):

    def test_exec_export_config_command(self):
        # check output received attributes
        prefix = self._get_export_path()
        try:
            result = cmds.AceExportServiceConfig(address=self._address, filePrefix=prefix)
        except AttributeError:
            self.assertTrue(False, "AceExportServiceConfig command does not exist.")

        self.assertTrue(result)

    def test_exec_export_config_command_args(self):
        # check output received attributes
        prefix = self._get_export_path()

        try:
            result = cmds.AceExportServiceConfig(
                address=self._address, apiKey="key-args", filePrefix=prefix
            )
        except TypeError:
            self.assertTrue(False, "AceExportServiceConfig command has wrong arguments.")

        try:
            result = cmds.AceExportServiceConfig(
                address=self._address, functionId="id-args", filePrefix=prefix
            )
        except TypeError:
            self.assertTrue(False, "AceExportServiceConfig command has wrong arguments.")

        # try:
        #     result = cmds.AceExportServiceConfig(node='node-args')
        # except TypeError:
        #     self.assertTrue(False, "AceExportServiceConfig command has wrong arguments.")

        self.assertTrue(result)

    def test_exec_export_config_command_with_server(self):
        # check output received attributes
        prefix = self._get_export_path()
        try:
            result = cmds.AceExportServiceConfig(
                address=self._address,
                apiKey=self._api_key,
                functionId=self._function_id,
                filePrefix=prefix,
            )
        except TypeError:
            self.assertTrue(False, "AceExportServiceConfig command has wrong arguments.")

        self.assertTrue(result)

        for filename in result:
            filepath = prefix + filename
            data = self._read_json_file(filepath)
            print(filepath, data)

    def _get_export_path(self):
        # get a temp file path and export the configs
        temp_file_handle, temp_file_path = tempfile.mkstemp()
        os.close(temp_file_handle)
        self.addCleanup(os.remove, temp_file_path)
        return temp_file_path

    def _read_json_file(self, temp_file_path):
        with open(temp_file_path, "r") as json_file:
            data = json.load(json_file)
        return data


class TestCommandAceRequestSendAudio(TestMayaClientBase):

    def test_exec_request_sendaudio_command(self):
        # check output received attributes

        received = cmds.getAttr(f"{self._aceplayer}.received")
        self.assertFalse(received)

        result = cmds.AceRequestSendAudio(self._aceplayer)

        received = cmds.getAttr(f"{self._aceplayer}.received")
        self.assertTrue(received)

        frames = cmds.getAttr(f"{self._aceplayer}.receivedFrames")
        self.assertGreater(frames, 100)

        num_names = cmds.getAttr(f"{self._aceplayer}.outputWeightNames", size=1)
        self.assertGreater(num_names, 10)

    def test_exec_request_sendaudio_twice(self):
        # check output received attributes

        result = cmds.AceRequestSendAudio(self._aceplayer)
        num_names = cmds.getAttr(f"{self._aceplayer}.outputWeightNames", size=1)
        self.assertGreater(num_names, 10)

        result = cmds.AceRequestSendAudio(self._aceplayer)
        val1 = cmds.getAttr(f"{self._aceplayer}.outputWeights[0]")
        bs_name = cmds.getAttr(f"{self._aceplayer}.outputWeightNames[0]")
        val2 = cmds.getAttr(f"{self._aceplayer}.out_{bs_name}")
        self.assertLess(val1 - val2, 1e-3)
