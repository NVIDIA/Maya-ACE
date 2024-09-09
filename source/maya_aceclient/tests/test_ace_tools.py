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

# remove Qt warnings with vanila mayapy
try:
    from PySide2 import QtCore, QtWidgets

    QtCore.QCoreApplication.setAttribute(QtCore.Qt.AA_ShareOpenGLContexts)
except ImportError:
    pass

TEST_DIR = os.path.dirname(__file__)
TEST_AUDIO_1 = os.path.abspath(os.path.join(TEST_DIR, "../../../sample_data/audio_4sec_16k_s16le.wav"))
TEST_AUDIO_2 = os.path.abspath(os.path.join(TEST_DIR, "../../../sample_data/audio_6sec_48k_s16le.wav"))
TEST_SCENE = os.path.abspath(os.path.join(TEST_DIR, "../../../sample_data/simple_blendshape.ma"))


class TestAceTools(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_aceclient")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_aceclient", force=True)

    def setUp(self):
        from maya import cmds, mel

        node = cmds.createNode("AceAnimationPlayer", name="test_player")
        cmds.setAttr(f"{node}.time", 0.0)
        audio_nodes = cmds.file(TEST_AUDIO_1, type="audio", i=1, ignoreVersion=1, ra=1, returnNewNodes=1)
        self.assertIn("audio_4sec_16k_s16le", audio_nodes)

        self.player = node
        self.audio = audio_nodes[0]

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_import_audio(self):
        import ace_tools
        from maya import cmds, mel

        imported = ace_tools.import_audio(TEST_AUDIO_2)
        self.assertEqual(imported, "audio_6sec_48k_s16le")

    def test_find_audio(self):
        import ace_tools
        from maya import cmds, mel

        found = list(ace_tools.find_audio_node_from_path(TEST_AUDIO_1))

        self.assertIn("audio_4sec_16k_s16le", [n for n, p in found])

    def test_connect_audio_to_player(self):
        import ace_tools
        from maya import cmds, mel

        player = cmds.createNode(ace_tools.NODE_TYPE_PLAYER)

        result = ace_tools.connect_audio_to_animation_player(self.audio, player)
        print(result)

        self.assertIn(self.audio, cmds.listConnections(player + ".audiofile"))
        self.assertIn(self.audio, cmds.listConnections(player + ".audioOffset"))
        self.assertIn(self.audio, cmds.listConnections(player + ".audioStart"))
        self.assertIn(self.audio, cmds.listConnections(player + ".audioEnd"))

    def test_create_animation_player(self):
        import ace_tools
        from maya import cmds, mel

        player = ace_tools.create_animation_player(name="TestPlayer")

        self.assertIn("time1", cmds.listConnections(f"{player}.time"))
        self.assertEqual(ace_tools.NODE_TYPE_PLAYER, cmds.nodeType(f"{player}"))


class TestAceToolsWithBlendshape(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_aceclient")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_aceclient", force=True)

    def setUp(self):
        from maya import cmds, mel

        cmds.file(TEST_SCENE, o=1, f=1)

        self.transform = "neutral"
        self.shape = "neutralShape"
        self.blendshape = "blendShape1"

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_find_blendshape_from_node(self):
        import ace_tools
        from maya import cmds, mel

        self.assertIn(self.blendshape, ace_tools.get_blendshapes_from_node(self.transform))
        self.assertIn(self.blendshape, ace_tools.get_blendshapes_from_node(self.shape))
        self.assertFalse(ace_tools.get_blendshapes_from_node("xx"))

    def test_connect_player_to_blendshape(self):
        import ace_tools
        from maya import cmds, mel

        player = cmds.createNode(ace_tools.NODE_TYPE_PLAYER)

        ace_tools.connect_animation_player_to_blendshape(player, self.blendshape)

        self.assertIn(player, cmds.listConnections(f"{self.blendshape}.weight[0]"))

    def test_connect_player_to_shape(self):
        import ace_tools
        from maya import cmds, mel

        player = cmds.createNode(ace_tools.NODE_TYPE_PLAYER)

        ace_tools.connect_animation_player_to_blendshape(player, self.shape)

        self.assertIn(player, cmds.listConnections(f"{self.blendshape}.weight[0]"))

    def test_connect_player_to_transform(self):
        import ace_tools
        from maya import cmds, mel

        player = cmds.createNode(ace_tools.NODE_TYPE_PLAYER)

        ace_tools.connect_animation_player_to_blendshape(player, self.transform)

        self.assertIn(player, cmds.listConnections(f"{self.blendshape}.weight[0]"))

    def test_attach_player_to_blendshape(self):
        import ace_tools
        from maya import cmds, mel

        ace_tools.attach_animation_player_to_blendshape(self.blendshape)

        connected = cmds.listConnections(f"{self.blendshape}.weight[0]")
        self.assertTrue(cmds.ls(connected, type=ace_tools.NODE_TYPE_PLAYER))
