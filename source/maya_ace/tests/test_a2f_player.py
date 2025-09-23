# SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

from common import A2F_PLAYER_NODE_NAME, PLUGIN_NAME, ROOT_DIR, TEST_AUDIO_FILEPATH
from maya import cmds, standalone

# TODO: change these path to a custom model
A2E_MODEL_PATH = os.path.normpath(
    os.path.join(
        ROOT_DIR, r"sample_project/models/audio2emotion-models/audio2emotion-v2.2/model.json"
    )
)
A2F_MODEL_PATH = os.path.normpath(
    os.path.join(ROOT_DIR, r"sample_project/models/audio2face-models/audio2face-3d-v3.0/model.json")
)


def create_node_with_default_params(init_func=None):
    node = cmds.createNode(A2F_PLAYER_NODE_NAME)
    cmds.setAttr(f"{node}.a2eModelPath", A2E_MODEL_PATH, type="string")
    cmds.setAttr(f"{node}.a2fModelPath", A2F_MODEL_PATH, type="string")
    cmds.setAttr(f"{node}.identityIndex", 1)
    cmds.setAttr(f"{node}.audiofile", TEST_AUDIO_FILEPATH, type="string")
    cmds.setAttr(f"{node}.audioStart", 0.0)
    cmds.setAttr(f"{node}.audioEnd", 4.0)

    if init_func:
        init_func(node)

    cmds.setAttr(f"{node}.time", 0)
    cmds.dgeval(f"{node}.outputWeights")
    cmds.setAttr(f"{node}.time", 0)
    cmds.dgeval(f"{node}.outputWeights")  # ensure background evaluation is done
    return node


