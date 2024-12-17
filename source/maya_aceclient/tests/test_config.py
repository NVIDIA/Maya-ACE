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
import json
import os
import tempfile
import unittest

from maya import cmds, standalone


class TestAceImportExportConfigParameters(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        standalone.initialize(name="python")
        cmds.loadPlugin("maya_aceclient")

    @classmethod
    def tearDownClass(cls):
        cmds.file(newFile=1, force=1)
        cmds.unloadPlugin("maya_aceclient", force=True)

    def setUp(self):
        from maya import cmds

        # create a test node
        test_node_name = "test_ace_player_node"
        created = cmds.createNode("AceAnimationPlayer", name=test_node_name)

        self.node_name = created

    def tearDown(self):
        from maya import cmds

        cmds.file(newFile=1, force=1)

    def test_export_blendshape_parameters(self):
        from ace_tools import ARKIT_FACE_EXPRESSIONS

        expectedBlendshapeMultiplers = []
        expectedBlendshapeOffsets = []
        for i, attrName in enumerate(ARKIT_FACE_EXPRESSIONS):
            expectedMultipler = 0.3 + (i / len(ARKIT_FACE_EXPRESSIONS)) * 0.1  # range: [0.3, 0.4)
            expectedOffset = 0.4 + (i / len(ARKIT_FACE_EXPRESSIONS)) * 0.1  # range: [0.4, 0.5)
            cmds.setAttr(f"{self.node_name}.multiply_{attrName}", expectedMultipler)
            cmds.setAttr(f"{self.node_name}.offset_{attrName}", expectedOffset)
            expectedBlendshapeMultiplers.append(expectedMultipler)
            expectedBlendshapeOffsets.append(expectedOffset)

        data = self._export_and_read_parameters(self.node_name)

        for i in range(len(ARKIT_FACE_EXPRESSIONS)):
            self.assertAlmostEqual(
                expectedBlendshapeMultiplers[i],
                data["blendshape_params"]["bsWeightMultipliers"][i])
            self.assertAlmostEqual(
                expectedBlendshapeOffsets[i],
                data["blendshape_params"]["bsWeightOffsets"][i])

    def test_import_blendshape_parameters(self):
        from ace_tools import config, ARKIT_FACE_EXPRESSIONS

        # arrange
        expectedBlendshapeMultiplers = []
        expectedBlendshapeOffsets = []
        for i, attrName in enumerate(ARKIT_FACE_EXPRESSIONS):
            expectedMultipler = 0.3 + (i / len(ARKIT_FACE_EXPRESSIONS)) * 0.1  # range: [0.3, 0.4)
            expectedOffset = 0.4 + (i / len(ARKIT_FACE_EXPRESSIONS)) * 0.1  # range: [0.4, 0.5)
            cmds.setAttr(f"{self.node_name}.multiply_{attrName}", 0.1)
            cmds.setAttr(f"{self.node_name}.offset_{attrName}", 0.2)
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
                cmds.getAttr(f"{self.node_name}.multiply_{expression}")
            )
            self.assertAlmostEqual(
                expectedBlendshapeOffsets[i],
                cmds.getAttr(f"{self.node_name}.offset_{expression}"))

    def test_export_emotion_parameters(self):
        from ace_tools import EMOTION_NAMES

        expectedEmotion = []
        for i, attrName in enumerate(EMOTION_NAMES):
            expectedValue = 0.2 + (i / len(EMOTION_NAMES)) * 0.1  # range: [0.2, 0.3)
            cmds.setAttr(f"{self.node_name}.{attrName}", expectedValue)
            expectedEmotion.append(expectedValue)

        data = self._export_and_read_parameters(self.node_name)

        for i in range(len(EMOTION_NAMES)):
            self.assertAlmostEqual(expectedEmotion[i], data["preferred_emotion"][i])

    def test_import_preferred_emotion(self):
        from ace_tools import config, EMOTION_NAMES

        # arrange
        expectedEmotion = []
        for i, attrName in enumerate(EMOTION_NAMES):
            expectedValue = 0.2 + (i / len(EMOTION_NAMES)) * 0.1  # range: [0.2, 0.3)
            cmds.setAttr(
                f"{self.node_name}.{attrName}", 0.1
            )  # deliberately set to a value different from the expected value
            expectedEmotion.append(expectedValue)
        exported_path = self._patch_and_export_params(
            {"preferred_emotion": expectedEmotion, "preferred_emotion_names": EMOTION_NAMES}
        )

        # act
        config.ace_import_config_parameters(self.node_name, exported_path)

        # assert
        for i, attrName in enumerate(EMOTION_NAMES):
            self.assertAlmostEqual(
                expectedEmotion[i], cmds.getAttr(f"{self.node_name}.{attrName}"))

    def test_import_emotion_parameters(self):
        from ace_tools import config

        # arrange
        test_params = {
            "emotion_strength": 0.314,
            "emotion_contrast": 0.314,
            "max_emotions": 3,
            "live_blend_coef": 0.714,
            "enable_preferred_emotion": True,
            "preferred_emotion_strength": 0.314
        }
        exported_path = self._patch_and_export_params({"emotion_params": test_params})

        # act
        config.ace_import_config_parameters(self.node_name, exported_path)

        # assert
        self.assertAlmostEqual(cmds.getAttr(f"{self.node_name}.emotionStrength"), 0.314)
        self.assertAlmostEqual(cmds.getAttr(f"{self.node_name}.emotionContrast"), 0.314)
        self.assertAlmostEqual(cmds.getAttr(f"{self.node_name}.preferredEmotionStrength"), 0.314)
        self.assertAlmostEqual(cmds.getAttr(f"{self.node_name}.maxEmotion"), 3)
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
        from ace_tools.config import ace_import_config_parameters, FACE_PARAM_MAPPING

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
                expectedFaceParam[jsonKey], cmds.getAttr(f"{self.node_name}.{attrName}"))

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
