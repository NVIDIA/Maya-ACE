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
import unittest

from common import ENV_DEFAULT_URL, ENV_API_KEY

# remove Qt warnings with vanila mayapy
try:
    from PySide2 import QtCore, QtWidgets

    QtCore.QCoreApplication.setAttribute(QtCore.Qt.AA_ShareOpenGLContexts)
except ImportError:
    pass

TEST_DIR = os.path.dirname(__file__)
TEST_AUDIO_FILEPATH = os.path.abspath(os.path.join(TEST_DIR, "../../../sample_data/audio_4sec_16k_s16le.wav"))
TEST_SERVER_URL = "http://127.0.0.1:50051"


class TestAceAnimationPlayerNode(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_aceclient")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds, standalone

        cmds.unloadPlugin("maya_aceclient", force=True)

    def setUp(self):
        from maya import cmds

    def tearDown(self):
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def test_create_animation_player_node(self):
        from maya import api, cmds

        node = cmds.createNode("AceAnimationPlayer", name="test_node")

        self.assertIn("test_node", cmds.ls(type="AceAnimationPlayer"))

    def test_animation_player_set_audiofile_attribute(self):
        from maya import api, cmds

        node = cmds.createNode("AceAnimationPlayer", name="test_node")
        cmds.setAttr(f"{node}.audiofile", "__test_value__", type="string")

        self.assertEqual(cmds.getAttr(f"{node}.audiofile"), "__test_value__")

    def test_animation_player_load_sample_audio(self):
        from maya import api, cmds

        node = cmds.createNode("AceAnimationPlayer", name="test_node")
        cmds.setAttr(f"{node}.audiofile", TEST_AUDIO_FILEPATH, type="string")

        cmds.dgeval(f"{node}.triggerLoad")

        loaded = cmds.getAttr(f"{node}.loaded")
        self.assertTrue(loaded)

        samples = cmds.getAttr(f"{node}.audioSamples")
        self.assertEqual(samples, 64000)

    def test_animation_player_has_audio_time_attributes(self):
        from maya import cmds

        node = cmds.createNode("AceAnimationPlayer", name="test_node")

        self.assertTrue(cmds.ls(f"{node}.audioOffset"))
        self.assertTrue(cmds.ls(f"{node}.audioStart"))
        self.assertTrue(cmds.ls(f"{node}.audioEnd"))


class TestAceAnimationPlayerRequestAnimation(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Setup scene and request animation data.
        Do not change any audio/network info during tests in this class.
        """
        networkAddress = TEST_SERVER_URL
        apiKey = os.getenv(ENV_API_KEY, "")
        # Start a mock server if the env variable is not defined or its value is empty.
        if not os.getenv(ENV_DEFAULT_URL, ""):
            # Start a mock server
            from mock_ace_server.main import setup_grpc_server

            server = setup_grpc_server(TEST_SERVER_URL)
            server.start()
            cls._mock_ace_server = server
            print(f"Mock server started, listening on {TEST_SERVER_URL}")
        else:
            networkAddress = os.getenv(ENV_DEFAULT_URL)
        print(f"The target server for unit testing is located at {networkAddress}.")

        # start maya standalone
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_aceclient")

        node = cmds.createNode("AceAnimationPlayer", name="test_node")
        cmds.setAttr(f"{node}.time", 0.0)
        cmds.setAttr(f"{node}.audiofile", TEST_AUDIO_FILEPATH, type="string")  # 4 sec
        cmds.setAttr(f"{node}.networkAddress", networkAddress, type="string")
        cmds.setAttr(f"{node}.apiKey", apiKey, type="string")
        # cmds.setAttr(f"{node}.networkAddress", "localhost:443", type="string")

        # force to evaluate the node
        cmds.dgeval(f"{node}.triggerRequest")

        cls._aceplayer = node

    @classmethod
    def tearDownClass(cls):
        from maya import cmds, standalone

        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_aceclient", force=True)

        if hasattr(cls, "_mock_ace_server"):
            cls._mock_ace_server.stop(0)
            print(f"Mock server stopped")

    # def setUp(self):
    #     from maya import cmds

    # def tearDown(self):
    #     from maya import cmds

    def test_animation_player_received_data_and_update_output_received(self):
        # check output received attributes
        from maya import api, cmds

        received = cmds.getAttr(f"{self._aceplayer}.received")
        self.assertTrue(received)

        frames = cmds.getAttr(f"{self._aceplayer}.receivedFrames")
        self.assertGreater(frames, 100)

    def test_animation_player_updated_output_weights(self):
        # check output weights
        from maya import api, cmds

        weights = cmds.getAttr(f"{self._aceplayer}.outputWeights")
        self.assertGreater(len(weights[0]), 2)

    def test_animation_player_updated_output_for_another_frame(self):
        # check output weights
        from maya import api, cmds

        cmds.setAttr(f"{self._aceplayer}.time", 0.0)
        cmds.dgeval(f"{self._aceplayer}.outputWeights")
        framenum1 = cmds.getAttr(f"{self._aceplayer}.currentFrame")

        cmds.setAttr(f"{self._aceplayer}.time", 24.0)
        cmds.dgeval(f"{self._aceplayer}.outputWeights")
        framenum2 = cmds.getAttr(f"{self._aceplayer}.currentFrame")

        self.assertEqual(framenum1, 0)
        self.assertEqual(framenum2, 30)

    def test_animation_player_updated_output_for_audio_time_changes(self):
        # check output weights
        from maya import api, cmds

        cmds.setAttr(f"{self._aceplayer}.time", 48.0)
        cmds.dgeval(f"{self._aceplayer}.outputWeights")
        framenum1 = cmds.getAttr(f"{self._aceplayer}.currentFrame")

        cmds.setAttr(f"{self._aceplayer}.audioOffset", 24.0)
        cmds.dgeval(f"{self._aceplayer}.outputWeights")
        framenum2 = cmds.getAttr(f"{self._aceplayer}.currentFrame")

        cmds.setAttr(f"{self._aceplayer}.audioStart", 24.0)
        cmds.dgeval(f"{self._aceplayer}.outputWeights")
        framenum3 = cmds.getAttr(f"{self._aceplayer}.currentFrame")

        cmds.setAttr(f"{self._aceplayer}.audioEnd", 24.0)
        cmds.dgeval(f"{self._aceplayer}.outputWeights")
        framenum4 = cmds.getAttr(f"{self._aceplayer}.currentFrame")

        self.assertEqual(framenum1, 60)
        self.assertEqual(framenum2, 30)
        self.assertEqual(framenum3, 60)
        self.assertEqual(framenum4, 30)

    def test_animation_player_change_url_should_not_trigger_request(self):
        """outputWeights does not trigger service request."""
        from maya import api, cmds

        before = cmds.getAttr(f"{self._aceplayer}.receivedTime")
        cmds.setAttr(f"{self._aceplayer}.networkAddress", TEST_SERVER_URL, type="string")
        cmds.dgeval(f"{self._aceplayer}.outputWeights")
        after = cmds.getAttr(f"{self._aceplayer}.receivedTime")

        self.assertEqual(before, after)
