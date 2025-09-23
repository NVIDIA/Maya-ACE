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
import builtins
import json
import os
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, mock_open, patch

from common import TEST_DIR
from maya import cmds, standalone

_ORIGINAL_IMPORT = builtins.__import__


class TestExportA2FParameters(unittest.TestCase):
    """Test cases for the export_a2f_parameters function."""

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_ace")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.unloadPlugin("maya_ace", force=True)

    def setUp(self):
        """Set up test node."""
        from maya import cmds

        test_node_name = "test_export_ace_player"
        created = cmds.createNode("AceAnimationPlayer", name=test_node_name)
        self.node_name = created
        cmds.dgeval(f"{self.node_name}.status")

    def tearDown(self):
        """Clean up after each test."""
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def test_export_a2f_parameters_basic(self):
        """Test basic export_a2f_parameters functionality."""
        import os
        import tempfile

        from ace_tools.config import export_a2f_parameters

        # Create a temporary file path
        temp_file_handle, temp_file_path = tempfile.mkstemp(suffix=".json")
        os.close(temp_file_handle)
        os.chmod(temp_file_path, 0o644)  # Make it readable
        self.addCleanup(os.remove, temp_file_path)

        # Call the function
        result = export_a2f_parameters(self.node_name, temp_file_path)

        # Verify result structure
        self.assertIsInstance(result, dict)
        self.assertIn("a2e", result)
        self.assertIn("a2f", result)

        # Verify a2e section
        self.assertIsInstance(result["a2e"], dict)

        # Verify a2f section
        self.assertIsInstance(result["a2f"], dict)

    def test_export_a2f_parameters_with_capitalize(self):
        """Test export_a2f_parameters with capitalize_array_names=True."""
        import os
        import tempfile

        from ace_tools.config import export_a2f_parameters

        # Create a temporary file path
        temp_file_handle, temp_file_path = tempfile.mkstemp(suffix=".json")
        os.close(temp_file_handle)
        os.chmod(temp_file_path, 0o644)  # Make it readable
        self.addCleanup(os.remove, temp_file_path)

        # Call the function with capitalization enabled
        result = export_a2f_parameters(self.node_name, temp_file_path, capitalize_array_names=True)

        # Verify result structure is still correct
        self.assertIsInstance(result, dict)
        self.assertIn("a2e", result)
        self.assertIn("a2f", result)

    def test_export_a2f_parameters_invalid_node(self):
        """Test export_a2f_parameters with invalid node."""
        import os
        import tempfile

        from ace_tools.config import export_a2f_parameters

        # Create a temporary file path
        temp_file_handle, temp_file_path = tempfile.mkstemp(suffix=".json")
        os.close(temp_file_handle)
        os.chmod(temp_file_path, 0o644)  # Make it readable
        self.addCleanup(os.remove, temp_file_path)

        # Test with empty node name
        with self.assertRaises(Exception) as context:
            export_a2f_parameters("", temp_file_path)
        self.assertIn("Please specify a target ace player node", str(context.exception))

        # Test with None node name
        with self.assertRaises(Exception) as context:
            export_a2f_parameters(None, temp_file_path)
        self.assertIn("Please specify a target ace player node", str(context.exception))

    def test_export_a2f_parameters_invalid_file_path(self):
        """Test export_a2f_parameters with invalid file path."""
        from ace_tools.config import export_a2f_parameters

        # Test with empty file path
        with self.assertRaises(Exception) as context:
            export_a2f_parameters(self.node_name, "")
        self.assertIn("Please specify a path to export the config file", str(context.exception))

        # Test with None file path
        with self.assertRaises(Exception) as context:
            export_a2f_parameters(self.node_name, None)
        self.assertIn("Please specify a path to export the config file", str(context.exception))

        # Test with non-writable file path
        with self.assertRaises(Exception) as context:
            export_a2f_parameters(self.node_name, "/nonexistent/path/file.json")
        self.assertIn("Please specify a path to export the config file", str(context.exception))


