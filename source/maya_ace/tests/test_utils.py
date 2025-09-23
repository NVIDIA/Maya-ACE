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

from common import TEST_DIR

TEST_SCENE = os.path.abspath(os.path.join(TEST_DIR, "data/simple_blendshape.ma"))


class TestAceToolsUtilsAnimationPlayers(unittest.TestCase):
    """Test animation player set related functions."""

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def setUp(self):
        from maya import cmds, mel

        # Create a test node
        self.test_node = cmds.createNode("transform", name="testPlayer")

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_create_animation_players_set_new(self):
        """Test creating a new animation players set."""
        from ace_tools.utils import create_animation_players_set
        from maya import cmds

        # Test creating a new set
        result = create_animation_players_set(self.test_node, "NewAnimationSet")
        self.assertEqual(result, "NewAnimationSet")

        # Verify the set exists and contains our node
        self.assertTrue(cmds.objExists("NewAnimationSet"))
        self.assertIn(self.test_node, cmds.sets("NewAnimationSet", q=True) or [])

    def test_create_animation_players_set_existing(self):
        """Test adding to an existing animation players set."""
        from ace_tools.utils import create_animation_players_set
        from maya import cmds

        # Create an existing set first
        existing_set = cmds.sets(name="ExistingAnimationSet")

        # Test adding to existing set
        result = create_animation_players_set(self.test_node, "ExistingAnimationSet")
        self.assertEqual(result, "ExistingAnimationSet")

        # Verify our node was added to the existing set
        set_members = cmds.sets("ExistingAnimationSet", q=True) or []
        self.assertIn(self.test_node, set_members)

    def test_create_animation_players_set_name_conflict(self):
        """Test handling name conflicts with non-set objects."""
        from ace_tools.utils import create_animation_players_set
        from maya import cmds

        # Create a non-set object with the same name
        cmds.createNode("transform", name="ConflictingName")

        # Test creating set with conflicting name
        result = create_animation_players_set(self.test_node, "ConflictingName")

        # Should create a new set with the requested name (Maya will auto-rename if needed)
        self.assertIsNotNone(result)
        self.assertTrue(cmds.objExists(result))


