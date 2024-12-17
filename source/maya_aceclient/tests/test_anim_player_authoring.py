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
from test_anim_player import TEST_SERVER_URL


class TestAceAnimationPlayerAuthoring(unittest.TestCase):

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
        cmds.setAttr(f"{node}.clientType", 1)  # 0=Streaming, 1=Authoring

        # force to evaluate the node
        cmds.dgeval(f"{node}.triggerSendAudio")

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

    def test_fetch_frame_cleans_needs_update(self):
        """outputWeights does not trigger service request."""
        from maya import api, cmds

        before = cmds.getAttr(f"{self._aceplayer}.needsUpdate")
        self.assertFalse(before)

        cmds.setAttr(f"{self._aceplayer}.skinStrength", 2.0)
        cmds.dgeval(f"{self._aceplayer}.outputWeights")
        after = cmds.getAttr(f"{self._aceplayer}.needsUpdate")
        self.assertEqual(before, after)
