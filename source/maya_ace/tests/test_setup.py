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

from common import TEST_DIR

TEST_AUDIO_1 = os.path.abspath(
    os.path.join(TEST_DIR, "../../../sample_data/audio_4sec_16k_s16le.wav")
)
TEST_AUDIO_2 = os.path.abspath(
    os.path.join(TEST_DIR, "../../../sample_data/audio_6sec_48k_s16le.wav")
)
TEST_SCENE = os.path.abspath(os.path.join(TEST_DIR, "data/simple_blendshape.ma"))


class TestAceToolsSetup(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_ace")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_ace", force=True)

    def setUp(self):
        from maya import cmds, mel

        node = cmds.createNode("AceAnimationPlayer", name="test_player")
        cmds.setAttr(f"{node}.time", 0.0)
        audio_nodes = cmds.file(
            TEST_AUDIO_1, type="audio", i=1, ignoreVersion=1, ra=1, returnNewNodes=1
        )
        self.assertIn("audio_4sec_16k_s16le", audio_nodes)

        self.player = node
        self.audio = audio_nodes[0]

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_connect_audio_to_player(self):
        from ace_tools.setup import (
            NODE_TYPE_ACE_PLAYER,
            connect_audio_to_animation_player,
        )
        from maya import cmds, mel

        player = cmds.createNode(NODE_TYPE_ACE_PLAYER)

        result = connect_audio_to_animation_player(self.audio, player)
        print(result)

        self.assertIn(self.audio, cmds.listConnections(player + ".audiofile"))
        self.assertIn(self.audio, cmds.listConnections(player + ".audioOffset"))
        self.assertIn(self.audio, cmds.listConnections(player + ".audioStart"))
        self.assertIn(self.audio, cmds.listConnections(player + ".audioEnd"))

    def test_create_ace_animation_player(self):
        from ace_tools.setup import NODE_TYPE_ACE_PLAYER, create_ace_animation_player
        from maya import cmds, mel

        player = create_ace_animation_player(name="TestPlayer")

        self.assertIn("time1", cmds.listConnections(f"{player}.time"))
        self.assertEqual(NODE_TYPE_ACE_PLAYER, cmds.nodeType(f"{player}"))

    def test_audio_connections_constant(self):
        """Test that AUDIO_CONNECTIONS constant is properly defined."""
        from ace_tools.setup import AUDIO_CONNECTIONS

        # Verify the constant exists and has expected structure
        self.assertIsInstance(AUDIO_CONNECTIONS, list)
        self.assertEqual(len(AUDIO_CONNECTIONS), 4)

        # Verify each connection tuple has the expected format
        expected_connections = [
            ("filename", "audiofile"),
            ("offset", "audioOffset"),
            ("sourceStart", "audioStart"),
            ("sourceEnd", "audioEnd"),
        ]

        for i, (audio_attr, player_attr) in enumerate(AUDIO_CONNECTIONS):
            self.assertEqual((audio_attr, player_attr), expected_connections[i])


