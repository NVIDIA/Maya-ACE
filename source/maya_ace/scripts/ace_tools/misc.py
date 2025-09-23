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
import maya.cmds as cmds


def clean_invalid_plugins(delete_unknown_nodes=True):
    """Strips missing plug-in entries from the file and, optionally,
    deletes any nodes of type 'unknown' that those plug-ins left behind.

    Args:
        delete_unknown_nodes (bool): If True, all nodes whose type is 'unknown' are deleted.

    Returns:
        (None)
    """
    # 1. Find unresolved plug-ins
    missing = cmds.unknownPlugin(q=True, list=True) or []

    if not missing:
        cmds.warning("Scene has no invalid plug-in dependencies.")
    else:
        for plug in missing:
            print("#------------------------------------------")
            print("Removing missing plug-in: {0}".format(plug))
            cmds.unknownPlugin(plug, remove=True)  # drop the 'requires' entry
        print("#------------------------------------------\n")

    # 2. Delete orphaned unknown nodes
    if delete_unknown_nodes:
        unknown_nodes = cmds.ls(type="unknown") or []
        if unknown_nodes:
            print("Deleting {0} unknown node(s)...".format(len(unknown_nodes)))
            cmds.delete(unknown_nodes)
        else:
            print("No unknown nodes found.")

    print("\nDone. Save the scene to make the cleanup permanent.")
