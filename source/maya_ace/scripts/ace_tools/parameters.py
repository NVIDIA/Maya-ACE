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

from types import MappingProxyType

# NOTE: the format is similar to the server's stylization_config file.
#    CAUTION: do not modify this in the code.
_A2E_PARAM_MAPPING_DICT = {
    # group key: "a2e"
    # config key: attribute name or [attribute names] for arrays
    "post_processing_params": MappingProxyType(
        {
            "emotion_contrast": "emotionContrast",
            "emotion_strength": "emotionStrength",
            "enable_preferred_emotion": "enablePreferredEmotion",
            "live_blend_coef": "liveBlendCoef",
            "max_emotions": "maxEmotions",
            "preferred_emotion_strength": "preferredEmotionStrength",
        }
    ),
    "preferred_emotions": ("preferredEmotions",),  # tuple is immutable
}
# Create a read-only mapping to prevent accidental modifications
A2E_PARAM_MAPPING = MappingProxyType(_A2E_PARAM_MAPPING_DICT)

# NOTE: the format is similar to the server's stylization_config file.
#    CAUTION: do not modify this in the code.
_A2F_PARAM_MAPPING_DICT = {
    # group key: "a2f"
    # config key: attribute name or [attribute names] for arrays
    "blendshape_params": MappingProxyType(
        {
            "weight_multipliers": ("faceMultipliers", "tongueMultipliers"),  # tuple is immutable
            "weight_offsets": ("faceOffsets", "tongueOffsets"),  # tuple is immutable
        }
    ),
    "face_params": MappingProxyType(
        {
            "eyelid_offset": "eyelidOpenOffset",
            "face_mask_level": "faceMaskLevel",
            "face_mask_softness": "faceMaskSoftness",
            #   "input_strength": "inputStrength",  # unused
            "lip_close_offset": "lipOpenOffset",
            "lower_face_smoothing": "lowerFaceSmoothing",
            "lower_face_strength": "lowerFaceStrength",
            "skin_strength": "skinStrength",
            "upper_face_smoothing": "upperFaceSmoothing",
            "upper_face_strength": "upperFaceStrength",
        }
    ),
    "tongue_params": MappingProxyType(
        {  # a2f player only
            "tongue_strength": "tongueStrength",
            "tongue_height_offset": "tongueHeightOffset",
            "tongue_depth_offset": "tongueDepthOffset",
        }
    ),
}
# Create a read-only mapping to prevent accidental modifications
A2F_PARAM_MAPPING = MappingProxyType(_A2F_PARAM_MAPPING_DICT)

# deprecated. read-only
FACE_PARAM_MAPPING = {
    # attribute name: config key
    "lowerFaceSmoothing": "lower_face_smoothing",
    "upperFaceSmoothing": "upper_face_smoothing",
    "lowerFaceStrength": "lower_face_strength",
    "upperFaceStrength": "upper_face_strength",
    "faceMaskLevel": "face_mask_level",
    "faceMaskSoftness": "face_mask_softness",
    "skinStrength": "skin_strength",
    "eyelidOpenOffset": "eyelid_offset",
    "lipOpenOffset": "lip_close_offset",
}

# deprecated. read-only
EMOTION_PARAM_MAPPING = {
    # attribute name: config key
    "emotionStrength": "emotion_strength",
    "emotionContrast": "emotion_contrast",
    "maxEmotions": "max_emotions",
    "liveBlendCoef": "live_blend_coef",
    "enablePreferredEmotion": "enable_preferred_emotion",
    "preferredEmotionStrength": "preferred_emotion_strength",
}