class TestAceToolsUtilsWithBlendshape(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def setUp(self):
        from maya import cmds, mel

        cmds.file(TEST_SCENE, o=1, f=1, ignoreVersion=True)

        self.transform = "neutral"
        self.shape = "neutralShape"
        self.blendshape = "blendShape1"
        self.poses = ["MouthLeft", "MouthRight"]

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_find_blendshape_from_node(self):
        from ace_tools.utils import get_blendshapes_from_node
        from maya import cmds, mel

        self.assertIn(self.blendshape, get_blendshapes_from_node(self.transform))
        self.assertIn(self.blendshape, get_blendshapes_from_node(self.shape))
        self.assertIn(self.blendshape, get_blendshapes_from_node(self.blendshape))
        self.assertFalse(get_blendshapes_from_node("xx"))

    def test_filter_mesh_shapes(self):
        from ace_tools.utils import filter_mesh_shapes
        from maya import cmds, mel

        self.assertEqual(filter_mesh_shapes(self.shape), [self.shape])
        self.assertEqual(filter_mesh_shapes(self.transform), [self.shape])
        self.assertEqual(filter_mesh_shapes("xx"), [])

    def test_get_original_mesh(self):
        from ace_tools.utils import filter_mesh_shapes, get_original_mesh
        from maya import cmds, mel

        # get the exisiting original mesh
        self.assertEqual(get_original_mesh(self.shape), f"{self.shape}Orig.outMesh")

        # get nothing from a new mesh
        new_node = cmds.polyCube(n=f"newCube")
        new_mesh = filter_mesh_shapes(new_node)[0]
        self.assertEqual(get_original_mesh(new_mesh), "")

    def test_create_original_mesh(self):
        from ace_tools.utils import create_original_mesh, filter_mesh_shapes
        from maya import cmds, mel

        # get the exisiting original mesh
        self.assertEqual(create_original_mesh(self.shape), f"{self.shape}Orig.outMesh")

        new_node = cmds.polyCube(n=f"newCube")
        new_mesh = filter_mesh_shapes(new_node)[0]
        # create a new original mesh
        self.assertEqual(create_original_mesh(new_mesh), f"{new_mesh}Orig.outMesh")

    def test_get_meshes_from_history(self):
        """Test getting mesh nodes from history."""
        from ace_tools.utils import get_meshes_from_history
        from maya import cmds, mel

        # Test with transform node - should find mesh in history
        meshes = get_meshes_from_history(self.transform)
        self.assertIsInstance(meshes, list)
        self.assertTrue(len(meshes) > 0)

        # Test with shape node directly
        meshes = get_meshes_from_history(self.shape)
        self.assertIsInstance(meshes, list)
        self.assertIn(self.shape, meshes)

        # Test with blendshape node - should find upstream meshes
        meshes = get_meshes_from_history(self.blendshape)
        self.assertIsInstance(meshes, list)

        # Test with non-existent node - Maya raises ValueError for non-existent nodes
        with self.assertRaises(ValueError):
            get_meshes_from_history("nonExistentNode")

        # Test with list of nodes
        meshes = get_meshes_from_history([self.transform, self.shape])
        self.assertIsInstance(meshes, list)
        self.assertTrue(len(meshes) > 0)

    def test_get_meshes_from_history_empty_input(self):
        """Test get_meshes_from_history with empty/invalid input."""
        from ace_tools.utils import get_meshes_from_history
        from maya import cmds, mel

        # Test with empty list
        meshes = get_meshes_from_history([])
        self.assertEqual(meshes, [])

        # Test with empty string - Maya raises ValueError for empty node names
        with self.assertRaises(ValueError):
            get_meshes_from_history("")


class TestAceToolsUtilsAttributeAliases(unittest.TestCase):
    """Test attribute alias management functions."""

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def setUp(self):
        from maya import cmds, mel

        # Create a test node with some attributes
        self.test_node = cmds.createNode("transform", name="aliasTestNode")
        cmds.addAttr(
            self.test_node,
            longName="testArray",
            attributeType="double",
            multi=True,
            indexMatters=False,
        )
        cmds.addAttr(
            self.test_node,
            longName="singleAttr",
            attributeType="double",
        )

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_list_attr_aliases_all(self):
        """Test listing all attribute aliases."""
        from ace_tools.utils import list_attr_aliases
        from maya import cmds, mel

        # Add some test aliases
        cmds.aliasAttr("testAlias1", f"{self.test_node}.testArray[0]")
        cmds.aliasAttr("testAlias2", f"{self.test_node}.testArray[1]")
        cmds.aliasAttr("singleAlias", f"{self.test_node}.singleAttr")

        # Test listing all aliases
        aliases = list(list_attr_aliases(self.test_node))
        self.assertEqual(len(aliases), 3)

        # Verify alias names and attributes
        alias_dict = dict(aliases)
        self.assertIn("testAlias1", alias_dict)
        self.assertIn("testAlias2", alias_dict)
        self.assertIn("singleAlias", alias_dict)
        self.assertEqual(alias_dict["testAlias1"], "testArray[0]")
        self.assertEqual(alias_dict["testAlias2"], "testArray[1]")
        self.assertEqual(alias_dict["singleAlias"], "singleAttr")

    def test_list_attr_aliases_specific_attr(self):
        """Test listing aliases for a specific attribute."""
        from ace_tools.utils import list_attr_aliases
        from maya import cmds, mel

        # Add some test aliases
        cmds.aliasAttr("arrayAlias1", f"{self.test_node}.testArray[0]")
        cmds.aliasAttr("arrayAlias2", f"{self.test_node}.testArray[1]")
        cmds.aliasAttr("singleAlias", f"{self.test_node}.singleAttr")

        # Test listing aliases for specific array attribute
        aliases = list(list_attr_aliases(self.test_node, "testArray"))
        self.assertEqual(len(aliases), 2)

        alias_dict = dict(aliases)
        self.assertIn("arrayAlias1", alias_dict)
        self.assertIn("arrayAlias2", alias_dict)
        self.assertNotIn("singleAlias", alias_dict)

    def test_list_attr_aliases_no_aliases(self):
        """Test listing aliases when none exist."""
        from ace_tools.utils import list_attr_aliases
        from maya import cmds, mel

        # Test with node that has no aliases
        aliases = list(list_attr_aliases(self.test_node))
        self.assertEqual(len(aliases), 0)

    def test_remove_attr_aliases_all(self):
        """Test removing all attribute aliases."""
        from ace_tools.utils import list_attr_aliases, remove_attr_aliases
        from maya import cmds, mel

        # Add some test aliases
        cmds.aliasAttr("testAlias1", f"{self.test_node}.testArray[0]")
        cmds.aliasAttr("testAlias2", f"{self.test_node}.testArray[1]")
        cmds.aliasAttr("singleAlias", f"{self.test_node}.singleAttr")

        # Verify aliases exist
        aliases_before = list(list_attr_aliases(self.test_node))
        self.assertEqual(len(aliases_before), 3)

        # Remove all aliases
        remove_attr_aliases(self.test_node)

        # Verify aliases are removed
        aliases_after = list(list_attr_aliases(self.test_node))
        self.assertEqual(len(aliases_after), 0)

    def test_remove_attr_aliases_with_prefix(self):
        """Test removing attribute aliases with specific prefix."""
        from ace_tools.utils import list_attr_aliases, remove_attr_aliases
        from maya import cmds, mel

        # Add some test aliases with different prefixes
        cmds.aliasAttr("prefix1_alias1", f"{self.test_node}.testArray[0]")
        cmds.aliasAttr("prefix1_alias2", f"{self.test_node}.testArray[1]")
        cmds.aliasAttr("prefix2_alias1", f"{self.test_node}.singleAttr")

        # Remove only aliases with "prefix1_"
        remove_attr_aliases(self.test_node, prefix="prefix1_")

        # Verify only prefix1 aliases are removed
        remaining_aliases = list(list_attr_aliases(self.test_node))
        self.assertEqual(len(remaining_aliases), 1)

        alias_dict = dict(remaining_aliases)
        self.assertIn("prefix2_alias1", alias_dict)
        self.assertNotIn("prefix1_alias1", alias_dict)
        self.assertNotIn("prefix1_alias2", alias_dict)

    def test_remove_attr_aliases_force_flag(self):
        """Test remove_attr_aliases with force flag."""
        from ace_tools.utils import list_attr_aliases, remove_attr_aliases
        from maya import cmds, mel

        # Add test aliases
        cmds.aliasAttr("forceAlias", f"{self.test_node}.testArray[0]")

        # Test with force=True
        remove_attr_aliases(self.test_node, force=True)

        # Verify alias is removed
        remaining_aliases = list(list_attr_aliases(self.test_node))
        self.assertEqual(len(remaining_aliases), 0)

    def test_remove_attr_aliases_no_aliases(self):
        """Test remove_attr_aliases when no aliases exist."""
        from ace_tools.utils import remove_attr_aliases
        from maya import cmds, mel

        # Test removing aliases from node with no aliases - should not error
        remove_attr_aliases(self.test_node)
        # If we get here without exception, the test passes


class TestAceToolsUtilsCacheArrayAliases(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def setUp(self):
        from maya import cmds, mel

        # Create a test node with some attributes
        self.test_node = cmds.createNode("transform", name="testNode")
        cmds.addAttr(
            self.test_node,
            longName="testArray",
            attributeType="double",
            multi=True,
            indexMatters=False,
        )
        cmds.addAttr(
            self.test_node,
            longName="anotherArray",
            attributeType="double",
            multi=True,
            indexMatters=False,
        )

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_cache_array_aliases(self):
        """Test basic functionality of cache_array_aliases."""
        from ace_tools.utils import cache_array_aliases
        from maya import cmds, mel

        # Add a test target array.
        cmds.aliasAttr("faceMultiplier_Blink_Left", f"{self.test_node}.testArray[0]")
        cmds.aliasAttr("faceMultiplier_Blink_Right", f"{self.test_node}.testArray[1]")
        cmds.aliasAttr("faceMultiplier_Smile_Full", f"{self.test_node}.testArray[2]")
        # This alias is for a different array - should be filtered out
        cmds.aliasAttr("otherPrefix_MouthOpen", f"{self.test_node}.anotherArray[0]")

        # Test with testArray - should only return aliases for this specific array
        result = cache_array_aliases(self.test_node, "testArray")

        # Verify the results structure and content
        self.assertEqual(len(result), 3)  # Should only include testArray aliases
        self.assertIn("blink_left", result)  # Prefix removed, lowercase
        self.assertIn("blink_right", result)  # Prefix removed, lowercase
        self.assertIn("smile_full", result)  # Prefix removed, lowercase

        # Check that the values are correct tuples (original_alias, array_element)
        self.assertEqual(result["blink_left"][0], "faceMultiplier_Blink_Left")
        self.assertEqual(result["blink_left"][1], "testArray[0]")
        self.assertEqual(result["blink_right"][0], "faceMultiplier_Blink_Right")
        self.assertEqual(result["blink_right"][1], "testArray[1]")
        self.assertEqual(result["smile_full"][0], "faceMultiplier_Smile_Full")
        self.assertEqual(result["smile_full"][1], "testArray[2]")

    def test_cache_array_aliases_case_insensitive(self):
        """Test case-insensitive behavior of cache_array_aliases."""
        from ace_tools.utils import cache_array_aliases
        from maya import cmds, mel

        cmds.aliasAttr("prefix_LeftMouth", f"{self.test_node}.testArray[0]")  # CamelCase
        cmds.aliasAttr("prefix_RightMouth", f"{self.test_node}.testArray[1]")  # CamelCase
        cmds.aliasAttr("prefix_UPPERLIP", f"{self.test_node}.testArray[2]")  # UPPERCASE

        result = cache_array_aliases(self.test_node, "testArray")

        # All keys should be lowercase for case-insensitive matching
        # This allows "LeftMouth", "leftmouth", "LEFTMOUTH" to all match "leftmouth"
        self.assertIn("leftmouth", result)
        self.assertIn("rightmouth", result)
        self.assertIn("upperlip", result)

        # Verify original aliases are preserved in values (not lowercased)
        # This is important for Maya commands that need the exact alias name
        self.assertEqual(result["leftmouth"][0], "prefix_LeftMouth")
        self.assertEqual(result["rightmouth"][0], "prefix_RightMouth")
        self.assertEqual(result["upperlip"][0], "prefix_UPPERLIP")

    def test_cache_array_aliases_no_aliases(self):
        """Test cache_array_aliases behavior when no aliases exist.

        This test verifies that the function gracefully handles the case where
        an array attribute exists but has no aliases defined. The function should
        return an empty dictionary rather than raising an error.
        """
        from ace_tools.utils import cache_array_aliases
        from maya import cmds, mel

        # Call the function on an attribute with no aliases
        result = cache_array_aliases(self.test_node, "emptyArray")

        # Should return empty dictionary when no aliases exist
        # This is the expected behavior for attributes without any aliases
        self.assertEqual(result, {})

    def test_cache_array_aliases_no_matching_attribute(self):
        """Test cache_array_aliases filtering behavior for specific attributes."""
        from ace_tools.utils import cache_array_aliases
        from maya import cmds, mel

        # Add aliases only for testArray - anotherArray will have no aliases
        cmds.aliasAttr("prefix_Test1", f"{self.test_node}.testArray[0]")
        cmds.aliasAttr("prefix_Test2", f"{self.test_node}.testArray[1]")

        # Query for anotherArray (which has no aliases) - should be filtered out
        result = cache_array_aliases(self.test_node, "anotherArray")

        # Should return empty dictionary because anotherArray has no aliases
        # This tests the filtering logic in list_attr_aliases function
        self.assertEqual(result, {})

    def test_cache_array_aliases_complex_prefixes(self):
        """Test cache_array_aliases with complex prefixes containing underscores."""
        from ace_tools.utils import cache_array_aliases
        from maya import cmds, mel

        # Add aliases with complex prefixes (multiple underscores)
        # These simulate real-world scenarios with hierarchical naming
        cmds.aliasAttr(
            "very_long_prefix_FinalName",
            f"{self.test_node}.testArray[0]",
        )  # Multi-level prefix
        cmds.aliasAttr(
            "another_complex_prefix_AnotherName",
            f"{self.test_node}.testArray[1]",
        )  # Multi-level prefix
        cmds.aliasAttr(
            "simple_Name",
            f"{self.test_node}.testArray[2]",
        )  # Single prefix

        result = cache_array_aliases(self.test_node, "testArray")

        # The function should split on first underscore only and take the rest
        # "very_long_prefix_FinalName" -> key becomes "long_prefix_finalname"
        # "another_complex_prefix_AnotherName" -> key becomes "complex_prefix_anothername"
        # "simple_Name" -> key becomes "name"
        self.assertIn("long_prefix_finalname", result)
        self.assertIn("complex_prefix_anothername", result)
        self.assertIn("name", result)

        # Verify the original aliases are preserved exactly as they were
        # This is crucial for Maya operations that need the exact alias name
        self.assertEqual(result["long_prefix_finalname"][0], "very_long_prefix_FinalName")
        self.assertEqual(
            result["complex_prefix_anothername"][0],
            "another_complex_prefix_AnotherName",
        )
        self.assertEqual(result["name"][0], "simple_Name")
