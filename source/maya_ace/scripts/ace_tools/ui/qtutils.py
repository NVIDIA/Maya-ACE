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


def set_geometry_to_active_window(window, width=600, height=450):
    """Set the geometry of the window to the active window."""
    active_window = get_active_window()
    if active_window:
        x = active_window.x() + (active_window.width() - width) // 2
        y = active_window.y() + (active_window.height() - height) // 2
        window.setGeometry(x, y, width, height)
    else:
        window.setGeometry(100, 100, width, height)


def get_active_window():
    """Get the active main window."""
    from PySide2 import QtWidgets

    return QtWidgets.QApplication.activeWindow()


def get_maya_main_window():
    """Get the Maya main window."""
    try:
        # Get Maya's main window as parent if not provided
        import maya.OpenMayaUI as omui
        from PySide2 import QtWidgets
        from shiboken2 import wrapInstance

        maya_main_window_ptr = omui.MQtUtil.mainWindow()
        maya_main_window = wrapInstance(int(maya_main_window_ptr), QtWidgets.QWidget)
        return maya_main_window
    except ImportError:
        return None