class TestCollectConfigParameters(unittest.TestCase):
    """Test cases for the _collect_config_parameters function."""

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_ace")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.unloadPlugin("maya_ace", force=True)

    def setUp(self):
        """Set up test node."""
        from maya import cmds

        test_node_name = "test_collect_ace_player"
        created = cmds.createNode("AceAnimationPlayer", name=test_node_name)
        self.node_name = created
        cmds.dgeval(f"{self.node_name}.status")

    def tearDown(self):
        """Clean up after each test."""
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def test_collect_config_parameters_string_mapping(self):
        """Test _collect_config_parameters with string mapping."""
        from ace_tools.config import _collect_config_parameters
        from maya import cmds

        # Set up test attributes
        cmds.setAttr(f"{self.node_name}.emotionStrength", 0.75)
        cmds.setAttr(f"{self.node_name}.maxEmotions", 5)

        # Test mapping
        mapping = {"emotion_strength": "emotionStrength", "max_emotions": "maxEmotions"}

        results = list(_collect_config_parameters(self.node_name, mapping))

        # Verify results
        self.assertEqual(len(results), 2)
        results_dict = dict(results)

        self.assertAlmostEqual(results_dict["emotion_strength"], 0.75)
        self.assertEqual(results_dict["max_emotions"], 5)

    def test_collect_config_parameters_nested_dict_mapping(self):
        """Test _collect_config_parameters with nested dictionary mapping."""
        from ace_tools.config import _collect_config_parameters
        from maya import cmds

        # Set up test attributes
        cmds.setAttr(f"{self.node_name}.lowerFaceStrength", 1.5)
        cmds.setAttr(f"{self.node_name}.faceMaskLevel", 0.8)

        # Test nested mapping
        mapping = {
            "face_params": {
                "lower_face_strength": "lowerFaceStrength",
                "face_mask_level": "faceMaskLevel",
            }
        }

        results = list(_collect_config_parameters(self.node_name, mapping))

        # Verify results
        self.assertEqual(len(results), 1)
        face_params = results[0][1]  # Get the nested dictionary

        self.assertIsInstance(face_params, dict)
        self.assertAlmostEqual(face_params["lower_face_strength"], 1.5)
        self.assertAlmostEqual(face_params["face_mask_level"], 0.8)

    @patch("ace_tools.config.cache_array_aliases")
    def test_collect_config_parameters_array_mapping(self, mock_cache_aliases):
        """Test _collect_config_parameters with array mapping."""
        from ace_tools.config import _collect_config_parameters
        from maya import cmds

        # Mock the cache_array_aliases function
        mock_cache_aliases.return_value = {
            "blinkleft": ("faceMultiplier_BlinkLeft", "faceMultipliers[0]"),
            "browdownleft": ("faceMultiplier_BrowDownLeft", "faceMultipliers[1]"),
        }

        # Mock cmds.getAttr for the array elements
        def mock_getattr(attr):
            if attr.endswith("faceMultipliers[0]"):
                return 1.2
            elif attr.endswith("faceMultipliers[1]"):
                return 0.9
            return 0.0

        with patch("maya.cmds.getAttr", side_effect=mock_getattr):
            # Test array mapping
            mapping = {"weight_multipliers": ["faceMultipliers"]}

            results = list(_collect_config_parameters(self.node_name, mapping))

            # Verify results
            self.assertEqual(len(results), 1)
            key, array_params = results[0]

            self.assertEqual(key, "weight_multipliers")
            self.assertIsInstance(array_params, dict)

    def test_collect_config_parameters_nonexistent_attribute(self):
        """Test _collect_config_parameters with nonexistent attributes."""
        from ace_tools.config import _collect_config_parameters

        # Test mapping with nonexistent attribute
        mapping = {"nonexistent": "nonExistentAttribute"}

        # Should not raise exception, but should log error and not yield anything
        results = list(_collect_config_parameters(self.node_name, mapping))

        # Should get no results since the attribute doesn't exist
        self.assertEqual(len(results), 0)

    def test_collect_config_parameters_with_capitalize(self):
        """Test _collect_config_parameters with capitalize_array_names=True."""
        from ace_tools.config import _collect_config_parameters

        # Use a simple string mapping to test capitalization flag is passed through
        mapping = {"test_param": "emotionStrength"}

        # Should not raise exception when capitalize flag is used
        results = list(
            _collect_config_parameters(self.node_name, mapping, capitalize_array_names=True)
        )

        # Verify we get results
        self.assertEqual(len(results), 1)


