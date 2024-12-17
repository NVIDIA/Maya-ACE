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

from common import ENV_API_KEY, ENV_DEFAULT_URL, TEST_DIR, TEST_AUDIO_FILEPATH

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


class TestAceAnimationPlayerCloudAgreement(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Setup scene and request animation data.
        Do not change any audio/network info during tests in this class.
        """
        networkAddress = os.getenv(ENV_DEFAULT_URL, "")
        apiKey = os.getenv(ENV_API_KEY, "")

        if not networkAddress:
            # provide an available service, or use the mock server
            raise Exception(f"Environment variable {ENV_DEFAULT_URL} is required.")

        # start maya standalone
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_aceclient")

        node = cmds.createNode("AceAnimationPlayer", name="test_node")
        cmds.setAttr(f"{node}.time", 0.0)
        cmds.setAttr(f"{node}.audiofile", TEST_AUDIO_FILEPATH, type="string")  # 4 sec
        cmds.setAttr(f"{node}.networkAddress", networkAddress, type="string")
        cmds.setAttr(f"{node}.apiKey", apiKey, type="string")

        cls._aceplayer = node

    @classmethod
    def tearDownClass(cls):
        from maya import cmds, standalone

        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_aceclient", force=True)

        if hasattr(cls, "_mock_ace_server"):
            cls._mock_ace_server.stop(0)
            print(f"Mock server stopped")

    def test_animation_player_stops_with_nvcf_agreeement(self):
        # check output weights
        from maya import api, cmds

        # disagree with the agreement
        cmds.optionVar(iv=['NVCF_AGREEMENT_ACCEPTED', 0])
        cmds.setAttr(
            f"{self._aceplayer}.networkAddress",
            'https://grpc.nvcf.nvidia.com:443',
            type="string"
        )
        # force to evaluate the node
        cmds.dgeval(f"{self._aceplayer}.triggerSendAudio")
        received = cmds.getAttr(f"{self._aceplayer}.received")
        self.assertFalse(received)
