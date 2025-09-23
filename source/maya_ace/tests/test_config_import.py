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


class TestReadConfigFile(unittest.TestCase):
    """Test cases for the _read_config_file function."""

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
        """Set up test data paths."""
        self.test_data_dir = os.path.join(TEST_DIR, "data")
        self.config_ace_player_old_path = os.path.join(
            self.test_data_dir, "config_ace_player_old.json"
        )
        self.config_nvcf13_james_path = os.path.join(self.test_data_dir, "config_nvcf13_james.json")
        self.config_nvcf13_james_yaml_path = os.path.join(
            self.test_data_dir, "config_nvcf13_james.yaml"
        )

    def test_read_valid_json_config_old(self):
        """Test reading the config_ace_player_old.json file."""
        from ace_tools.config import _read_config_file

        # Test that the function can read the old config format
        config_data = _read_config_file(self.config_ace_player_old_path)

        # Verify structure and some key values
        self.assertIsInstance(config_data, dict)
        self.assertIn("face_params", config_data)
        self.assertIn("emotion_params", config_data)

        # Verify specific values from the test file
        self.assertEqual(config_data["face_params"]["upper_face_smoothing"], 0.001)
        self.assertEqual(config_data["emotion_params"]["max_emotions"], 6)

    def test_read_valid_json_config_new(self):
        """Test reading the config_nvcf13_james.json file."""
        from ace_tools.config import _read_config_file

        # Test that the function can read the new config format
        config_data = _read_config_file(self.config_nvcf13_james_path)

        # Verify structure and some key values
        self.assertIsInstance(config_data, dict)
        self.assertIn("a2e", config_data)
        self.assertIn("a2f", config_data)

        # Verify a2e section
        self.assertTrue(config_data["a2e"]["enabled"])
        self.assertEqual(config_data["a2e"]["live_transition_time"], 0.5)
        self.assertIn("post_processing_params", config_data["a2e"])

        # Verify a2f section
        self.assertIn("blendshape_params", config_data["a2f"])
        self.assertIn("face_params", config_data["a2f"])

        # Verify specific face params
        self.assertEqual(config_data["a2f"]["face_params"]["lower_face_strength"], 1.2)

    def test_read_valid_yaml_config_new(self):
        """Test reading the config_nvcf13_james.yaml file."""
        from ace_tools.config import _read_config_file

        # Test that the function can read the new YAML config format
        config_data = _read_config_file(self.config_nvcf13_james_yaml_path)

        # Verify structure and some key values
        self.assertIsInstance(config_data, dict)
        self.assertIn("a2e", config_data)
        self.assertIn("a2f", config_data)

        # Verify a2e section
        self.assertEqual(config_data["a2e"]["live_transition_time"], 0.5)
        self.assertIn("post_processing_params", config_data["a2e"])

        # Verify a2f section
        self.assertIn("blendshape_params", config_data["a2f"])
        self.assertIn("face_params", config_data["a2f"])

        # Verify specific face params
        self.assertEqual(config_data["a2f"]["face_params"]["lower_face_strength"], 1.2)
        self.assertEqual(config_data["a2f"]["inference_model_id"], "james_v2.3")

        # Verify blendshape parameters structure
        self.assertIn("weight_multipliers", config_data["a2f"]["blendshape_params"])
        self.assertIn("weight_offsets", config_data["a2f"]["blendshape_params"])

    def test_read_json_when_yaml_not_available(self):
        """Test reading JSON when PyYAML is not available."""
        from ace_tools.config import _read_config_file

        def mock_import_side_effect(name, *args, **kwargs):
            if name == "yaml":
                raise ImportError("No module named 'yaml'")
            return _ORIGINAL_IMPORT(name, *args, **kwargs)

        # read file while yaml package is not available
        with patch("builtins.__import__", side_effect=mock_import_side_effect):
            # Should still be able to read JSON files when yaml import fails
            config_data = _read_config_file(self.config_ace_player_old_path)

            self.assertIsInstance(config_data, dict)
            self.assertIn("face_params", config_data)

            # read an invalid json file, and get an exception
            with tempfile.NamedTemporaryFile(mode="w", suffix=".json", delete=False) as f:
                f.write('{"invalid": json, content}')  # Invalid JSON syntax
                invalid_json_path = f.name
            try:
                with self.assertRaises(Exception) as context:
                    _read_config_file(invalid_json_path)
                self.assertIn("not a valid JSON file", str(context.exception))
            finally:
                os.unlink(invalid_json_path)

    def test_read_nonexistent_file(self):
        """Test handling of nonexistent files."""
        from ace_tools.config import _read_config_file

        nonexistent_path = "/path/that/does/not/exist.json"

        with self.assertRaises(FileNotFoundError):
            _read_config_file(nonexistent_path)

    @patch("yaml.load")
    @patch("builtins.open", new_callable=mock_open, read_data="valid: yaml\ncontent: true")
    def test_read_valid_yaml_file(self, mock_file, mock_yaml_load):
        """Test reading valid YAML files when PyYAML is available."""
        from ace_tools.config import _read_config_file

        # Mock yaml.load to return test data
        expected_data = {"valid": "yaml", "content": True}
        mock_yaml_load.return_value = expected_data

        config_data = _read_config_file("test.yaml")

        self.assertEqual(config_data, expected_data)
        mock_yaml_load.assert_called_once()

    @patch("yaml.load")
    @patch("builtins.open", new_callable=mock_open, read_data="invalid: yaml: content: :")
    def test_read_invalid_yaml_file(self, mock_file, mock_yaml_load):
        """Test handling invalid YAML files when PyYAML is available."""
        # Mock yaml.load to raise YAMLError
        import yaml
        from ace_tools.config import _read_config_file

        mock_yaml_load.side_effect = yaml.YAMLError("Invalid YAML")

        with self.assertRaises(Exception) as context:
            _read_config_file("invalid.yaml")

        self.assertIn("not a valid YAML file", str(context.exception))


