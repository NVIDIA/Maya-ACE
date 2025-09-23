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
import os
import unittest
from unittest.mock import patch

from common import (
    ACE_PLAYER_NODE_NAME,
    ENV_API_KEY,
    ENV_DEFAULT_URL,
    TEST_AUDIO_FILEPATH,
    TEST_DIR,
)

OPTIONVAR_NAME = "NVCF_AGREEMENT_ACCEPTED"


class TestAceAnimationPlayerStreaming(unittest.TestCase):

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
        cmds.loadPlugin("maya_ace")

        node = cmds.createNode(ACE_PLAYER_NODE_NAME, name="test_node")
        cmds.setAttr(f"{node}.time", 0.0)
        cmds.setAttr(f"{node}.audiofile", TEST_AUDIO_FILEPATH, type="string")  # 4 sec
        cmds.setAttr(f"{node}.networkAddress", networkAddress, type="string")
        cmds.setAttr(f"{node}.apiKey", apiKey, type="string")

        # initialize the node and clear all dirty flags
        cmds.dgeval(f"{node}.triggerSendAudio")

        cls._aceplayer = node

    @classmethod
    def tearDownClass(cls):
        from maya import cmds, standalone

        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_ace", force=True)

        if hasattr(cls, "_mock_ace_server"):
            cls._mock_ace_server.stop(0)
            print(f"Mock server stopped")

    # def setUp(self):
    #     from maya import cmds

    # def tearDown(self):
    #     from maya import cmds

    def test_update_output_received(self):
        # check output received attributes
        from maya import api, cmds

        received = cmds.getAttr(f"{self._aceplayer}.received")
        self.assertTrue(received)

        frames = cmds.getAttr(f"{self._aceplayer}.receivedFrames")
        self.assertGreater(frames, 100)

    def test_updated_output_weights(self):
        # check output weights
        from maya import api, cmds

        weights = cmds.getAttr(f"{self._aceplayer}.outputWeights")
        self.assertGreater(len(weights[0]), 2)

    def test_update_output_for_another_frame(self):
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

    def test_update_output_for_audio_time_changes(self):
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

    def test_change_url_should_not_trigger_request(self):
        """outputWeights does not trigger service request."""
        from maya import api, cmds

        before = cmds.getAttr(f"{self._aceplayer}.receivedTime")
        prev_url = cmds.getAttr(f"{self._aceplayer}.networkAddress")
        cmds.setAttr(f"{self._aceplayer}.networkAddress", "some_invalid_url", type="string")
        cmds.dgeval(f"{self._aceplayer}.outputWeights")
        after = cmds.getAttr(f"{self._aceplayer}.receivedTime")

        self.assertEqual(before, after)
        cmds.setAttr(f"{self._aceplayer}.networkAddress", prev_url, type="string")

    def test_parameter_change_set_needs_update(self):
        """outputWeights does not trigger service request."""
        from maya import api, cmds

        # all dirty flags should be cleared
        cmds.dgeval(f"{self._aceplayer}.triggerSendAudio")
        before = cmds.getAttr(f"{self._aceplayer}.needsUpdate")
        self.assertFalse(before)

        # change parameter. trigger needsUpdate to be True
        cmds.setAttr(f"{self._aceplayer}.skinStrength", 2.0)
        cmds.dgeval(f"{self._aceplayer}.outputWeights")
        after = cmds.getAttr(f"{self._aceplayer}.needsUpdate")
        self.assertNotEqual(before, after)

        # force to evaluate the node
        cmds.dgeval(f"{self._aceplayer}.triggerSendAudio")
        after = cmds.getAttr(f"{self._aceplayer}.needsUpdate")
        self.assertEqual(before, after)


