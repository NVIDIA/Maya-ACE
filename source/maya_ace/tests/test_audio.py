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

from common import ROOT_DIR

TEST_AUDIO_1 = os.path.abspath(os.path.join(ROOT_DIR, "sample_data/audio_4sec_16k_s16le.wav"))
TEST_AUDIO_2 = os.path.abspath(os.path.join(ROOT_DIR, "sample_data/audio_6sec_48k_s16le.wav"))


class TestAceToolsTimeSliderAudio(unittest.TestCase):

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

        audio_nodes = cmds.file(
            TEST_AUDIO_1, type="audio", i=1, ignoreVersion=1, ra=1, returnNewNodes=1
        )
        self.assertIn("audio_4sec_16k_s16le", audio_nodes)

        self.audio = audio_nodes[0]

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_import_audio(self):
        from ace_tools.audio import import_audio
        from maya import cmds, mel

        imported = import_audio(TEST_AUDIO_2)
        self.assertEqual(imported, "audio_6sec_48k_s16le")

    def test_import_audio_with_time_slider(self):
        from ace_tools.audio import import_audio
        from maya import cmds, mel

        # Test importing audio with set_time_slider=True
        # This should work even in batch mode (should skip the time slider setting)
        imported = import_audio(TEST_AUDIO_2, set_time_slider=True)
        self.assertEqual(imported, "audio_6sec_48k_s16le")

    def test_find_audio(self):
        from ace_tools.audio import find_audio_node_from_path
        from maya import cmds, mel

        found = list(find_audio_node_from_path(TEST_AUDIO_1))

        self.assertIn("audio_4sec_16k_s16le", [n for n, p in found])