class TestPopulateConfigAttributes(unittest.TestCase):
    """Test cases for the _populate_config_attributes function."""

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")

    def setUp(self):
        """Set up test data paths."""
        self.test_data_dir = os.path.join(TEST_DIR, "data")
        self.config_nvcf13_james_path = os.path.join(self.test_data_dir, "config_nvcf13_james.json")

    def test_basic_string_mapping(self):
        """Test basic string to string mapping."""
        from ace_tools.config import _populate_config_attributes

        config_data = {"emotion_strength": 0.6, "emotion_contrast": 1.0, "max_emotions": 3}

        mapping = {
            "emotion_strength": "emotionStrength",
            "emotion_contrast": "emotionContrast",
            "max_emotions": "maxEmotions",
        }

        results = list(_populate_config_attributes(config_data, mapping))
        expected = [("emotionStrength", 0.6), ("emotionContrast", 1.0), ("maxEmotions", 3)]

        self.assertEqual(len(results), 3)
        for expected_result in expected:
            self.assertIn(expected_result, results)

    def test_nested_dict_mapping(self):
        """Test nested dictionary mapping."""
        from ace_tools.config import _populate_config_attributes

        config_data = {
            "face_params": {
                "eyelid_offset": 0.06,
                "face_mask_level": 0.6,
                "lower_face_strength": 1.2,
            }
        }

        mapping = {
            "face_params": {
                "eyelid_offset": "eyelidOpenOffset",
                "face_mask_level": "faceMaskLevel",
                "lower_face_strength": "lowerFaceStrength",
            }
        }

        results = list(_populate_config_attributes(config_data, mapping))
        expected = [("eyelidOpenOffset", 0.06), ("faceMaskLevel", 0.6), ("lowerFaceStrength", 1.2)]

        self.assertEqual(len(results), 3)
        for expected_result in expected:
            self.assertIn(expected_result, results)

    def test_array_mapping(self):
        """Test array mapping where mapping value is a list."""
        from ace_tools.config import _populate_config_attributes

        config_data = {
            "weight_multipliers": {"BrowDownLeft": 1.0, "BrowDownRight": 0.8, "EyeBlinkLeft": 1.2}
        }

        mapping = {"weight_multipliers": ["faceMultipliers", "tongueMultipliers"]}

        results = list(_populate_config_attributes(config_data, mapping))
        expected_dict = {"BrowDownLeft": 1.0, "BrowDownRight": 0.8, "EyeBlinkLeft": 1.2}

        # Should yield the same dictionary for each attribute in the list
        expected = [("faceMultipliers", expected_dict), ("tongueMultipliers", expected_dict)]

        self.assertEqual(len(results), 2)
        for expected_result in expected:
            self.assertIn(expected_result, results)

    def test_skip_unmapped_keys(self):
        """Test that unmapped keys are skipped."""
        from ace_tools.config import _populate_config_attributes

        config_data = {
            "mapped_key": "mapped_value",
            "unmapped_key": "unmapped_value",
            "another_mapped": 42,
        }

        mapping = {
            "mapped_key": "mappedAttribute",
            "another_mapped": "anotherAttribute",
            # unmapped_key is intentionally not in mapping
        }

        results = list(_populate_config_attributes(config_data, mapping))
        expected = [("mappedAttribute", "mapped_value"), ("anotherAttribute", 42)]

        self.assertEqual(len(results), 2)
        for expected_result in expected:
            self.assertIn(expected_result, results)

        # Verify unmapped key is not present
        result_keys = [attr for attr, _ in results]
        self.assertNotIn("unmapped_key", result_keys)

    def test_complex_nested_structure(self):
        """Test complex nested structure similar to A2E mapping."""
        from ace_tools.config import _populate_config_attributes

        config_data = {
            "post_processing_params": {
                "emotion_contrast": 1.0,
                "emotion_strength": 0.6,
                "enable_preferred_emotion": True,
                "live_blend_coef": 0.7,
                "max_emotions": 3,
                "preferred_emotion_strength": 0.5,
            },
            "preferred_emotions": {"a": 0.1, "b": 0.2, "c": 0.3},
        }

        mapping = {
            "post_processing_params": {
                "emotion_contrast": "emotionContrast",
                "emotion_strength": "emotionStrength",
                "enable_preferred_emotion": "enablePreferredEmotion",
                "live_blend_coef": "liveBlendCoef",
                "max_emotions": "maxEmotions",
                "preferred_emotion_strength": "preferredEmotionStrength",
            },
            "preferred_emotions": ["preferredEmotions"],
        }

        results = list(_populate_config_attributes(config_data, mapping))

        # Check nested dict results
        nested_expected = [
            ("emotionContrast", 1.0),
            ("emotionStrength", 0.6),
            ("enablePreferredEmotion", True),
            ("liveBlendCoef", 0.7),
            ("maxEmotions", 3),
            ("preferredEmotionStrength", 0.5),
        ]

        # Check array result
        array_expected = [("preferredEmotions", {"a": 0.1, "b": 0.2, "c": 0.3})]

        expected = nested_expected + array_expected
        self.assertEqual(len(results), 7)

        for expected_result in expected:
            self.assertIn(expected_result, results)

    def test_with_a2f_param_mapping(self):
        """Test using the actual A2F_PARAM_MAPPING with real config data."""
        from ace_tools.config import _populate_config_attributes, _read_config_file
        from ace_tools.parameters import A2F_PARAM_MAPPING

        # Load the test config file
        config_data = _read_config_file(self.config_nvcf13_james_path)
        a2f_data = config_data.get("a2f", {})

        results = list(_populate_config_attributes(a2f_data, A2F_PARAM_MAPPING))

        # Verify we get results
        self.assertGreater(len(results), 0)

        # Check specific expected mappings from face_params
        face_param_results = [
            (attr, val)
            for attr, val in results
            if attr in ["eyelidOpenOffset", "faceMaskLevel", "lowerFaceStrength"]
        ]

        expected_face_params = [
            ("eyelidOpenOffset", 0.06),
            ("faceMaskLevel", 0.6),
            ("lowerFaceStrength", 1.2),
        ]

        for expected_param in expected_face_params:
            self.assertIn(expected_param, face_param_results)

        # Check that blendshape arrays are mapped correctly
        blendshape_results = [
            (attr, val)
            for attr, val in results
            if attr in ["faceMultipliers", "tongueMultipliers", "faceOffsets", "tongueOffsets"]
        ]

        # Should have 4 blendshape attribute mappings (2 for multipliers, 2 for offsets)
        self.assertEqual(len(blendshape_results), 4)

        # Each blendshape mapping should contain dictionary with ARKit expressions
        for attr, val in blendshape_results:
            self.assertIsInstance(val, dict)
            self.assertIn("BrowDownLeft", val)
            self.assertIn("EyeBlinkLeft", val)

    def test_with_a2e_param_mapping(self):
        """Test using the actual A2E_PARAM_MAPPING with real config data."""
        from ace_tools.config import _populate_config_attributes, _read_config_file
        from ace_tools.parameters import A2E_PARAM_MAPPING

        # Load the test config file
        config_data = _read_config_file(self.config_nvcf13_james_path)
        a2e_data = config_data.get("a2e", {})

        results = list(_populate_config_attributes(a2e_data, A2E_PARAM_MAPPING))

        # Verify we get results from post_processing_params
        self.assertGreater(len(results), 0)

        # Check specific expected mappings from post_processing_params
        expected_mappings = [
            ("emotionContrast", 1.0),
            ("emotionStrength", 0.6),
            ("enablePreferredEmotion", True),
            ("liveBlendCoef", 0.7),
            ("maxEmotions", 3),
            ("preferredEmotionStrength", 0.5),
        ]

        for expected_mapping in expected_mappings:
            self.assertIn(expected_mapping, results)

    def test_empty_config_data(self):
        """Test with empty config data."""
        from ace_tools.config import _populate_config_attributes

        config_data = {}
        mapping = {"key": "value"}

        results = list(_populate_config_attributes(config_data, mapping))
        self.assertEqual(len(results), 0)

    def test_empty_mapping(self):
        """Test with empty mapping."""
        from ace_tools.config import _populate_config_attributes

        config_data = {"key": "value"}
        mapping = {}

        results = list(_populate_config_attributes(config_data, mapping))
        self.assertEqual(len(results), 0)

    def test_mismatched_types(self):
        """Test behavior when config data and mapping types don't match."""
        from ace_tools.config import _populate_config_attributes

        # Case 1: mapping expects dict but config has non-dict value
        config_data = {"nested_key": "string_value"}
        mapping = {"nested_key": {"sub_key": "sub_attr"}}

        results = list(_populate_config_attributes(config_data, mapping))
        # Should skip this since types don't match
        self.assertEqual(len(results), 0)

        # Case 2: mapping is string but we have dict in config (should work)
        config_data = {"simple_key": {"complex": "value"}}
        mapping = {"simple_key": "simpleAttr"}

        results = list(_populate_config_attributes(config_data, mapping))
        expected = [("simpleAttr", {"complex": "value"})]
        self.assertEqual(results, expected)