class TestA2FPlayerNode(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        standalone.initialize(name="python")
        cmds.loadPlugin(PLUGIN_NAME)
        assert cmds.pluginInfo(PLUGIN_NAME, query=True, loaded=True), "Plugin failed to load"

    @classmethod
    def tearDownClass(cls):
        cmds.unloadPlugin(PLUGIN_NAME, force=True)

    def setUp(self):
        cmds.file(newFile=1, force=1)

    def tearDown(self):
        cmds.file(newFile=1, force=1)

    def test_create_node(self):
        test_node_name = "test_a2f_animation_player_node"
        node = cmds.createNode(A2F_PLAYER_NODE_NAME, name=test_node_name)
        self.assertEqual(node, test_node_name)
        self.assertIn(test_node_name, cmds.ls(type=A2F_PLAYER_NODE_NAME))

    def test_invalid_a2e_model_path(self):
        node = create_node_with_default_params(
            lambda node: cmds.setAttr(f"{node}.a2eModelPath", "invalid_path", type="string")
        )
        weights = cmds.getAttr(f"{node}.outputWeights")[0]
        self.assertEqual(len(weights), 0)

    def test_invalid_a2f_model_path(self):
        node = create_node_with_default_params(
            lambda node: cmds.setAttr(f"{node}.a2fModelPath", "invalid_path", type="string")
        )
        weights = cmds.getAttr(f"{node}.outputWeights")[0]
        self.assertEqual(len(weights), 0)

    def test_invalid_audio_file(self):
        node = create_node_with_default_params(
            lambda node: cmds.setAttr(f"{node}.audiofile", "invalid_path", type="string")
        )
        weights = cmds.getAttr(f"{node}.outputWeights")[0]
        self.assertEqual(len(weights), 0)

    def test_validate_test_node_default_params(self):
        """Validate the default parameters of the test node."""
        node = create_node_with_default_params()

        self.assertTrue(
            os.path.exists(cmds.getAttr(f"{node}.a2eModelPath")),
            f"A2E model path is invalid: {cmds.getAttr(f'{node}.a2eModelPath')}",
        )
        self.assertTrue(
            os.path.exists(cmds.getAttr(f"{node}.a2fModelPath")),
            f"A2F model path is invalid: {cmds.getAttr(f'{node}.a2fModelPath')}",
        )
        self.assertEqual(
            cmds.getAttr(f"{node}.identityIndex"),
            1,
            f"Identity index is not 1: {cmds.getAttr(f'{node}.identityIndex')}",
        )
        self.assertTrue(
            os.path.exists(cmds.getAttr(f"{node}.audiofile")),
            f"Audio file path is invalid: {cmds.getAttr(f'{node}.audiofile')}",
        )

        weights = cmds.getAttr(f"{node}.outputWeights")
        self.assertGreater(len(weights), 0)

    def test_get_blendshape_from_node(self):
        node = create_node_with_default_params()

        fps = 60
        duration_sec = 1

        frames = []
        for i in range(fps * duration_sec):
            t = i / float(fps)
            cmds.setAttr(f"{node}.time", t)
            cmds.dgeval(f"{node}.outputWeights")
            weights = cmds.getAttr(f"{node}.outputWeights")[0]
            self.assertGreater(len(weights), 0)
            frames.append(weights)

        for frame in frames:
            self.assertEqual(len(frame), len(frames[0]))

    def test_param_change_vs_time_change(self):
        node = create_node_with_default_params()

        MAX_RETRY = 5
        for i in range(MAX_RETRY):
            cmds.setAttr(f"{node}.lowerFaceStrength", i / float(MAX_RETRY))
            cmds.getAttr(f"{node}.outputWeights")
            self.assertTrue(cmds.getAttr(f"{node}.isQuickEstimate"))
            cmds.setAttr(f"{node}.time", 0)
            cmds.getAttr(f"{node}.outputWeights")
            cmds.getAttr(f"{node}.isQuickEstimate")
            cmds.setAttr(f"{node}.time", 0.1)
            cmds.getAttr(f"{node}.outputWeights")
            self.assertFalse(cmds.getAttr(f"{node}.isQuickEstimate"))

        for i in range(MAX_RETRY):
            cmds.setAttr(f"{node}.time", i % 2)  # alternate between 0 and 1
            cmds.getAttr(f"{node}.outputWeights")
            self.assertFalse(cmds.getAttr(f"{node}.isQuickEstimate"))

    def test_two_nodes(self):
        # create two nodes with different params
        node1 = create_node_with_default_params(
            lambda node: cmds.setAttr(f"{node}.lowerFaceStrength", 0.3)
        )
        node2 = create_node_with_default_params(
            lambda node: cmds.setAttr(f"{node}.lowerFaceStrength", 0.5)
        )

        frames1 = []
        frames2 = []
        MAX_RETRY = 50
        for i in range(MAX_RETRY):
            # only time is set, result should come from the background evaluation cache.
            cmds.setAttr(f"{node1}.time", i % 2)
            cmds.setAttr(f"{node2}.time", i % 2)
            weights1 = cmds.getAttr(f"{node1}.outputWeights")[0]
            weights2 = cmds.getAttr(f"{node2}.outputWeights")[0]
            self.assertFalse(cmds.getAttr(f"{node1}.isQuickEstimate"))
            self.assertFalse(cmds.getAttr(f"{node2}.isQuickEstimate"))
            self.assertGreater(len(weights1), 0)
            self.assertGreater(len(weights2), 0)
            frames1.append(weights1)
            frames2.append(weights2)

        for frame1, frame2 in zip(frames1, frames2):
            self.assertEqual(len(frame1), len(frame2))


class TestA2FPlayerNodeAdvanced(unittest.TestCase):
    """
    Test robustness of the node. Only create one node and use it for all tests.
    In each test, we will change the node's attributes and check if the node is still working.
    """

    @classmethod
    def setUpClass(cls):
        standalone.initialize(name="python")
        cmds.loadPlugin(PLUGIN_NAME)
        cls._player = create_node_with_default_params()

    @classmethod
    def tearDownClass(cls):
        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin(PLUGIN_NAME, force=True)

    def test_change_identity_index(self):
        cmds.setAttr(f"{self._player}.identityIndex", 2)
        weights = cmds.getAttr(f"{self._player}.outputWeights")[0]
        self.assertGreater(len(weights), 0)

    def test_change_audio_file(self):
        cmds.setAttr(f"{self._player}.audiofile", TEST_AUDIO_FILEPATH, type="string")
        weights = cmds.getAttr(f"{self._player}.outputWeights")[0]
        self.assertGreater(len(weights), 0)

    def test_change_a2e_model_path(self):
        cmds.setAttr(f"{self._player}.a2eModelPath", A2E_MODEL_PATH, type="string")
        weights = cmds.getAttr(f"{self._player}.outputWeights")[0]
        self.assertGreater(len(weights), 0)

    def test_change_a2f_model_path(self):
        cmds.setAttr(f"{self._player}.a2fModelPath", A2F_MODEL_PATH, type="string")
        weights = cmds.getAttr(f"{self._player}.outputWeights")[0]
        self.assertGreater(len(weights), 0)

    def test_change_emotion_params(self):
        cmds.setAttr(f"{self._player}.emotionStrength", 1.0)
        weights = cmds.getAttr(f"{self._player}.outputWeights")[0]
        self.assertGreater(len(weights), 0)

    def test_change_skin_params(self):
        cmds.setAttr(f"{self._player}.lowerFaceStrength", 0.5)
        weights = cmds.getAttr(f"{self._player}.outputWeights")[0]
        self.assertGreater(len(weights), 0)