class TestAceAnimationPlayerNode(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_ace")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds, standalone

        cmds.unloadPlugin("maya_ace", force=True)

    def setUp(self):
        from maya import cmds

    def tearDown(self):
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def test_create_animation_player_node(self):
        from maya import api, cmds

        node = cmds.createNode(ACE_PLAYER_NODE_NAME, name="test_node")

        self.assertIn("test_node", cmds.ls(type=ACE_PLAYER_NODE_NAME))

    def test_animation_player_set_audiofile_attribute(self):
        from maya import api, cmds

        node = cmds.createNode(ACE_PLAYER_NODE_NAME, name="test_node")
        cmds.setAttr(f"{node}.audiofile", "__test_value__", type="string")

        self.assertEqual(cmds.getAttr(f"{node}.audiofile"), "__test_value__")

    def test_animation_player_load_sample_audio(self):
        from maya import api, cmds

        node = cmds.createNode(ACE_PLAYER_NODE_NAME, name="test_node")
        cmds.setAttr(f"{node}.audiofile", TEST_AUDIO_FILEPATH, type="string")

        cmds.dgeval(f"{node}.triggerLoad")

        loaded = cmds.getAttr(f"{node}.loaded")
        self.assertTrue(loaded)

        samples = cmds.getAttr(f"{node}.audioSamples")
        self.assertEqual(samples, 64000)

    def test_animation_player_has_audio_time_attributes(self):
        from maya import cmds

        node = cmds.createNode(ACE_PLAYER_NODE_NAME, name="test_node")

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
        cmds.loadPlugin("maya_ace")

        node = cmds.createNode(ACE_PLAYER_NODE_NAME, name="test_node")
        cmds.setAttr(f"{node}.time", 0.0)
        cmds.setAttr(f"{node}.audiofile", TEST_AUDIO_FILEPATH, type="string")  # 4 sec
        cmds.setAttr(f"{node}.networkAddress", networkAddress, type="string")
        cmds.setAttr(f"{node}.apiKey", apiKey, type="string")

        cls._aceplayer = node

    @classmethod
    def tearDownClass(cls):
        from maya import cmds, standalone

        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_ace", force=True)

        if hasattr(cls, "_mock_ace_server"):
            cls._mock_ace_server.stop(0)
            print(f"Mock server stopped")

    def test_animation_player_stops_with_nvcf_agreeement(self):
        # check output weights
        from maya import api, cmds

        # disagree with the agreement
        cmds.optionVar(iv=["NVCF_AGREEMENT_ACCEPTED", 0])
        cmds.setAttr(
            f"{self._aceplayer}.networkAddress", "https://grpc.nvcf.nvidia.com:443", type="string"
        )
        # force to evaluate the node
        cmds.dgeval(f"{self._aceplayer}.triggerSendAudio")
        received = cmds.getAttr(f"{self._aceplayer}.received")
        self.assertFalse(received)


class TestEmulateAgreement(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")

    @classmethod
    def tearDownClass(cls):
        from maya import standalone

        # standalone.uninitialize()

    @patch("maya.cmds.optionVar")
    def test_nvcf_agreed(self, mock_optionVar):
        # Simulate optionVar exists and is set to 1
        def optionVar_side_effect(*args, **kwargs):
            if kwargs.get("exists") == OPTIONVAR_NAME:
                return True
            if kwargs.get("q") and kwargs.get("intValue") == OPTIONVAR_NAME:
                return 1

        mock_optionVar.side_effect = optionVar_side_effect
        self.assertTrue(dummyAcquireAgreement())

    @patch("maya.cmds.optionVar")
    def test_nvcf_denied(self, mock_optionVar):
        # Simulate optionVar exists and is set to 0
        def optionVar_side_effect(*args, **kwargs):
            if kwargs.get("exists") == OPTIONVAR_NAME:
                return True
            if kwargs.get("q") and kwargs.get("intValue") == OPTIONVAR_NAME:
                return 0

        mock_optionVar.side_effect = optionVar_side_effect
        self.assertFalse(dummyAcquireAgreement())

    @patch("maya.cmds.optionVar")
    def test_nvcf_not_agreed(self, mock_optionVar):
        # Simulate optionVar does not exist
        def optionVar_side_effect(*args, **kwargs):
            if kwargs.get("exists") == OPTIONVAR_NAME:
                return False

        mock_optionVar.side_effect = optionVar_side_effect
        self.assertFalse(dummyAcquireAgreement())


def dummyAcquireAgreement():
    from maya import cmds

    exists = cmds.optionVar(exists=OPTIONVAR_NAME)
    if exists:
        agreed = cmds.optionVar(q=1, intValue=OPTIONVAR_NAME)
        if agreed:
            print("You agreed to the terms of the NVIDIA Cloud Agreement. Proceeding.")
            return True
    # Simulate batch mode or dialog as needed for your test
    print("You must agree to the terms to proceed.")
    return False