class TestAceToolsSetupWithBlendshape(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_ace")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_ace", force=True)

    def setUp(self):
        from maya import cmds, mel

        cmds.file(TEST_SCENE, o=1, f=1, ignoreVersion=True)

        self.transform = "neutral"
        self.shape = "neutralShape"
        self.blendshape = "blendShape1"
        self.noshape_transform = "noshape"

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_connect_blendshape_via_blendshape(self):
        from ace_tools.setup import (
            NODE_TYPE_ACE_PLAYER,
            connect_animation_player_to_blendshapes,
        )
        from maya import cmds, mel

        player = cmds.createNode(NODE_TYPE_ACE_PLAYER)

        connect_animation_player_to_blendshapes(player, self.blendshape)

        self.assertIn(player, cmds.listConnections(f"{self.blendshape}.weight[0]"))

    def test_connect_blendshape_via_mesh(self):
        from ace_tools.setup import (
            NODE_TYPE_ACE_PLAYER,
            connect_animation_player_to_blendshapes,
        )
        from maya import cmds, mel

        player = cmds.createNode(NODE_TYPE_ACE_PLAYER)

        connect_animation_player_to_blendshapes(player, self.shape)

        self.assertIn(player, cmds.listConnections(f"{self.blendshape}.weight[0]"))

    def test_connect_blendshape_via_transform(self):
        from ace_tools.setup import (
            NODE_TYPE_ACE_PLAYER,
            connect_animation_player_to_blendshapes,
        )
        from maya import cmds, mel

        player = cmds.createNode(NODE_TYPE_ACE_PLAYER)

        connect_animation_player_to_blendshapes(player, self.transform)

        self.assertIn(player, cmds.listConnections(f"{self.blendshape}.weight[0]"))

    def test_attach_blendshape_via_blendshape(self):
        from ace_tools.setup import (
            NODE_TYPE_ACE_PLAYER,
            attach_ace_player_to_blendshapes,
        )
        from maya import cmds, mel

        player_node = attach_ace_player_to_blendshapes(self.blendshape)

        connected = cmds.listConnections(f"{self.blendshape}.weight[0]")  # MouthLeft
        self.assertTrue(cmds.ls(connected, type=NODE_TYPE_ACE_PLAYER))

        # check output aliases are set up
        self.assertTrue(cmds.objExists(f"{player_node}.out_EyeBlinkLeft"))
        self.assertTrue(cmds.objExists(f"{player_node}.out_amazement"))

    def test_attach_blendshape_via_mesh(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, attach_a2f_player_to_mesh
        from maya import cmds, mel

        player_node = attach_a2f_player_to_mesh(self.shape)

        self.assertEqual(NODE_TYPE_A2F_PLAYER, cmds.nodeType(f"{player_node}"))
        self.assertIn(self.shape, cmds.listConnections(f"{player_node}.faceGeometry", shapes=1))
        self.assertTrue(cmds.listConnections(f"{player_node}.faceGeometryOrig"))

    def test_initialize_output_aliases_and_get_weights_aliases(self):
        from ace_tools.setup import (
            NODE_TYPE_ACE_PLAYER,
            _get_output_weights_aliases,
            initialize_output_aliases,
        )
        from maya import cmds, mel

        player_node = cmds.createNode(NODE_TYPE_ACE_PLAYER)
        initialize_output_aliases(player_node)
        self.assertTrue(cmds.objExists(f"{player_node}.out_EyeBlinkLeft"))
        self.assertTrue(cmds.objExists(f"{player_node}.out_amazement"))

        out_bs_attr = f"{player_node}.outputWeights"
        out_bs_aliases = _get_output_weights_aliases(out_bs_attr)
        self.assertIn("eyeblinkleft", out_bs_aliases)
        self.assertEqual(len(out_bs_aliases), 68)

    def test_raise_error_attach_player_to_no_blendshape(self):
        """attach_ace_player_to_blendshapes() raises an exception with an invalid input node"""
        from ace_tools.setup import attach_ace_player_to_blendshapes
        from maya import cmds, mel

        self.assertRaises(
            RuntimeError, lambda: attach_ace_player_to_blendshapes(self.noshape_transform)
        )

    def test_connect_blendshape_force_false_with_existing_connections(self):
        from ace_tools.setup import (
            NODE_TYPE_ACE_PLAYER,
            connect_animation_player_to_blendshapes,
        )
        from maya import cmds, mel

        # Create two player nodes
        player1 = cmds.createNode(NODE_TYPE_ACE_PLAYER)
        player2 = cmds.createNode(NODE_TYPE_ACE_PLAYER)

        # Initialize output aliases for both players
        from ace_tools.setup import initialize_output_aliases

        initialize_output_aliases(player1)
        initialize_output_aliases(player2)

        # Connect first player to blendshape
        connect_animation_player_to_blendshapes(player1, self.blendshape, force=True)

        # Verify first player is connected
        connected_to_weight0 = cmds.listConnections(
            f"{self.blendshape}.weight[0]", plugs=1, source=True
        )
        self.assertIsNotNone(connected_to_weight0)
        self.assertTrue(any(player1 in conn for conn in connected_to_weight0))

        # Try to connect second player with force=False - should skip already connected weights
        connect_animation_player_to_blendshapes(player2, self.blendshape, force=False)

        # Verify first player is still connected to weight[0]
        connected_to_weight0 = cmds.listConnections(
            f"{self.blendshape}.weight[0]", plugs=1, source=True
        )
        self.assertTrue(any(player1 in conn for conn in connected_to_weight0))
        # Verify second player is NOT connected to weight[0]
        self.assertFalse(any(player2 in conn for conn in connected_to_weight0 or []))

    def test_connect_blendshape_force_false_partial_connection(self):
        from ace_tools.setup import (
            NODE_TYPE_ACE_PLAYER,
            connect_animation_player_to_blendshapes,
        )
        from maya import cmds, mel

        # Create two player nodes
        player1 = cmds.createNode(NODE_TYPE_ACE_PLAYER)
        player2 = cmds.createNode(NODE_TYPE_ACE_PLAYER)

        # Initialize output aliases
        from ace_tools.setup import initialize_output_aliases

        initialize_output_aliases(player1)
        initialize_output_aliases(player2)

        # Connect first player only to weight[0]
        cmds.connectAttr(f"{player1}.outputWeights[0]", f"{self.blendshape}.weight[0]")

        # Verify connection
        connected_to_weight0 = cmds.listConnections(
            f"{self.blendshape}.weight[0]", plugs=1, source=True
        )
        self.assertTrue(any(player1 in conn for conn in connected_to_weight0))

        # Connect second player with force=False - should connect to unconnected weights only
        connect_animation_player_to_blendshapes(player2, self.blendshape, force=False)

        # Verify first player is still connected to weight[0]
        connected_to_weight0 = cmds.listConnections(
            f"{self.blendshape}.weight[0]", plugs=1, source=True
        )
        self.assertTrue(any(player1 in conn for conn in connected_to_weight0))
        self.assertFalse(any(player2 in conn for conn in connected_to_weight0 or []))

        # Verify second player is connected to other weights (e.g., weight[1])
        if cmds.getAttr(f"{self.blendshape}.weight", size=1) > 1:
            connected_to_weight1 = cmds.listConnections(
                f"{self.blendshape}.weight[1]", plugs=1, source=True
            )
            self.assertTrue(any(player2 in conn for conn in connected_to_weight1 or []))


class TestAceToolsSetupWithGeometry(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_ace")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_ace", force=True)

    def setUp(self):
        from maya import cmds, mel

        cmds.file(TEST_SCENE, o=1, f=1, ignoreVersion=True)

        self.transform = "neutral"
        self.shape = "neutralShape"
        self.blendshape = "blendShape1"
        self.noshape_transform = "noshape"

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_connect_mesh_via_mesh(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, connect_a2f_player_to_mesh
        from maya import cmds, mel

        # Create a test player node
        player_node = cmds.createNode(NODE_TYPE_A2F_PLAYER)

        # Test connecting to mesh
        connect_a2f_player_to_mesh(player_node, self.shape)

        # Verify the original mesh is connected to player's faceGeometryOrig
        orig_connections = cmds.listConnections(
            f"{player_node}.faceGeometryOrig", plugs=1, source=True
        )
        self.assertIsNotNone(orig_connections)
        self.assertTrue(any("outMesh" in conn for conn in orig_connections))

        # Verify the player's faceGeometry is connected to the mesh's inMesh
        mesh_connections = cmds.listConnections(f"{self.shape}.inMesh", plugs=1, source=True)
        self.assertIsNotNone(mesh_connections)
        self.assertIn(f"{player_node}.faceGeometry", mesh_connections)

    def test_connect_mesh_via_transform_with_invalid_mesh(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, connect_a2f_player_to_mesh
        from maya import cmds, mel

        # Create a test player node
        player_node = cmds.createNode(NODE_TYPE_A2F_PLAYER)

        # Test with invalid mesh (transform with no shape)
        self.assertRaises(
            RuntimeError, lambda: connect_a2f_player_to_mesh(player_node, self.noshape_transform)
        )

    def test_connect_mesh_via_mesh_disconnects_existing(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, connect_a2f_player_to_mesh
        from maya import cmds, mel

        # Create two player nodes
        player1 = cmds.createNode(NODE_TYPE_A2F_PLAYER)
        player2 = cmds.createNode(NODE_TYPE_A2F_PLAYER)

        # Connect first player to mesh
        connect_a2f_player_to_mesh(player1, self.shape)

        # Verify first player is connected
        mesh_connections = cmds.listConnections(f"{self.shape}.inMesh", plugs=1, source=True)
        self.assertIn(f"{player1}.faceGeometry", mesh_connections)

        # Connect second player to same mesh
        connect_a2f_player_to_mesh(player2, self.shape)

        # Verify second player is now connected and first player is not
        mesh_connections = cmds.listConnections(f"{self.shape}.inMesh", plugs=1, source=True)
        self.assertIn(f"{player2}.faceGeometry", mesh_connections)
        self.assertNotIn(f"{player1}.faceGeometry", mesh_connections)

    def test_attach_a2f_player_to_mesh(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, attach_a2f_player_to_mesh
        from maya import cmds, mel

        # Test attaching to mesh
        player_node = attach_a2f_player_to_mesh(self.shape)

        # Verify the function returns a player node
        self.assertEqual(NODE_TYPE_A2F_PLAYER, cmds.nodeType(player_node))

        # Verify the player is connected to the mesh
        self.assertIn(self.shape, cmds.listConnections(f"{player_node}.faceGeometry", shapes=1))
        self.assertTrue(cmds.listConnections(f"{player_node}.faceGeometryOrig"))

        # Verify the player is added to the AnimationPlayersSet
        sets = cmds.listSets(object=player_node)
        self.assertTrue(any("AnimationPlayersSet" in s for s in (sets or [])))

        # Verify time is connected
        self.assertIn("time1", cmds.listConnections(f"{player_node}.time"))

    def test_attach_a2f_animation_player_to_transform(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, attach_a2f_player_to_mesh
        from maya import cmds, mel

        # Test attaching to transform (should work as it gets the shape)
        player_node = attach_a2f_player_to_mesh(self.transform)

        # Verify the function returns a player node
        self.assertEqual(NODE_TYPE_A2F_PLAYER, cmds.nodeType(player_node))

        # Verify the player is connected to the mesh shape
        self.assertIn(self.shape, cmds.listConnections(f"{player_node}.faceGeometry", shapes=1))

    def test_attach_a2f_player_to_mesh_with_invalid_mesh(self):
        from ace_tools.setup import attach_a2f_player_to_mesh
        from maya import cmds, mel

        # Test with invalid mesh (transform with no shape)
        self.assertRaises(RuntimeError, lambda: attach_a2f_player_to_mesh(self.noshape_transform))

    def test_attach_a2f_player_to_mesh_with_custom_set(self):
        from ace_tools.setup import attach_a2f_player_to_mesh
        from maya import cmds, mel

        # Test with custom set name
        custom_set = "MyCustomPlayersSet"
        player_node = attach_a2f_player_to_mesh(self.shape, set_name=custom_set)

        # Verify the player is added to the custom set
        sets = cmds.listSets(object=player_node)
        self.assertTrue(any(custom_set in s for s in (sets or [])))

    def test_attach_a2f_animation_player_with_audio(self):
        import ace_tools.setup
        from ace_tools.setup import attach_a2f_player_to_mesh
        from maya import cmds, mel

        # Import an audio file first
        audio_nodes = cmds.file(
            TEST_AUDIO_1, type="audio", i=1, ignoreVersion=1, ra=1, returnNewNodes=1
        )
        audio_node = audio_nodes[0]

        # Mock get_time_slider_audio to return our audio node
        # This simulates having audio in the timeline in non-batch mode
        with patch("ace_tools.setup.get_time_slider_audio", return_value=audio_node):
            # Attach player to mesh - should automatically connect audio
            player_node = attach_a2f_player_to_mesh(self.shape)

        # Verify audio is connected to the player
        audio_connections = cmds.listConnections(f"{player_node}.audiofile")
        self.assertIsNotNone(audio_connections)
        self.assertIn(audio_node, audio_connections)

    def test_attach_a2f_player_to_mesh_with_tongue(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, attach_a2f_player_to_mesh
        from maya import cmds, mel

        # Create a second mesh for tongue
        tongue_sphere = cmds.polySphere(name="tongue_geo")[0]
        tongue_shape = cmds.listRelatives(tongue_sphere, shapes=True)[0]

        # Test attaching to both skin and tongue mesh
        player_node = attach_a2f_player_to_mesh(self.shape, tongue_shape)

        # Verify the function returns a player node
        self.assertEqual(NODE_TYPE_A2F_PLAYER, cmds.nodeType(player_node))

        # Verify the player is connected to the skin mesh
        self.assertIn(self.shape, cmds.listConnections(f"{player_node}.faceGeometry", shapes=1))
        self.assertTrue(cmds.listConnections(f"{player_node}.faceGeometryOrig"))

        # Verify the player is connected to the tongue mesh
        self.assertIn(tongue_shape, cmds.listConnections(f"{player_node}.tongueGeometry", shapes=1))
        self.assertTrue(cmds.listConnections(f"{player_node}.tongueGeometryOrig"))

    def test_attach_a2f_player_to_mesh_no_tongue(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, attach_a2f_player_to_mesh
        from maya import cmds, mel

        # Test attaching to only skin mesh (no tongue)
        player_node = attach_a2f_player_to_mesh(self.shape, None)

        # Verify the function returns a player node
        self.assertEqual(NODE_TYPE_A2F_PLAYER, cmds.nodeType(player_node))

        # Verify the player is connected to the skin mesh
        self.assertIn(self.shape, cmds.listConnections(f"{player_node}.faceGeometry", shapes=1))

        # Verify no tongue connections exist
        tongue_connections = cmds.listConnections(f"{player_node}.tongueGeometry", shapes=1)
        self.assertIsNone(tongue_connections)

    def test_attach_a2f_player_with_selection_two_meshes(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, attach_a2f_player_to_mesh
        from maya import cmds, mel

        # Create a second mesh for tongue
        tongue_sphere = cmds.polySphere(name="tongue_geo")[0]

        # Select both meshes
        cmds.select(self.transform, tongue_sphere)

        # Test attachment using selection (no parameters)
        player_node = attach_a2f_player_to_mesh()

        # Verify both meshes are connected
        self.assertEqual(NODE_TYPE_A2F_PLAYER, cmds.nodeType(player_node))

        # Get the actual shape nodes
        tongue_shape = cmds.listRelatives(tongue_sphere, shapes=True)[0]

        # Verify connections
        self.assertIn(self.shape, cmds.listConnections(f"{player_node}.faceGeometry", shapes=1))
        self.assertIn(tongue_shape, cmds.listConnections(f"{player_node}.tongueGeometry", shapes=1))

    def test_connect_tongue_mesh_to_existing_player(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, connect_a2f_player_to_mesh
        from maya import cmds, mel

        # Create a player node and connect to skin mesh first
        player_node = cmds.createNode(NODE_TYPE_A2F_PLAYER)
        connect_a2f_player_to_mesh(player_node, self.shape, to_tongue=False)

        # Create a tongue mesh
        tongue_sphere = cmds.polySphere(name="tongue_geo")[0]
        tongue_shape = cmds.listRelatives(tongue_sphere, shapes=True)[0]

        # Connect tongue mesh to existing player
        connect_a2f_player_to_mesh(player_node, tongue_shape, to_tongue=True)

        # Verify both connections exist
        self.assertIn(self.shape, cmds.listConnections(f"{player_node}.faceGeometry", shapes=1))
        self.assertIn(tongue_shape, cmds.listConnections(f"{player_node}.tongueGeometry", shapes=1))

    def test_attach_a2f_player_with_invalid_tongue_mesh(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, attach_a2f_player_to_mesh
        from maya import cmds, mel

        # Test with valid skin mesh but invalid tongue mesh (transform with no shape)
        player_node = attach_a2f_player_to_mesh(self.shape, self.noshape_transform)

        # Should succeed but only connect skin mesh (tongue is skipped with info message)
        self.assertEqual(NODE_TYPE_A2F_PLAYER, cmds.nodeType(player_node))

        # Verify skin is connected
        self.assertIn(self.shape, cmds.listConnections(f"{player_node}.faceGeometry", shapes=1))

        # Verify no tongue connections
        tongue_connections = cmds.listConnections(f"{player_node}.tongueGeometry", shapes=1)
        self.assertIsNone(tongue_connections)

    def test_tongue_mesh_disconnects_existing(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, connect_a2f_player_to_mesh
        from maya import cmds, mel

        # Create two player nodes
        player1 = cmds.createNode(NODE_TYPE_A2F_PLAYER)
        player2 = cmds.createNode(NODE_TYPE_A2F_PLAYER)

        # Create a tongue mesh
        tongue_sphere = cmds.polySphere(name="tongue_geo")[0]
        tongue_shape = cmds.listRelatives(tongue_sphere, shapes=True)[0]

        # Connect first player to tongue mesh
        connect_a2f_player_to_mesh(player1, tongue_shape, to_tongue=True)

        # Verify first player is connected
        tongue_connections = cmds.listConnections(f"{tongue_shape}.inMesh", plugs=1, source=True)
        self.assertIn(f"{player1}.tongueGeometry", tongue_connections)

        # Connect second player to same tongue mesh
        connect_a2f_player_to_mesh(player2, tongue_shape, to_tongue=True)

        # Verify second player is now connected and first player is not
        tongue_connections = cmds.listConnections(f"{tongue_shape}.inMesh", plugs=1, source=True)
        self.assertIn(f"{player2}.tongueGeometry", tongue_connections)
        self.assertNotIn(f"{player1}.tongueGeometry", tongue_connections)

    def test_tongue_original_mesh_creation(self):
        from ace_tools.setup import NODE_TYPE_A2F_PLAYER, _connect_a2f_player_to_mesh
        from maya import cmds, mel

        # Create a player node
        player_node = cmds.createNode(NODE_TYPE_A2F_PLAYER)

        # Create a tongue mesh with some deformations
        tongue_sphere = cmds.polySphere(name="tongue_geo")[0]
        tongue_shape = cmds.listRelatives(tongue_sphere, shapes=True)[0]

        # Connect tongue mesh
        _connect_a2f_player_to_mesh(player_node, tongue_shape, to_tongue=True)

        # Verify original tongue mesh is created and connected
        orig_connections = cmds.listConnections(
            f"{player_node}.tongueGeometryOrig", plugs=1, source=True
        )
        self.assertIsNotNone(orig_connections)
        self.assertTrue(any("outMesh" in conn for conn in orig_connections))

        # Verify the original mesh is an intermediate object
        orig_mesh_node = orig_connections[0].split(".")[0]
        self.assertTrue(cmds.getAttr(f"{orig_mesh_node}.intermediateObject"))
