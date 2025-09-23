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


class TestPresets(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        from maya import cmds, standalone

        standalone.initialize(name="python")
        cmds.loadPlugin("maya_ace")

    def test_search_preset_files(self):
        """Search preset files from predefined locations."""
        from ace_tools import preset

        for filepath in preset.search_presets(directories=[os.path.join(TEST_DIR, "data")]):
            if "maya_ace" in filepath:
                self.assertIn("ace_presets.json", filepath)
                return
        else:
            self.assertTrue(False, msg="Cannot find a preset in search.")

    def test_read_preset_default_files(self):
        """Check reading default presets from ace_tools"""
        from ace_tools import preset

        data = preset.read_presets(directories=[os.path.join(TEST_DIR, "data")])

        default_presets = 0
        for entry in data:
            if "nvcf.nvidia.com" in entry.get("networkAddress", ""):
                default_presets += 1

        self.assertEqual(default_presets, 2)