class TestAceToolsUtilsTimeSlider(unittest.TestCase):
    """Test time slider related functions."""

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

        # Create a test audio node
        audio_nodes = cmds.file(
            TEST_AUDIO_1, type="audio", i=1, ignoreVersion=1, ra=1, returnNewNodes=1
        )
        self.audio = audio_nodes[0]

    def tearDown(self):
        from maya import cmds, mel

        cmds.file(newFile=1, force=1)

    def test_get_time_slider_private_function(self):
        """Test the private _get_time_slider function."""
        from ace_tools.audio import _get_time_slider
        from maya import cmds

        # In batch mode, should return empty string
        if cmds.about(batch=True):
            result = _get_time_slider()
            self.assertEqual(result, "")
        else:
            # In interactive mode, should return time slider name
            result = _get_time_slider()
            self.assertIsInstance(result, str)

    def test_set_time_slider_audio_batch_mode(self):
        """Test set_time_slider_audio function in batch mode."""
        from ace_tools.audio import set_time_slider_audio
        from maya import cmds

        # In batch mode, this should handle gracefully
        result = set_time_slider_audio(self.audio)
        # The function should still return something, even if it can't set UI
        self.assertIsNotNone(result)

    def test_get_time_slider_audio_batch_mode(self):
        """Test get_time_slider_audio function in batch mode."""
        from ace_tools.audio import get_time_slider_audio
        from maya import cmds

        # In batch mode, this should handle gracefully
        result = get_time_slider_audio()
        # The function should return something, even if it can't query UI
        self.assertIsNotNone(result)

    def test_fit_time_range_to_audio_basic(self):
        """Test fit_time_range_to_audio with all attributes set."""
        from ace_tools.audio import fit_time_range_to_audio
        from maya import cmds

        # Set up audio attributes (duration is read-only and set by the audio file)
        cmds.setAttr(f"{self.audio}.offset", 10)
        cmds.setAttr(f"{self.audio}.sourceStart", 24)
        cmds.setAttr(f"{self.audio}.sourceEnd", 96)

        # Call the function
        result = fit_time_range_to_audio(self.audio)

        # Calculate expected values based on the new logic
        # start_frame = offset = 10
        # end_frame = offset + (source_end - 1) - source_start = 10 + (96 - 1) - 24 = 81
        self.assertEqual(result, (0, 10, 81, 81))

        # Verify playbackOptions were set correctly
        self.assertEqual(cmds.playbackOptions(q=True, min=True), 10)
        self.assertEqual(cmds.playbackOptions(q=True, max=True), 81)
        self.assertEqual(cmds.playbackOptions(q=True, animationStartTime=True), 0)
        self.assertEqual(cmds.playbackOptions(q=True, animationEndTime=True), 81)

    def test_fit_time_range_to_audio_zero_source_end(self):
        """Test fit_time_range_to_audio when sourceEnd is 0 and needs calculation."""
        from ace_tools.audio import fit_time_range_to_audio
        from maya import cmds

        # Set up audio attributes with sourceEnd = 0
        cmds.setAttr(f"{self.audio}.offset", 0)
        cmds.setAttr(f"{self.audio}.sourceStart", 10)
        cmds.setAttr(f"{self.audio}.sourceEnd", 0)  # This should trigger calculation

        # Get the actual duration from the audio file
        duration = cmds.getAttr(f"{self.audio}.duration")

        # Call the function
        result = fit_time_range_to_audio(self.audio)

        # When sourceEnd is 0, it's calculated as duration - sourceStart
        calculated_source_end = duration - 10
        # start_frame = offset = 0
        # end_frame = offset + (source_end - 1) - source_start = 0 + (calculated_source_end - 1) - 10
        expected_end_frame = int(0 + (calculated_source_end - 1) - 10)
        self.assertEqual(result, (0, 0, expected_end_frame, expected_end_frame))

        # Verify playbackOptions were set correctly
        self.assertEqual(cmds.playbackOptions(q=True, min=True), 0)
        self.assertEqual(cmds.playbackOptions(q=True, max=True), expected_end_frame)
        self.assertEqual(cmds.playbackOptions(q=True, animationStartTime=True), 0)
        self.assertEqual(cmds.playbackOptions(q=True, animationEndTime=True), expected_end_frame)

    def test_fit_time_range_to_audio_all_zeros(self):
        """Test fit_time_range_to_audio when all attributes are 0."""
        from ace_tools.audio import fit_time_range_to_audio
        from maya import cmds

        # Set all audio attributes to 0 (except duration which is read-only)
        cmds.setAttr(f"{self.audio}.offset", 0)
        cmds.setAttr(f"{self.audio}.sourceStart", 0)
        cmds.setAttr(f"{self.audio}.sourceEnd", 0)

        # Get the actual duration from the audio file
        duration = cmds.getAttr(f"{self.audio}.duration")

        # Call the function
        result = fit_time_range_to_audio(self.audio)

        # When sourceEnd is 0, it should be calculated as duration - sourceStart
        calculated_source_end = duration - 0
        # start_frame = offset = 0
        # end_frame = offset + (source_end - 1) - source_start = 0 + (calculated_source_end - 1) - 0
        expected_end_frame = int(calculated_source_end - 1)
        self.assertEqual(result, (0, 0, expected_end_frame, expected_end_frame))

        # Verify playbackOptions were set correctly
        self.assertEqual(cmds.playbackOptions(q=True, min=True), 0)
        self.assertEqual(cmds.playbackOptions(q=True, max=True), expected_end_frame)
        self.assertEqual(cmds.playbackOptions(q=True, animationStartTime=True), 0)
        self.assertEqual(cmds.playbackOptions(q=True, animationEndTime=True), expected_end_frame)

    def test_fit_time_range_to_audio_preserves_initial_playback_state(self):
        """Test that fit_time_range_to_audio properly sets playbackOptions regardless of initial state."""
        from ace_tools.audio import fit_time_range_to_audio
        from maya import cmds

        # Set initial playback options to different values
        cmds.playbackOptions(min=100, max=200, animationStartTime=150, animationEndTime=180)

        # Set up audio attributes (duration is read-only)
        cmds.setAttr(f"{self.audio}.offset", 5)
        cmds.setAttr(f"{self.audio}.sourceStart", 30)
        cmds.setAttr(f"{self.audio}.sourceEnd", 90)

        # Call the function
        result = fit_time_range_to_audio(self.audio)

        # Calculate expected values based on the new logic
        # start_frame = offset = 5
        # end_frame = offset + (source_end - 1) - source_start = 5 + (90 - 1) - 30 = 64
        self.assertEqual(result, (0, 5, 64, 64))

        # Verify playbackOptions were updated from initial values
        self.assertEqual(cmds.playbackOptions(q=True, min=True), 5)
        self.assertEqual(cmds.playbackOptions(q=True, max=True), 64)
        self.assertEqual(cmds.playbackOptions(q=True, animationStartTime=True), 0)
        self.assertEqual(cmds.playbackOptions(q=True, animationEndTime=True), 64)

    def test_fit_time_range_to_audio_handles_none_values(self):
        """Test fit_time_range_to_audio handles None values gracefully."""
        from ace_tools.audio import fit_time_range_to_audio
        from maya import cmds

        # Create a new audio node without setting any attributes
        # Maya typically returns 0 for unset numeric attributes, testing the 'or' logic
        new_audio = cmds.createNode("audio", name="testAudioNone")

        # Call the function with the new audio node
        result = fit_time_range_to_audio(new_audio)

        # With defaults: offset=0, sourceStart=0, sourceEnd=1 (due to 'or 1'), duration=0
        # start_frame = offset = 0
        # end_frame = offset + (source_end - 1) - source_start = 0 + (1 - 1) - 0 = 0
        self.assertEqual(result, (0, 0, 0, 0))

        # Cleanup
        cmds.delete(new_audio)

    def test_fit_time_range_to_audio_float_to_int_conversion(self):
        """Test that fit_time_range_to_audio properly converts float values to integers."""
        from ace_tools.audio import fit_time_range_to_audio
        from maya import cmds

        # Set up audio attributes with float values (duration is read-only)
        cmds.setAttr(f"{self.audio}.offset", 10.7)
        cmds.setAttr(f"{self.audio}.sourceStart", 24.3)
        cmds.setAttr(f"{self.audio}.sourceEnd", 96.8)

        # Call the function
        result = fit_time_range_to_audio(self.audio)

        # Calculate expected values with float to int conversion
        # start_frame = int(offset) = int(10.7) = 10
        # end_frame = int(offset + (source_end - 1) - source_start)
        #           = int(10.7 + (96.8 - 1) - 24.3) = int(82.2) = 82
        self.assertEqual(result, (0, 10, 82, 82))

        # Verify playbackOptions were set with integer values
        self.assertEqual(cmds.playbackOptions(q=True, min=True), 10)
        self.assertEqual(cmds.playbackOptions(q=True, max=True), 82)
        self.assertEqual(cmds.playbackOptions(q=True, animationStartTime=True), 0)
        self.assertEqual(cmds.playbackOptions(q=True, animationEndTime=True), 82)

    def test_fit_time_range_to_audio_with_offset(self):
        """Test fit_time_range_to_audio with various offset values."""
        from ace_tools.audio import fit_time_range_to_audio
        from maya import cmds

        # Test case 1: Positive offset (duration is read-only)
        cmds.setAttr(f"{self.audio}.offset", 50)
        cmds.setAttr(f"{self.audio}.sourceStart", 10)
        cmds.setAttr(f"{self.audio}.sourceEnd", 40)

        result = fit_time_range_to_audio(self.audio)

        # start_frame = offset = 50
        # end_frame = offset + (source_end - 1) - source_start = 50 + (40 - 1) - 10 = 79
        self.assertEqual(result, (0, 50, 79, 79))

        # Test case 2: Negative offset (Maya clamps negative min to 0)
        cmds.setAttr(f"{self.audio}.offset", -10)
        cmds.setAttr(f"{self.audio}.sourceStart", 0)
        cmds.setAttr(f"{self.audio}.sourceEnd", 30)

        result = fit_time_range_to_audio(self.audio)

        # start_frame = offset = -10
        # end_frame = offset + (source_end - 1) - source_start = -10 + (30 - 1) - 0 = 19
        self.assertEqual(result, (0, -10, 19, 19))

        # Maya clamps negative min values to 0
        self.assertEqual(cmds.playbackOptions(q=True, min=True), 0.0)
        self.assertEqual(cmds.playbackOptions(q=True, max=True), 19)
