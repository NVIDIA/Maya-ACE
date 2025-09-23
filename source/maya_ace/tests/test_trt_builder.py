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
import argparse
import json
import os
import sys
import unittest
from unittest.mock import MagicMock, mock_open, patch


class TestTrtBuilder(unittest.TestCase):
    """Test suite for trtgen.builder module."""

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

    @patch("json.load")
    @patch("os.path.realpath")
    @patch("os.path.isfile")
    @patch("builtins.open", new_callable=mock_open)
    def test_build_trt_command_with_trt_info(
        self, mock_file, mock_isfile, mock_realpath, mock_json_load
    ):
        """Test build_trt_command with trt_info.json file."""
        from trtgen import builder

        # Configure mock data
        trt_info_data = {
            "trt_build_param": {
                "batch": [
                    "--minShapes=input:1x{MIN_DURATION}x{FEATURE_SIZE}",
                    "--optShapes=input:{OPT_BATCH_SIZE}x{OPT_DURATION}x{FEATURE_SIZE}",
                    "--maxShapes=input:{MAX_BATCH_SIZE}x{MAX_DURATION}x{FEATURE_SIZE}",
                ]
            },
            "defaults": {
                "MIN_DURATION": 100,
                "OPT_DURATION": 500,
                "MAX_DURATION": 1000,
                "FEATURE_SIZE": 1024,
                "MAX_BATCH_SIZE": 1,
                "OPT_BATCH_SIZE": 1,
            },
        }
        mock_json_load.return_value = trt_info_data

        # Mock realpath to return predictable paths
        def realpath_side_effect(path):
            if "model.onnx" in path:
                return "/full/path/model.onnx"
            elif "trtexec" in path:
                return "/full/path/trtexec.exe"
            return path

        mock_realpath.side_effect = realpath_side_effect
        mock_isfile.return_value = True

        # Test the function
        cmd = builder.build_trt_command(
            "model.onnx", output_path="model.trt", trtexec_path="trtexec.exe"
        )

        # Verify the command
        expected_cmd = [
            "/full/path/trtexec.exe",
            "--onnx=/full/path/model.onnx",
            "--saveEngine=model.trt",
            "--minShapes=input:1x100x1024",
            "--optShapes=input:1x500x1024",
            "--maxShapes=input:1x1000x1024",
        ]
        self.assertEqual(cmd, expected_cmd)

        # Verify file was opened (normalize path for cross-platform)
        expected_path = os.path.join("/full/path", "trt_info.json")
        mock_file.assert_called_once_with(expected_path, "r")

    @patch("os.path.realpath")
    @patch("os.path.isfile")
    def test_build_trt_command_without_trt_info(self, mock_isfile, mock_realpath):
        """Test build_trt_command without trt_info.json file."""
        from trtgen import builder

        mock_realpath.side_effect = lambda x: f"/full/path/{x}"
        mock_isfile.return_value = False

        # Test the function
        cmd = builder.build_trt_command("model.onnx", user_params=["fp16", "verbose"])

        # Verify the command
        expected_cmd = [
            "/full/path/trtexec.exe",
            "--onnx=/full/path/model.onnx",
            "--saveEngine=/full/path/model.trt",
            "--fp16",
            "--verbose",
        ]
        self.assertEqual(cmd, expected_cmd)

    @patch("json.load")
    @patch("os.path.realpath")
    @patch("os.path.isfile")
    @patch("builtins.open", new_callable=mock_open)
    def test_build_trt_command_with_optimize_batch_size(
        self, mock_file, mock_isfile, mock_realpath, mock_json_load
    ):
        """Test build_trt_command with optimize_batch_size=True."""
        from trtgen import builder

        # Configure mock data
        trt_info_data = {
            "trt_build_param": {
                "batch": [
                    "--optShapes=input:{OPT_BATCH_SIZE}x{OPT_DURATION}",
                    "--maxShapes=input:{MAX_BATCH_SIZE}x{MAX_DURATION}",
                ]
            },
            "defaults": {
                "OPT_DURATION": 500,
                "MAX_DURATION": 1000,
                "MAX_BATCH_SIZE": 1,
                "OPT_BATCH_SIZE": 1,
            },
        }
        mock_json_load.return_value = trt_info_data

        mock_realpath.side_effect = lambda x: f"/full/path/{x}"
        mock_isfile.return_value = True

        # Test with optimize_batch_size=True
        cmd = builder.build_trt_command("model.onnx", optimize_batch_size=True)

        # Verify batch size was optimized
        self.assertIn("--optShapes=input:1x500", cmd[3])
        self.assertIn("--maxShapes=input:1x1000", cmd[4])

    @patch("json.load")
    @patch("os.path.realpath")
    @patch("os.path.isfile")
    @patch("builtins.open", new_callable=mock_open)
    def test_build_trt_command_with_user_defaults(
        self, mock_file, mock_isfile, mock_realpath, mock_json_load
    ):
        """Test build_trt_command with user_defaults parameter."""
        from trtgen import builder

        # Configure mock data
        trt_info_data = {
            "trt_build_param": {"batch": ["--shapes=input:{BATCH}x{DIM}"]},
            "defaults": {"BATCH": 1, "DIM": 256},
        }
        mock_json_load.return_value = trt_info_data

        mock_realpath.side_effect = lambda x: f"/full/path/{x}"
        mock_isfile.return_value = True

        # Test with custom user defaults
        user_defaults = {"BATCH": 4, "DIM": 512}
        cmd = builder.build_trt_command("model.onnx", user_defaults=user_defaults)

        # Verify user defaults were applied
        self.assertIn("--shapes=input:4x512", cmd[3])

    def test_get_parser(self):
        """Test get_parser function."""
        from trtgen import builder

        parser = builder.get_parser()

        # Test parser has required arguments
        self.assertIsInstance(parser, argparse.ArgumentParser)

        # Test parsing different commands
        # Test build command
        args = parser.parse_args(["build", "mark_v2.3", "--params", "fp16,verbose"])
        self.assertEqual(args.command, "build")
        self.assertEqual(args.model_name, "mark_v2.3")
        self.assertEqual(args.params, "fp16,verbose")

        # Test list command
        args = parser.parse_args(["list"])
        self.assertEqual(args.command, "list")

        # Test clean command
        args = parser.parse_args(["clean", "*.trt"])
        self.assertEqual(args.command, "clean")
        self.assertEqual(args.model_name, "*.trt")

    @patch("glob.glob")
    @patch("os.path.isdir")
    def test_list_file_paths(self, mock_isdir, mock_glob):
        """Test list_file_paths function."""
        from trtgen import builder

        # Setup mocks
        mock_isdir.return_value = True
        mock_glob.side_effect = [
            ["/models/mark_v2.3", "/models/claire_v2.3"],  # model dirs
            ["/models/mark_v2.3/network.onnx"],  # files in mark
            ["/models/claire_v2.3/network.onnx"],  # files in claire
        ]

        # Test the function
        results = list(builder.list_file_paths(["/models"], "*_v2.3", "network.onnx"))

        # Verify results
        self.assertEqual(len(results), 2)
        self.assertIn(os.path.realpath("/models/mark_v2.3/network.onnx"), results)
        self.assertIn(os.path.realpath("/models/claire_v2.3/network.onnx"), results)

    @patch("subprocess.run")
    @patch("os.path.exists")
    def test_build_trt_engines_success(self, mock_exists, mock_run):
        """Test build_trt_engines with successful builds."""
        from trtgen import builder

        # Mock subprocess run to succeed
        mock_run.return_value = MagicMock(returncode=0)
        mock_exists.return_value = True

        # Patch build_trt_command
        with patch.object(builder, "build_trt_command") as mock_build_cmd:
            mock_build_cmd.return_value = ["trtexec", "--onnx=model.onnx"]

            # Test the function
            model_paths = ["/path/model1.onnx", "/path/model2.onnx"]
            results = builder.build_trt_engines(model_paths, "trtexec")

        # Verify results
        self.assertEqual(len(results), 2)
        self.assertEqual(results, ["/path/model1.trt", "/path/model2.trt"])
        self.assertEqual(mock_run.call_count, 2)

    @patch("subprocess.run")
    @patch("os.path.exists")
    def test_build_trt_engines_failure(self, mock_exists, mock_run):
        """Test build_trt_engines with failed builds."""
        from trtgen import builder

        # Mock subprocess run to fail
        mock_run.return_value = MagicMock(returncode=1)
        mock_exists.return_value = False

        # Patch build_trt_command
        with patch.object(builder, "build_trt_command") as mock_build_cmd:
            mock_build_cmd.return_value = ["trtexec", "--onnx=model.onnx"]

            # Test the function
            model_paths = ["/path/model1.onnx"]
            results = builder.build_trt_engines(model_paths, "trtexec")

        # Verify no results for failed builds
        self.assertEqual(len(results), 0)

    @patch("os.remove")
    def test_clean_trt_files(self, mock_remove):
        """Test clean_trt_files function."""
        from trtgen import builder

        trt_paths = ["/path/model1.trt", "/path/model2.trt"]
        builder.clean_trt_files(trt_paths)

        # Verify files were removed
        self.assertEqual(mock_remove.call_count, 2)
        mock_remove.assert_any_call("/path/model1.trt")
        mock_remove.assert_any_call("/path/model2.trt")

    @patch("os.remove")
    def test_clean_trt_files_with_error(self, mock_remove):
        """Test clean_trt_files with OSError."""
        from trtgen import builder

        # Mock remove to raise OSError
        mock_remove.side_effect = OSError("Permission denied")

        # Should not raise exception
        trt_paths = ["/path/model1.trt"]
        builder.clean_trt_files(trt_paths)  # Should handle error gracefully

    @patch("os.getenv")
    @patch("os.path.exists")
    def test_find_trtexec_path_from_env(self, mock_exists, mock_getenv):
        """Test find_trtexec_path using environment variable."""
        from trtgen import builder

        # Mock environment variables
        mock_getenv.side_effect = lambda x: {
            "TRTEXEC_PATH": "/custom/path/trtexec.exe",
            "PATH": "/bin;/usr/bin",
        }.get(x)

        mock_exists.return_value = True

        result = builder.find_trtexec_path()

        # Should use TRTEXEC_PATH from environment
        self.assertIn("trtexec.exe", result)

    @patch("os.path.realpath")
    @patch("os.getenv")
    @patch("os.path.exists")
    def test_find_trtexec_path_in_system_path(self, mock_exists, mock_getenv, mock_realpath):
        """Test find_trtexec_path searching in system PATH."""
        from trtgen import builder

        # Mock environment variables
        mock_getenv.side_effect = lambda x: {
            "TRTEXEC_PATH": None,
            "PATH": "/bin;/usr/bin;/nvidia/bin",
        }.get(x)

        # Mock realpath to return the full path
        def realpath_side_effect(path):
            # Normalize path separators for testing
            return path.replace("\\", "/")

        mock_realpath.side_effect = realpath_side_effect

        # Only exists in /nvidia/bin
        def exists_side_effect(path):
            path_normalized = path.replace("\\", "/")
            return "/nvidia/bin" in path_normalized and "trtexec.exe" in path_normalized

        mock_exists.side_effect = exists_side_effect

        result = builder.find_trtexec_path()

        # Should find in system PATH (normalize path for cross-platform)
        result_normalized = result.replace("\\", "/")
        self.assertIn("/nvidia/bin", result_normalized)
        self.assertIn("trtexec.exe", result_normalized)

    @patch("os.getenv")
    @patch("os.path.exists")
    def test_find_trtexec_path_custom_name(self, mock_exists, mock_getenv):
        """Test find_trtexec_path with custom executable name."""
        from trtgen import builder

        mock_getenv.return_value = "/bin;/usr/bin"
        mock_exists.return_value = True

        result = builder.find_trtexec_path("custom_trtexec.exe")

        # Should use custom name
        self.assertIn("custom_trtexec.exe", result)

    @patch("sys.exit")
    @patch("os.path.exists")
    @patch("os.getenv")
    def test_main_trtexec_not_found(self, mock_getenv, mock_exists, mock_exit):
        """Test main function when trtexec is not found."""
        from trtgen import builder

        mock_getenv.return_value = ""
        mock_exists.return_value = False

        with patch.object(builder, "get_parser") as mock_parser:
            mock_args = MagicMock()
            mock_args.command = "build"
            mock_args.trtexec = None
            mock_args.model_dir = None
            mock_args.model_name = "*"

            mock_parser.return_value.parse_args.return_value = mock_args

            builder.main()

        # Should exit with error
        mock_exit.assert_called_once_with(1)

    @patch("sys.exit")
    @patch("os.path.exists")
    def test_main_build_command(self, mock_exists, mock_exit):
        """Test main function with build command."""
        from trtgen import builder

        mock_exists.return_value = True

        with patch.object(builder, "get_parser") as mock_parser:
            mock_args = MagicMock()
            mock_args.command = "build"
            mock_args.trtexec = "trtexec.exe"
            mock_args.model_dir = "/models"
            mock_args.model_name = "mark_v2.3"
            mock_args.params = "fp16,verbose"

            mock_parser.return_value.parse_args.return_value = mock_args

            with patch("trtgen.builder.list_local_a2f_models") as mock_list_a2f:
                with patch("trtgen.builder.list_local_a2e_models") as mock_list_a2e:
                    with patch.object(builder, "build_trt_engines") as mock_build:
                        mock_list_a2f.return_value = iter(["/models/mark_v2.3/network.onnx"])
                        mock_list_a2e.return_value = iter([])
                        mock_build.return_value = ["/models/mark_v2.3/network.trt"]

                        builder.main()

                        # Verify build was called
                        mock_build.assert_called_once()
                        self.assertFalse(mock_exit.called)

    @patch("sys.exit")
    @patch("os.path.exists")
    def test_main_clean_command(self, mock_exists, mock_exit):
        """Test main function with clean command."""
        from trtgen import builder

        mock_exists.return_value = True

        with patch.object(builder, "get_parser") as mock_parser:
            mock_args = MagicMock()
            mock_args.command = "clean"
            mock_args.trtexec = None
            mock_args.model_dir = "/models"
            mock_args.model_name = "*"

            mock_parser.return_value.parse_args.return_value = mock_args

            with patch("trtgen.builder.list_local_a2f_models") as mock_list_a2f:
                with patch("trtgen.builder.list_local_a2e_models") as mock_list_a2e:
                    with patch.object(builder, "clean_trt_files") as mock_clean:
                        mock_list_a2f.return_value = iter(["/models/mark_v2.3/network.trt"])
                        mock_list_a2e.return_value = iter([])

                        builder.main()

                        # Verify clean was called
                        mock_clean.assert_called_once_with(["/models/mark_v2.3/network.trt"])
                        self.assertFalse(mock_exit.called)
        pass

    @patch("sys.exit")
    @patch("os.path.exists")
    def test_main_list_command(self, mock_exists, mock_exit):
        """Test main function with list command."""
        from trtgen import builder

        mock_exists.return_value = True

        with patch.object(builder, "get_parser") as mock_parser:
            mock_args = MagicMock()
            mock_args.command = "list"
            mock_args.trtexec = None
            mock_args.model_dir = None
            mock_args.model_name = "*"

            mock_parser.return_value.parse_args.return_value = mock_args

            with patch("trtgen.builder.list_local_a2f_models") as mock_list_a2f:
                with patch("trtgen.builder.list_local_a2e_models") as mock_list_a2e:
                    mock_list_a2f.return_value = iter(["/models/mark_v2.3/network.onnx"])
                    mock_list_a2e.return_value = iter(["/models/a2e_v2.0/network.onnx"])

                    builder.main()

                    # Should just list files, not exit
                    self.assertFalse(mock_exit.called)
        pass