class TestGetArrayParameterValues(unittest.TestCase):
    """Test cases for the _get_array_parameter_values function."""

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_ace")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.unloadPlugin("maya_ace", force=True)

    def setUp(self):
        """Set up test node."""
        from maya import cmds

        test_node_name = "test_array_ace_player"
        created = cmds.createNode("AceAnimationPlayer", name=test_node_name)
        self.node_name = created
        cmds.dgeval(f"{self.node_name}.status")

    def tearDown(self):
        """Clean up after each test."""
        from maya import cmds

        cmds.file(newFile=1, force=1)

    @patch("ace_tools.config.cache_array_aliases")
    def test_get_array_parameter_values_basic(self, mock_cache_aliases):
        """Test basic _get_array_parameter_values functionality."""
        from ace_tools.config import _get_array_parameter_values

        # Mock the cache_array_aliases function
        mock_cache_aliases.return_value = {
            "blinkleft": ("faceMultiplier_BlinkLeft", "faceMultipliers[0]"),
            "browdownleft": ("faceMultiplier_BrowDownLeft", "faceMultipliers[1]"),
        }

        # Mock cmds.getAttr for the array elements
        def mock_getattr(attr):
            if attr.endswith("faceMultipliers[0]"):
                return 1.2
            elif attr.endswith("faceMultipliers[1]"):
                return 0.9
            return 0.0

        with patch("maya.cmds.getAttr", side_effect=mock_getattr):
            results = list(_get_array_parameter_values(self.node_name, "faceMultipliers"))

            # Verify results
            self.assertEqual(len(results), 2)
            results_dict = dict(results)

            # Check that the function returns processed names directly as keys
            self.assertIn("BlinkLeft", results_dict)
            self.assertIn("BrowDownLeft", results_dict)

            # Verify values are returned directly (no nested structure)
            self.assertEqual(results_dict["BlinkLeft"], 1.2)
            self.assertEqual(results_dict["BrowDownLeft"], 0.9)

    @patch("ace_tools.config.cache_array_aliases")
    def test_get_array_parameter_values_with_capitalize(self, mock_cache_aliases):
        """Test _get_array_parameter_values with capitalize_array_names=True."""
        from ace_tools.config import _get_array_parameter_values

        # Mock the cache_array_aliases function with lowercase names
        mock_cache_aliases.return_value = {
            "blinkleft": ("faceMultiplier_blinkLeft", "faceMultipliers[0]"),
            "browdownright": ("faceMultiplier_browDownRight", "faceMultipliers[1]"),
        }

        # Mock cmds.getAttr
        def mock_getattr(attr):
            if attr.endswith("faceMultipliers[0]"):
                return 1.5
            elif attr.endswith("faceMultipliers[1]"):
                return 2.0
            return 0.0

        with patch("maya.cmds.getAttr", side_effect=mock_getattr):
            results = list(
                _get_array_parameter_values(
                    self.node_name, "faceMultipliers", capitalize_array_names=True
                )
            )

            # Verify results
            self.assertEqual(len(results), 2)
            results_dict = dict(results)

            # The function should return capitalized names directly as keys
            self.assertIn("BlinkLeft", results_dict)  # Capitalized
            self.assertIn("BrowDownRight", results_dict)  # Capitalized

            # Verify values are returned directly
            self.assertEqual(results_dict["BlinkLeft"], 1.5)
            self.assertEqual(results_dict["BrowDownRight"], 2.0)

    @patch("ace_tools.config.cache_array_aliases")
    def test_get_array_parameter_values_complex_names(self, mock_cache_aliases):
        """Test _get_array_parameter_values with complex names containing underscores."""
        from ace_tools.config import _get_array_parameter_values

        # Mock with complex names that contain underscores
        mock_cache_aliases.return_value = {
            "brow_down_left": ("faceMultiplier_Brow_Down_Left", "faceMultipliers[0]"),
            "eye_blink_left": ("faceMultiplier_Eye_Blink_Left", "faceMultipliers[1]"),
        }

        # Mock cmds.getAttr
        def mock_getattr(attr):
            if attr.endswith("faceMultipliers[0]"):
                return 2.0
            elif attr.endswith("faceMultipliers[1]"):
                return 1.8
            return 0.0

        with patch("maya.cmds.getAttr", side_effect=mock_getattr):
            results = list(_get_array_parameter_values(self.node_name, "faceMultipliers"))

            # Verify results
            self.assertEqual(len(results), 2)
            results_dict = dict(results)

            # Check that complex names are processed correctly (removing prefix)
            self.assertIn("Brow_Down_Left", results_dict)
            self.assertIn("Eye_Blink_Left", results_dict)

            # Verify values are returned directly
            self.assertEqual(results_dict["Brow_Down_Left"], 2.0)
            self.assertEqual(results_dict["Eye_Blink_Left"], 1.8)

    @patch("ace_tools.config.cache_array_aliases")
    def test_get_array_parameter_values_empty_aliases(self, mock_cache_aliases):
        """Test _get_array_parameter_values with empty aliases."""
        from ace_tools.config import _get_array_parameter_values

        # Mock with empty aliases
        mock_cache_aliases.return_value = {}

        results = list(_get_array_parameter_values(self.node_name, "faceMultipliers"))

        # Should return empty list
        self.assertEqual(len(results), 0)

    @patch("ace_tools.config.cache_array_aliases")
    def test_get_array_parameter_values_single_underscore_prefix(self, mock_cache_aliases):
        """Test _get_array_parameter_values with single underscore prefix."""
        from ace_tools.config import _get_array_parameter_values

        # Mock with simple prefix (single underscore split)
        mock_cache_aliases.return_value = {"test": ("simple_Test", "testArray[0]")}

        # Mock cmds.getAttr
        def mock_getattr(attr):
            if attr.endswith("testArray[0]"):
                return 3.0
            return 0.0

        with patch("maya.cmds.getAttr", side_effect=mock_getattr):
            results = list(_get_array_parameter_values(self.node_name, "testArray"))

            # Verify results
            self.assertEqual(len(results), 1)
            key, value = results[0]

            # The function returns the processed name directly as key (after removing prefix)
            self.assertEqual(key, "Test")
            self.assertEqual(value, 3.0)


