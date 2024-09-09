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
import json
import os
import tempfile
import unittest

from common import ENV_DEFAULT_URL, ENV_API_KEY
from maya import cmds, standalone

# remove Qt warnings with vanila mayapy
try:
    from PySide2 import QtCore, QtWidgets

    QtCore.QCoreApplication.setAttribute(QtCore.Qt.AA_ShareOpenGLContexts)
except ImportError:
    pass

TEST_DIR = os.path.dirname(__file__)
TEST_AUDIO_FILEPATH = os.path.abspath(os.path.join(TEST_DIR, "../../../sample_data/audio_4sec_16k_s16le.wav"))
TEST_SERVER_URL = "http://127.0.0.1:50051"


class TestCommandAceRequestAnimation(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Setup scene and request animation data.
        Do not change any audio/network info during tests in this class.
        """
        test_url = TEST_SERVER_URL
        test_api_key = os.getenv(ENV_API_KEY, "")
        # Start a mock server if the env variable is not defined or its value is empty.
        if not os.getenv(ENV_DEFAULT_URL, ""):
            # start mock server
            from mock_ace_server.main import setup_grpc_server

            server = setup_grpc_server(TEST_SERVER_URL)
            server.start()
            cls._mock_ace_server = server
            print(f"Mock server started, listening on {TEST_SERVER_URL}")
        else:
            test_url = os.getenv(ENV_DEFAULT_URL)
        print(f"The target server for unit testing is located at {test_url}.")

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_aceclient")

        nodename = cmds.createNode("AceAnimationPlayer", name="_test_node_01")

        cmds.setAttr(f"{nodename}.time", 0.0)
        cmds.setAttr(f"{nodename}.audiofile", TEST_AUDIO_FILEPATH, type="string")
        cmds.setAttr(f"{nodename}.networkAddress", test_url, type="string")
        cmds.setAttr(f"{nodename}.apiKey", test_api_key, type="string")

        cls._aceplayer = nodename

    @classmethod
    def tearDownClass(cls):
        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_aceclient", force=True)
        # standalone.uninitialize()

        if hasattr(cls, "_mock_ace_server"):
            cls._mock_ace_server.stop(0)
            print(f"Mock server stopped")

    def test_exec_request_animation_command(self):
        # check output received attributes

        received = cmds.getAttr(f"{self._aceplayer}.received")
        self.assertFalse(received)

        result = cmds.AceRequestAnimation(self._aceplayer)

        received = cmds.getAttr(f"{self._aceplayer}.received")
        self.assertTrue(received)

        frames = cmds.getAttr(f"{self._aceplayer}.receivedFrames")
        self.assertGreater(frames, 100)

        num_names = cmds.getAttr(f"{self._aceplayer}.outputBlendshapeNames", size=1)
        self.assertGreater(num_names, 10)

    def test_exec_request_animation_twice(self):
        # check output received attributes

        result = cmds.AceRequestAnimation(self._aceplayer)
        num_names = cmds.getAttr(f"{self._aceplayer}.outputBlendshapeNames", size=1)
        self.assertGreater(num_names, 10)

        result = cmds.AceRequestAnimation(self._aceplayer)
        val1 = cmds.getAttr(f"{self._aceplayer}.outputWeights[0]")
        bs_name = cmds.getAttr(f"{self._aceplayer}.outputBlendshapeNames[0]")
        val2 = cmds.getAttr(f"{self._aceplayer}.out_{bs_name}")
        self.assertLess(val1 - val2, 1e-3)


class TestAceExportConfigParametersCommand(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        standalone.initialize(name="python")
        cmds.loadPlugin("maya_aceclient")

    @classmethod
    def tearDownClass(cls):
        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_aceclient", force=True)

    def setUp(self):
        from maya import cmds

        # create a test node
        test_node_name = "test_ace_player_node"
        created = cmds.createNode("AceAnimationPlayer", name=test_node_name)

        self.node_name = created

    def tearDown(self):
        from maya import cmds
        cmds.file(newFile=1, force=1)

    def test_export_blendshape_parameters(self):
        from ace_config import ARKIT_FACE_EXPRESSIONS

        expectedBlendshapeMultiplers = []
        expectedBlendshapeOffsets = []
        for i, attrName in enumerate(ARKIT_FACE_EXPRESSIONS):
            expectedMultipler = 0.3 + (i / len(ARKIT_FACE_EXPRESSIONS)) * 0.1 # range: [0.3, 0.4)
            expectedOffset = 0.4 + (i / len(ARKIT_FACE_EXPRESSIONS)) * 0.1 # range: [0.4, 0.5)
            cmds.setAttr(f"{self.node_name}.multiply_{attrName}", expectedMultipler)
            cmds.setAttr(f"{self.node_name}.offset_{attrName}", expectedOffset)
            expectedBlendshapeMultiplers.append(expectedMultipler)
            expectedBlendshapeOffsets.append(expectedOffset)

        data = self._export_and_read_parameters(self.node_name)

        for i in range(len(ARKIT_FACE_EXPRESSIONS)):
            self.assertAlmostEqual(expectedBlendshapeMultiplers[i], data["blendshape_params"]["bsWeightMultipliers"][i])
            self.assertAlmostEqual(expectedBlendshapeOffsets[i], data["blendshape_params"]["bsWeightOffsets"][i])

    def test_export_emotion_parameters(self):
        from ace_config import EMOTION_LIST

        expectedEmotion = []
        for i, attrName in enumerate(EMOTION_LIST):
            expectedValue = 0.2 + (i / len(EMOTION_LIST)) * 0.1 # range: [0.2, 0.3)
            cmds.setAttr(f"{self.node_name}.{attrName}", expectedValue)
            expectedEmotion.append(expectedValue)

        data = self._export_and_read_parameters(self.node_name)

        for i in range(len(EMOTION_LIST)):
            self.assertAlmostEqual(expectedEmotion[i], data["preferred_emotion"][i])

    def test_export_face_parameters(self):
        from ace_config import FACE_PARAM_MAPPING

        expectedFaceParam = {}
        for i, attrName in enumerate(FACE_PARAM_MAPPING):
            expectedValue = 0.1 + (i / len(FACE_PARAM_MAPPING)) * 0.1 # range: [0.1, 0.2)
            cmds.setAttr(f"{self.node_name}.{attrName}", expectedValue)
            expectedFaceParam[attrName] = expectedValue

        data = self._export_and_read_parameters(self.node_name)

        for attrName, jsonKey in FACE_PARAM_MAPPING.items():
            self.assertAlmostEqual(expectedFaceParam[attrName], data["face_params"][jsonKey])

    def _export_and_read_parameters(self, test_node_name):
        # get a temp file path and export the configs
        temp_file_handle, temp_file_path = tempfile.mkstemp()
        os.close(temp_file_handle)
        cmds.AceExportConfigParameters(test_node_name, temp_file_path)

        # load the exported params and verify the values are correctly saved
        with open(temp_file_path, "r") as json_file:
            data = json.load(json_file)
        self.addCleanup(os.remove, temp_file_path)

        return data
