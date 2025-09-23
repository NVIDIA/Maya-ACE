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
from unittest.mock import MagicMock, patch


class TestTrtModels(unittest.TestCase):
    """Test suite for trtgen.models module."""

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")

    @classmethod
    def tearDownClass(cls):
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def setUp(self):
        pass

    @patch("os.getenv")
    @patch("os.path.exists")
    def test_list_local_a2f_models_single_dir(self, mock_exists, mock_getenv):
        """Test listing A2F models from a single directory."""
        import fnmatch

        from trtgen import models

        # Mock environment variable
        model_dir = "/path/to/a2f/models"
        mock_getenv.return_value = model_dir

        # Mock find_all_files results
        mock_files = [
            "/path/to/a2f/models/multi_v3.1/network.onnx",
            # "/path/to/a2f/models/multi_v3.1/network.trt",
            "/path/to/a2f/models/james_v2.3/network.onnx",
            # "/path/to/a2f/models/james_v2.3/network.trt",
            "/path/to/a2f/models/a2f_no_trt_v1.0/network.onnx",
            # "/path/to/a2f/models/a2f_no_onnx_v1.0/network.trt",
        ]

        # Mock exists for network.trt files
        mock_exists.return_value = True

        # Patch find_all_files inside the test
        with patch.object(models, "find_all_files", return_value=iter(mock_files)) as mock_find_all:
            # Test the function
            results = list(models.list_local_a2f_models())
            mock_find_all.assert_called_once_with("*/*.onnx", model_dir)

        # Verify results
        self.assertEqual(len(results), 3)
        self.assertIn(os.path.normpath("/path/to/a2f/models/multi_v3.1/network.onnx"), results)
        self.assertIn(os.path.normpath("/path/to/a2f/models/james_v2.3/network.onnx"), results)

        # Verify environment variable was called correctly
        mock_getenv.assert_called_with("A2F_MODEL_DIRS", "")

    @patch("os.getenv")
    @patch("os.path.exists")
    def test_list_local_a2f_models_multiple_dirs(self, mock_exists, mock_getenv):
        """Test listing A2F models from multiple directories."""
        import fnmatch

        from trtgen import models

        # Mock environment variable with multiple paths
        model_dir = "/path/to/a2f/models;/another/path/models"
        mock_getenv.return_value = model_dir

        # Mock exists for network.trt files
        mock_files = [
            "/path/to/a2f/models/mark_v2.3/network.onnx",
            "/path/to/a2f/models/claire_v2.3/network.onnx",
            "/another/path/models/multi_v3.1/network.onnx",
            "/another/path/models/james_v2.3/network.onnx",
        ]

        # Mock exists for network.trt files
        mock_exists.return_value = True

        # Patch find_all_files inside the test
        with patch.object(models, "find_all_files", return_value=iter(mock_files)):
            # Test the function
            results = list(models.list_local_a2f_models())

        # Verify results from both directories
        self.assertEqual(len(results), 4)
        self.assertTrue(any("mark_v2.3" in r for r in results))
        self.assertTrue(any("multi_v3.1" in r for r in results))

    @patch("os.getenv")
    @patch("os.path.exists")
    def test_list_local_a2f_models_with_invalid(self, mock_exists, mock_getenv):
        """Test listing A2F models with invalid models (missing network.trt)."""
        from trtgen import models

        mock_getenv.return_value = "/path/to/a2f/models"

        # Mock find_all_files results including invalid model
        mock_files = [
            "/path/to/a2f/models/mark_v2.3/network.onnx",
            "/path/to/a2f/models/invalid_v1.0/network.onnx",
        ]

        # Mock exists - invalid model doesn't have network.trt
        def exists_side_effect(path):
            if "invalid_v1.0" in path and "network.trt" in path:
                return False
            elif "network.trt" in path:
                return True
            return False

        mock_exists.side_effect = exists_side_effect

        # Patch find_all_files inside the test
        with patch.object(models, "find_all_files", return_value=iter(mock_files)):
            # Test with skip_invalid=True (default)
            results = list(models.list_local_a2f_models(skip_invalid=True))
            self.assertEqual(len(results), 1)
            self.assertIn("mark_v2.3", results[0])

        # Patch find_all_files again for the second test
        with patch.object(models, "find_all_files", return_value=iter(mock_files)):
            # Test with skip_invalid=False
            results = list(models.list_local_a2f_models(skip_invalid=False))
            self.assertEqual(len(results), 2)

    @patch("os.getenv")
    def test_list_local_a2f_models_empty_env(self, mock_getenv):
        """Test listing A2F models with empty environment variable."""
        from trtgen import models

        mock_getenv.return_value = ""

        # Patch find_all_files to return empty iterator
        with patch.object(models, "find_all_files", return_value=iter([])):
            results = list(models.list_local_a2f_models())
            self.assertEqual(len(results), 0)

    @patch("os.getenv")
    @patch("os.path.exists")
    def test_list_local_a2e_models_single_dir(self, mock_exists, mock_getenv):
        """Test listing A2E models from a single directory."""
        import fnmatch

        from trtgen import models

        # Mock environment variable
        model_dir = "/path/to/a2e/models"
        mock_getenv.return_value = model_dir

        # Mock find_all_files results
        mock_files = [
            "/path/to/a2e/models/a2e_v2.2/network.onnx",
            # "/path/to/a2e/models/a2e_v2.2/network.trt",
            "/path/to/a2e/models/a2e_v1.0/network.onnx",
            # "/path/to/a2e/models/a2e_v1.0/network.trt",
        ]

        # Mock exists for network.trt files
        mock_exists.return_value = True

        # Patch find_all_files inside the test
        with patch.object(models, "find_all_files", return_value=iter(mock_files)) as mock_find_all:
            # Test the function
            results = list(models.list_local_a2e_models())
            mock_find_all.assert_called_once_with("*/*.onnx", model_dir)

        # Verify results
        self.assertEqual(len(results), 2)
        self.assertIn(os.path.normpath("/path/to/a2e/models/a2e_v2.2/network.onnx"), results)
        self.assertIn(os.path.normpath("/path/to/a2e/models/a2e_v1.0/network.onnx"), results)

        # Verify environment variable was called correctly
        mock_getenv.assert_called_with("A2E_MODEL_DIRS", "")

    @patch("os.getenv")
    @patch("os.path.exists")
    def test_list_local_a2e_models_with_invalid(self, mock_exists, mock_getenv):
        """Test listing A2E models with invalid models."""
        from trtgen import models

        mock_getenv.return_value = "/path/to/a2e/models"

        # Mock find_all_files results including invalid model
        mock_files = [
            "/path/to/a2e/models/a2e_v2.2/network.onnx",
            "/path/to/a2e/models/a2e_invalid_v0.5/network.onnx",
        ]

        # Mock exists - invalid model doesn't have network.trt
        def exists_side_effect(path):
            if "a2e_invalid_v0.5" in path and "network.trt" in path:
                return False
            elif "network.trt" in path:
                return True
            return False

        mock_exists.side_effect = exists_side_effect

        # Patch find_all_files inside the test
        with patch.object(models, "find_all_files", return_value=iter(mock_files)):
            # Test with skip_invalid=True (default)
            results = list(models.list_local_a2e_models(skip_invalid=True))
            self.assertEqual(len(results), 1)
            self.assertIn("a2e_v2.2", results[0])

    @patch("os.path.isfile")
    @patch("os.path.exists")
    def test_is_valid_model_with_file_path(self, mock_exists, mock_isfile):
        """Test is_valid_model with file path."""
        from trtgen import models

        # Mock that the input is a file
        mock_isfile.return_value = True
        # Test with valid model (has network.trt)
        mock_exists.return_value = True
        self.assertTrue(models.is_valid_model("/path/to/model/mark_v2.3/network.onnx"))

        # Verify it checked for network.trt in the directory
        expected_path = os.path.join("/path/to/model/mark_v2.3", "network.trt")
        mock_exists.assert_called_with(expected_path)

    @patch("os.path.exists")
    def test_is_valid_model_with_dir_path(self, mock_exists):
        """Test is_valid_model with directory path."""
        from trtgen import models

        # Test with valid model directory
        mock_exists.return_value = True
        self.assertTrue(models.is_valid_model("/path/to/model/mark_v2.3"))

        # Verify it checked for network.trt
        expected_path = os.path.join("/path/to/model/mark_v2.3", "network.trt")
        mock_exists.assert_called_with(expected_path)

    @patch("os.path.exists")
    def test_is_valid_model_invalid(self, mock_exists):
        """Test is_valid_model with invalid model."""
        from trtgen import models

        # Test with invalid model (no network.trt)
        mock_exists.return_value = False
        self.assertFalse(models.is_valid_model("/path/to/model/invalid_v1.0"))

    @patch("glob.glob")
    def test_find_all_files_single_directory(self, mock_glob):
        """Test find_all_files with single directory."""
        from trtgen import models

        mock_glob.return_value = [
            "/path/to/models/mark_v2.3/network.onnx",
            "/path/to/models/claire_v2.3/network.onnx",
        ]

        results = list(models.find_all_files("*_v*/*.onnx", "/path/to/models"))

        self.assertEqual(len(results), 2)
        # Use os.path.join to handle path separators correctly
        expected_pattern = os.path.join("/path/to/models", "*_v*/*.onnx")
        mock_glob.assert_called_once_with(expected_pattern)

    @patch("glob.glob")
    def test_find_all_files_multiple_directories(self, mock_glob):
        """Test find_all_files with multiple directories."""
        from trtgen import models

        def glob_side_effect(pattern):
            # Normalize path separators for cross-platform compatibility
            pattern_normalized = pattern.replace("\\", "/")
            if "path1" in pattern_normalized:
                return ["/path1/mark_v2.3/network.onnx"]
            elif "path2" in pattern_normalized:
                return ["/path2/claire_v2.3/network.onnx"]
            return []

        mock_glob.side_effect = glob_side_effect

        results = list(models.find_all_files("*_v*/*.onnx", "/path1", "/path2"))

        self.assertEqual(len(results), 2)
        self.assertEqual(mock_glob.call_count, 2)

    @patch("glob.glob")
    def test_find_all_files_with_none_directory(self, mock_glob):
        """Test find_all_files with None in directories list."""
        from trtgen import models

        mock_glob.return_value = ["/path/to/models/mark_v2.3/network.onnx"]

        # Should skip None directories
        results = list(models.find_all_files("*.onnx", None, "/path/to/models", None))

        self.assertEqual(len(results), 1)
        # Should only be called once for the valid directory
        expected_pattern = os.path.join("/path/to/models", "*.onnx")
        mock_glob.assert_called_once_with(expected_pattern)

    @patch("glob.glob")
    def test_find_all_files_empty_results(self, mock_glob):
        """Test find_all_files with no matching files."""
        from trtgen import models

        mock_glob.return_value = []

        results = list(models.find_all_files("*.nonexistent", "/path/to/models"))

        self.assertEqual(len(results), 0)

    @patch("os.getenv")
    @patch("os.path.exists")
    def test_integration_a2f_and_a2e_models(self, mock_exists, mock_getenv):
        """Integration test for both A2F and A2E model listing."""
        from trtgen import models

        # Set up environment variables
        def getenv_side_effect(var, default=""):
            if var == "A2F_MODEL_DIRS":
                return "/path/to/a2f/models"
            elif var == "A2E_MODEL_DIRS":
                return "/path/to/a2e/models"
            return default

        mock_getenv.side_effect = getenv_side_effect

        # Mock exists to always return True
        mock_exists.return_value = True

        # Mock find_all_files for A2F models
        a2f_mock_files = [
            "/path/to/a2f/models/mark_v2.3/network.onnx",
            "/path/to/a2f/models/claire_v2.3/network.onnx",
        ]

        # Mock find_all_files for A2E models
        a2e_mock_files = [
            "/path/to/a2e/models/a2e_v2.2/network.onnx",
        ]

        # Test A2F models
        with patch.object(models, "find_all_files", return_value=iter(a2f_mock_files)):
            a2f_results = list(models.list_local_a2f_models())
            self.assertEqual(len(a2f_results), 2)

        # Test A2E models
        with patch.object(models, "find_all_files", return_value=iter(a2e_mock_files)):
            a2e_results = list(models.list_local_a2e_models())
            self.assertEqual(len(a2e_results), 1)
