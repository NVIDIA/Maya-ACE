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
import re

SCRIPTS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ICON_DIR = os.path.join(SCRIPTS_DIR, "icon")

H_BTN = 19
W_BTN = 60
# H_OPTION = 20  # default value


def get_nice_name(name):
    """Convert a name to a nice ui name.

    Example:
        camelCase -> Camel Case
        PascalCase -> Pascal Case
        snake_case -> Snake Case

    Args:
        name (str): a name to convert

    Returns:
        (str) a nicely formatted name
    """
    # camel case to snake
    s1 = get_snake_case(name)
    return " ".join(word.title() for word in s1.split("_"))


def get_snake_case(name):
    """Convert a name to snake case

    Args:
        name (str): a name to convert

    Returns:
        (str) a snake_case name
    """
    # 0Aa -> 0_Aa
    s1 = re.sub("(.)([A-Z][a-z]+)", r"\1_\2", name)
    # __A -> _A
    s2 = re.sub("__([A-Z])", r"_\1", s1)
    # 0A -> 0_A
    s3 = re.sub("([a-z0-9])([A-Z])", r"\1_\2", s2)
    return s3.lower()