class TestExportA2FParametersA2FPlayer(unittest.TestCase):
    """Test cases for the export_a2f_parameters function with A2FAnimationPlayer node."""

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
        """Set up test node."""
        from maya import cmds

        test_node_name = "test_export_a2f_player"
        created = cmds.createNode("A2FAnimationPlayer", name=test_node_name)
        self.node_name = created
        cmds.dgeval(f"{self.node_name}.outputWeights")

    def tearDown(self):
        """Clean up after each test."""
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def test_export_a2f_parameters_basic(self):
        """Test basic export_a2f_parameters functionality with A2FAnimationPlayer."""
        import os
        import tempfile

        from ace_tools.config import export_a2f_parameters

        # Create a temporary file path
        temp_file_handle, temp_file_path = tempfile.mkstemp(suffix=".json")
        os.close(temp_file_handle)
        os.chmod(temp_file_path, 0o644)  # Make it readable
        self.addCleanup(os.remove, temp_file_path)

        # Call the function
        result = export_a2f_parameters(self.node_name, temp_file_path)

        # Verify result structure
        self.assertIsInstance(result, dict)
        self.assertIn("a2e", result)
        self.assertIn("a2f", result)

        # Verify a2e section
        self.assertIsInstance(result["a2e"], dict)

        # Verify a2f section
        self.assertIsInstance(result["a2f"], dict)

        # Verify a2e section contents
        self.assertIn("post_processing_params", result["a2e"])
        self.assertIn("preferred_emotions", result["a2e"])

        # Verify emotion_params structure
        emotion_params = result["a2e"]["post_processing_params"]
        self.assertIsInstance(emotion_params, dict)

        # Verify preferred_emotion structure
        preferred_emotion = result["a2e"]["preferred_emotions"]
        self.assertIsInstance(preferred_emotion, dict)

        # Verify a2f section contents
        self.assertIn("blendshape_params", result["a2f"])
        self.assertIn("face_params", result["a2f"])
        self.assertIn("tongue_params", result["a2f"])

        # Verify blendshape_params structure
        blendshape_params = result["a2f"]["blendshape_params"]
        self.assertIn("weight_multipliers", blendshape_params)
        self.assertIn("weight_offsets", blendshape_params)


