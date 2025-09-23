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
from maya import cmds, mel


def center_to_main_window(window, height_offset=0, width_offset=0):
    """Center the window on Maya's main window."""
    main_window = mel.eval("$tmpVar=$gMainWindow")

    main_topleft = cmds.window(main_window, query=1, topLeftCorner=1)
    main_height = cmds.window(main_window, query=1, height=1)
    main_width = cmds.window(main_window, query=1, width=1)

    self_topleft = cmds.window(window, query=1, topLeftCorner=1)
    self_height = cmds.window(window, query=1, height=1)
    self_width = cmds.window(window, query=1, width=1)

    new_top = main_topleft[0] + main_height // 2 - self_height // 2 + height_offset
    new_left = main_topleft[1] + main_width // 2 - self_width // 2 + width_offset

    cmds.window(window, edit=1, topLeftCorner=(new_top, new_left))