class TestAceImportExportConfigParameters(unittest.TestCase):
    """deprecated format. for backward compatibility"""

    @classmethod
    def setUpClass(cls):
        standalone.initialize(name="python")
        cmds.loadPlugin("maya_ace")

    @classmethod
    def tearDownClass(cls):
        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_ace", force=True)

    def setUp(self):
        from maya import cmds

        # create a test node
        test_node_name = "test_ace_player_node"
        created = cmds.createNode("AceAnimationPlayer", name=test_node_name)

        self.node_name = created
        cmds.dgeval(f"{self.node_name}.status")

    def tearDown(self):
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def test_export_blendshape_parameters(self):
        from ace_tools.names import ARKIT_FACE_EXPRESSIONS

        expectedBlendshapeMultiplers = []
        expectedBlendshapeOffsets = []
        for i, attrName in enumerate(ARKIT_FACE_EXPRESSIONS):
            expectedMultipler = 0.3 + (i / len(ARKIT_FACE_EXPRESSIONS)) * 0.1  # range: [0.3, 0.4)
            expectedOffset = 0.4 + (i / len(ARKIT_FACE_EXPRESSIONS)) * 0.1  # range: [0.4, 0.5)
            cmds.setAttr(f"{self.node_name}.faceMultiplier_{attrName}", expectedMultipler)
            cmds.setAttr(f"{self.node_name}.faceOffset_{attrName}", expectedOffset)
            expectedBlendshapeMultiplers.append(expectedMultipler)
            expectedBlendshapeOffsets.append(expectedOffset)

        data = self._export_and_read_parameters(self.node_name)

        for i in range(len(ARKIT_FACE_EXPRESSIONS)):
            self.assertAlmostEqual(
                expectedBlendshapeMultiplers[i], data["blendshape_params"]["bsWeightMultipliers"][i]
            )
            self.assertAlmostEqual(
                expectedBlendshapeOffsets[i], data["blendshape_params"]["bsWeightOffsets"][i]
            )

    def test_import_blendshape_parameters(self):
        from ace_tools import config
        from ace_tools.names import ARKIT_FACE_EXPRESSIONS

        # arrange
        expectedBlendshapeMultiplers = []
        expectedBlendshapeOffsets = []
        for i, attrName in enumerate(ARKIT_FACE_EXPRESSIONS):
            expectedMultipler = 0.3 + (i / len(ARKIT_FACE_EXPRESSIONS)) * 0.1  # range: [0.3, 0.4)
            expectedOffset = 0.4 + (i / len(ARKIT_FACE_EXPRESSIONS)) * 0.1  # range: [0.4, 0.5)
            cmds.setAttr(f"{self.node_name}.faceMultiplier_{attrName}", 0.1)
            cmds.setAttr(f"{self.node_name}.faceOffset_{attrName}", 0.2)
            expectedBlendshapeMultiplers.append(expectedMultipler)
            expectedBlendshapeOffsets.append(expectedOffset)
        exported_path = self._patch_and_export_params(
            {
                "blendshape_params": {
                    "bsWeightMultipliers": expectedBlendshapeMultiplers,
                    "bsWeightOffsets": expectedBlendshapeOffsets,
                },
                "blendshape_names": ARKIT_FACE_EXPRESSIONS,
            }
        )

        # act
        config.ace_import_config_parameters(self.node_name, exported_path)

        # assert
        for i, expression in enumerate(ARKIT_FACE_EXPRESSIONS):
            self.assertAlmostEqual(
                expectedBlendshapeMultiplers[i],
                cmds.getAttr(f"{self.node_name}.faceMultiplier_{expression}"),
            )
            self.assertAlmostEqual(
                expectedBlendshapeOffsets[i],
                cmds.getAttr(f"{self.node_name}.faceOffset_{expression}"),
            )

    def test_export_emotion_parameters(self):
        from ace_tools.names import A2F_EMOTION_NAMES

        expectedEmotion = []
        for i, attrName in enumerate(A2F_EMOTION_NAMES):
            expectedValue = 0.2 + (i / len(A2F_EMOTION_NAMES)) * 0.1  # range: [0.2, 0.3)
            cmds.setAttr(f"{self.node_name}.preferred_{attrName}", expectedValue)
            expectedEmotion.append(expectedValue)

        data = self._export_and_read_parameters(self.node_name)

        for i in range(len(A2F_EMOTION_NAMES)):
            self.assertAlmostEqual(expectedEmotion[i], data["preferred_emotion"][i])

    def test_import_preferred_emotion(self):
        from ace_tools import config
        from ace_tools.names import A2F_EMOTION_NAMES

        # arrange
        expectedEmotion = []
        for i, attrName in enumerate(A2F_EMOTION_NAMES):
            expectedValue = 0.2 + (i / len(A2F_EMOTION_NAMES)) * 0.1  # range: [0.2, 0.3)
            cmds.setAttr(
                f"{self.node_name}.preferredEmotions[0]", 0.1
            )  # deliberately set to a value different from the expected value
            expectedEmotion.append(expectedValue)
        exported_path = self._patch_and_export_params(
            {"preferred_emotion": expectedEmotion, "preferred_emotion_names": A2F_EMOTION_NAMES}
        )

        # act
        config.ace_import_config_parameters(self.node_name, exported_path)

        # assert
        for i, attrName in enumerate(A2F_EMOTION_NAMES):
            self.assertAlmostEqual(
                expectedEmotion[i], cmds.getAttr(f"{self.node_name}.preferred_{attrName}")
            )

    def test_import_emotion_parameters(self):
        from ace_tools import config

        # arrange
        test_params = {
            "emotion_strength": 0.314,
            "emotion_contrast": 0.314,
            "max_emotions": 3,
            "live_blend_coef": 0.714,
            "enable_preferred_emotion": True,
            "preferred_emotion_strength": 0.314,
        }
        exported_path = self._patch_and_export_params({"emotion_params": test_params})

        # act
        config.ace_import_config_parameters(self.node_name, exported_path)

        # assert
        self.assertAlmostEqual(cmds.getAttr(f"{self.node_name}.emotionStrength"), 0.314)
        self.assertAlmostEqual(cmds.getAttr(f"{self.node_name}.emotionContrast"), 0.314)
        self.assertAlmostEqual(cmds.getAttr(f"{self.node_name}.preferredEmotionStrength"), 0.314)
        self.assertAlmostEqual(cmds.getAttr(f"{self.node_name}.maxEmotions"), 3)
        self.assertAlmostEqual(cmds.getAttr(f"{self.node_name}.liveBlendCoef"), 0.714)
        self.assertAlmostEqual(cmds.getAttr(f"{self.node_name}.enablePreferredEmotion"), True)

    def test_export_face_parameters(self):
        from ace_tools.config import FACE_PARAM_MAPPING

        expectedFaceParam = {}
        for i, attrName in enumerate(FACE_PARAM_MAPPING):
            expectedValue = 0.1 + (i / len(FACE_PARAM_MAPPING)) * 0.1  # range: [0.1, 0.2)
            cmds.setAttr(f"{self.node_name}.{attrName}", expectedValue)
            expectedFaceParam[attrName] = expectedValue

        data = self._export_and_read_parameters(self.node_name)

        for attrName, jsonKey in FACE_PARAM_MAPPING.items():
            self.assertAlmostEqual(expectedFaceParam[attrName], data["face_params"][jsonKey])

    def test_import_face_parameters(self):
        from ace_tools.config import FACE_PARAM_MAPPING, ace_import_config_parameters

        # arrange
        expectedFaceParam = {}
        for i, (attrName, jsonKey) in enumerate(FACE_PARAM_MAPPING.items()):
            expectedValue = 0.1 + (i / len(FACE_PARAM_MAPPING)) * 0.1  # range: [0.1, 0.2)
            cmds.setAttr(
                f"{self.node_name}.{attrName}", 0.3
            )  # deliberately set to a value different from the expected value
            expectedFaceParam[jsonKey] = expectedValue
        exported_path = self._patch_and_export_params({"face_params": expectedFaceParam})

        # act
        ace_import_config_parameters(self.node_name, exported_path)

        # assert
        for attrName, jsonKey in FACE_PARAM_MAPPING.items():
            self.assertAlmostEqual(
                expectedFaceParam[jsonKey], cmds.getAttr(f"{self.node_name}.{attrName}")
            )

    def _export_and_read_parameters(self, test_node_name):
        # get a temp file path and export the configs
        temp_file_handle, temp_file_path = tempfile.mkstemp()
        os.close(temp_file_handle)
        from ace_tools.config import ace_export_config_parameters

        ace_export_config_parameters(test_node_name, temp_file_path)

        # load the exported params and verify the values are correctly saved
        with open(temp_file_path, "r") as json_file:
            data = json.load(json_file)
        self.addCleanup(os.remove, temp_file_path)

        return data

    def _patch_and_export_params(self, patch_dict):
        json_data = {
            "face_params": {},
            "preferred_emotion": {},
            "preferred_emotion_names": {},
            "blendshape_params": {
                "bsWeightMultipliers": {},
                "bsWeightOffsets": {},
            },
            "blendshape_names": {},
            "emotion_params": {},
        }
        json_data.update(patch_dict)

        temp_file_handle, temp_file_path = tempfile.mkstemp()
        os.close(temp_file_handle)
        self.addCleanup(os.remove, temp_file_path)
        with open(temp_file_path, "w") as json_file:
            json.dump(json_data, json_file, indent=2)

        return temp_file_path
