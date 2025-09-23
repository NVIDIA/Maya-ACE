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
import subprocess
import threading
from typing import Any, Dict, List, Union

# PySide compatibility layer for Maya 2024 (PySide2) and Maya 2025 (PySide6)
try:
    from PySide6 import QtCore, QtGui, QtWidgets
    from PySide6.QtCore import Qt, Signal, Slot

    PYSIDE_VERSION = 6
except ImportError:
    from PySide2 import QtCore, QtGui, QtWidgets
    from PySide2.QtCore import Qt, Signal, Slot

    PYSIDE_VERSION = 2

from .builder import build_trt_command, find_trtexec_path
from .models import is_valid_model, list_local_a2e_models, list_local_a2f_models
from .process_ui import ProcessOutputDialog

_BUILDER_WINDOW = None


class TRTBuilderDialog(QtWidgets.QDialog):
    """
    Main application class for managing TRT files.
    """

    def __init__(self, parent=None):
        self.custom_a2f_model_paths = {}
        self.custom_a2e_model_paths = {}

        WINDOW_WIDTH = 1000
        WINDOW_HEIGHT = 500

        super().__init__(parent)
        self.setWindowTitle("TRT Manager")
        active_window = self.get_active_window()
        if active_window:
            x = active_window.x() + (active_window.width() - WINDOW_WIDTH) // 2
            y = active_window.y() + (active_window.height() - WINDOW_HEIGHT) // 2
            self.setGeometry(x, y, WINDOW_WIDTH, WINDOW_HEIGHT)
        else:
            self.setGeometry(100, 100, WINDOW_WIDTH, WINDOW_HEIGHT)

        # Set window flags for proper modal behavior in Maya
        self.setWindowModality(Qt.ApplicationModal)
        self.setWindowFlags(Qt.Window)

        # Ensure the dialog is raised and activated
        self.raise_()
        self.activateWindow()

        self.initUI()

    def get_active_window(self):
        """
        Get the Maya main window.
        """
        return QtWidgets.QApplication.activeWindow()

    def initUI(self):
        self.a2f_tree = QtWidgets.QTreeWidget(self)
        self.a2f_tree.setColumnCount(4)
        self.a2f_tree.setHeaderLabels(["Name", "TRT", "Model Path", "Notes"])
        self.a2f_tree.setSelectionMode(QtWidgets.QAbstractItemView.ExtendedSelection)

        self.a2e_tree = QtWidgets.QTreeWidget(self)
        self.a2e_tree.setColumnCount(4)
        self.a2e_tree.setHeaderLabels(["Name", "TRT", "Model Path", "Notes"])
        self.a2e_tree.setSelectionMode(QtWidgets.QAbstractItemView.ExtendedSelection)

        self.rebuild_button = QtWidgets.QPushButton("(Re)Build TRT Files", self)
        self.rebuild_button.clicked.connect(self.rebuild_trt)

        self.delete_button = QtWidgets.QPushButton("Delete TRT Files", self)
        self.delete_button.clicked.connect(self.delete_trt)

        self.show_dirs_button = QtWidgets.QPushButton("Show Model Directories", self)
        self.show_dirs_button.clicked.connect(self.show_model_directories)

        layout = QtWidgets.QVBoxLayout()

        # Create a horizontal layout for the label and button
        a2f_header_layout = QtWidgets.QHBoxLayout()
        a2f_header_layout.addWidget(QtWidgets.QLabel(""))
        a2f_header_layout.addStretch()
        a2f_header_layout.addWidget(self.show_dirs_button)

        layout.addLayout(a2f_header_layout)
        layout.addWidget(QtWidgets.QLabel("Audio2Face Models"))
        layout.addWidget(self.a2f_tree)
        layout.addWidget(QtWidgets.QLabel("Audio2Emotion Models"))
        layout.addWidget(self.a2e_tree)
        layout.addWidget(self.rebuild_button)
        layout.addWidget(self.delete_button)

        container = QtWidgets.QWidget()
        container.setLayout(layout)
        self.setLayout(layout)

        self.populate_trees()

    def showEvent(self, event):
        """Override showEvent to ensure the dialog stays on top when shown."""
        super().showEvent(event)
        # Force the window to stay on top and maintain focus
        self.raise_()
        self.activateWindow()
        # On Windows, we may need to force the window to the foreground
        if hasattr(self, "winId"):
            try:
                # This helps on Windows to truly bring the window to front
                import ctypes

                ctypes.windll.user32.SetForegroundWindow(int(self.winId()))
            except:
                pass

    def add_custom_a2f_model_path(self, model_path: str, notes: str = ""):
        """
        Add a custom A2F model path to the tree.
        This is useful for models that are not in the default model directories.

        Args:
            model_path (str): path to the model.
        """
        self.custom_a2f_model_paths[model_path] = notes

    def add_custom_a2e_model_path(self, model_path: str, notes: str = ""):
        """
        Add a custom A2E model path to the tree.
        This is useful for models that are not in the default model directories.

        Args:
            model_path (str): path to the model.
        """
        self.custom_a2e_model_paths[model_path] = notes

    def clear_custom_a2f_model_paths(self):
        self.custom_a2f_model_paths.clear()

    def clear_custom_a2e_model_paths(self):
        self.custom_a2e_model_paths.clear()

    def populate_trees(self):
        # a2f

        a2f_models = set()
        self.a2f_tree.clear()
        for model_path in list_local_a2f_models(skip_invalid=False):
            model_path_norm = os.path.normpath(model_path)
            model_path_dir = os.path.dirname(model_path_norm)
            self.add_model_to_tree(model_path_norm, self.a2f_tree)
            a2f_models.add(model_path_dir)

        for model_path, notes in self.custom_a2f_model_paths.items():
            # add custom models that are not in the default model directories
            model_path_norm = os.path.normpath(model_path)
            model_path_dir = os.path.dirname(model_path_norm)
            if model_path_dir in a2f_models:
                # skip if the model is already in the tree
                continue
            self.add_model_to_tree(model_path_norm, self.a2f_tree, notes=notes)

        # a2e
        a2e_models = set()
        self.a2e_tree.clear()
        for model_path in list_local_a2e_models(skip_invalid=False):
            model_path_norm = os.path.normpath(model_path)
            model_path_dir = os.path.dirname(model_path_norm)
            self.add_model_to_tree(model_path_norm, self.a2e_tree)
            a2e_models.add(model_path_dir)

        for model_path, notes in self.custom_a2e_model_paths.items():
            # add custom models that are not in the default model directories
            model_path_norm = os.path.normpath(model_path)
            model_path_dir = os.path.dirname(model_path_norm)
            if model_path_dir in a2e_models:
                # skip if the model is already in the tree
                continue
            self.add_model_to_tree(model_path_norm, self.a2e_tree, notes=notes)

        # fit the tree columns to the content
        self.a2f_tree.expandAll()
        self.a2e_tree.expandAll()
        for col in (0, 1, 2):
            column_width = max(
                self.a2f_tree.sizeHintForColumn(col), self.a2e_tree.sizeHintForColumn(col)
            )
            if col == 2:
                column_width = min(column_width, 600)
            self.a2f_tree.setColumnWidth(col, column_width + 10)
            self.a2e_tree.setColumnWidth(col, column_width + 10)

    def update_trt_statuses(self, model_name=""):
        if model_name:
            for item in self.a2f_tree.findItems(model_name, Qt.MatchExactly):
                self.update_model_trt_status(item)
            for item in self.a2e_tree.findItems(model_name, Qt.MatchExactly):
                self.update_model_trt_status(item)
        else:
            for idx in range(self.a2f_tree.topLevelItemCount()):
                item = self.a2f_tree.topLevelItem(idx)
                self.update_model_trt_status(item)
            for idx in range(self.a2e_tree.topLevelItemCount()):
                item = self.a2e_tree.topLevelItem(idx)
                self.update_model_trt_status(item)

    def get_model_dir(self, item: QtWidgets.QTreeWidgetItem):
        return item.text(2)

    def get_model_label(self, item: QtWidgets.QTreeWidgetItem):
        return item.text(0)

    def update_model_trt_status(self, item: QtWidgets.QTreeWidgetItem):
        model_dir = self.get_model_dir(item)
        item.setText(1, "Yes" if is_valid_model(model_dir + "/network.trt") else "No")

    def add_model_to_tree(self, model_path, tree, notes=""):
        """Add a model to the tree.

        Args:
            model_path (str): full path to the model card or trt file.
            tree (QTreeWidget): the tree to add the model to.
            notes (str): notes to display in the tree.

        Returns:
            (QTreeWidgetItem, str, str): the item, directory path, and version name.
        """
        dirname = os.path.dirname(model_path)
        vername = dirname.split(os.path.sep)[-1]
        trt_available = "Built" if is_valid_model(model_path) else "Missing"
        item = QtWidgets.QTreeWidgetItem([vername, trt_available, dirname, notes])
        tree.addTopLevelItem(item)
        return item, dirname, vername

    def rebuild_trt(self):
        # Logic to rebuild TRT files
        print("Rebuilding TRT files...")

        # get the selected models, so we don't worry about the tree changing
        selected_models = [self.get_model_dir(item) for item in self.get_selected_items()]

        for idx, model_name in enumerate(selected_models):
            print("  " + model_name)
            trtexec_path = find_trtexec_path()
            cmd = build_trt_command(
                model_name + "/network.onnx", trtexec_path=trtexec_path, optimize_batch_size=True
            )

            # Create and show the output dialog
            output_dialog = ProcessOutputDialog(self, f"Building TRT for {model_name}")
            output_dialog.show()

            # Run the command in the dialog
            output_dialog.run_command(cmd)

            # Wait for the process to complete
            while output_dialog.thread and output_dialog.thread.is_alive():
                QtWidgets.QApplication.processEvents()
                QtCore.QThread.msleep(100)  # Sleep for 100ms

            # Check if process succeeded
            if output_dialog.process and output_dialog.process.returncode != 0:
                # Process failed - wait for user to close dialog
                dialog_result = output_dialog.exec_()
                if not self._ask_continue_after_error(model_name):
                    print(f"  CANCELLED: User stopped TRT build process")
                    break
                else:
                    continue
            else:
                # Success - close dialog automatically after a brief delay
                QtCore.QTimer.singleShot(1000, output_dialog.accept)  # Close after 1 second
                output_dialog.exec_()  # This will return immediately after the timer fires

            # Update the tree widgets after each model is processed
            self.update_trt_statuses()
            QtWidgets.QApplication.processEvents()  # Ensure UI updates are processed

        self.populate_trees()

        try:
            from maya import cmds

            # refresh the editor templates to ensure the new TRT files are available
            cmds.refreshEditorTemplates()
        except ImportError:
            pass

    def _ask_continue_after_error(self, model_name):
        error_msg = (
            f"TRT build failed for {model_name}.\n\n"
            "Would you like to continue with the remaining models?"
        )
        if PYSIDE_VERSION == 6:
            button_yes = QtWidgets.QMessageBox.StandardButton.Yes
            button_no = QtWidgets.QMessageBox.StandardButton.No
        else:
            button_yes = QtWidgets.QMessageBox.Yes
            button_no = QtWidgets.QMessageBox.No
        reply = QtWidgets.QMessageBox.question(
            self, "TRT Build Error", error_msg, button_yes | button_no, button_yes
        )
        if reply != button_yes:
            return False
        return True

    def delete_trt(self):
        # Logic to delete selected TRT files
        print("Deleting selected TRT files...")
        for item in self.get_selected_items():
            model_dir = self.get_model_dir(item)
            print(f"Deleting TRT file at '{model_dir}'")
            path = os.path.join(model_dir, "network.trt")
            if os.path.exists(path):
                os.remove(path)
        self.populate_trees()
        try:
            from maya import cmds

            cmds.refreshEditorTemplates()
        except ImportError:
            pass

    def get_selected_items(self):
        return self.a2f_tree.selectedItems() + self.a2e_tree.selectedItems()

    def show_model_directories(self):
        """Show a dialog displaying the current model directories."""
        # Get the model directories from environment variables
        a2f_dirs = os.getenv("A2F_MODEL_DIRS", "").split(";")
        a2e_dirs = os.getenv("A2E_MODEL_DIRS", "").split(";")

        # Filter out empty strings
        a2f_dirs = [d for d in a2f_dirs if d]
        a2e_dirs = [d for d in a2e_dirs if d]

        # Create the message
        message = "Model Directories:\n"
        message += f"  Set the environment variables to customize directories.\n\n"

        message += "Audio2Face (A2F_MODEL_DIRS):\n"
        if a2f_dirs:
            for dir_path in a2f_dirs:
                message += f"- {dir_path}\n"
        else:
            message += "- (None configured)\n"

        message += "\nAudio2Emotion (A2E_MODEL_DIRS):\n"
        if a2e_dirs:
            for dir_path in a2e_dirs:
                message += f"- {dir_path}\n"
        else:
            message += "- (None configured)\n"

        # Create and show the dialog
        dialog = QtWidgets.QMessageBox(self)
        dialog.setWindowTitle("Model Directories")
        dialog.setText(message)
        dialog.setIcon(QtWidgets.QMessageBox.Information)
        dialog.setStandardButtons(QtWidgets.QMessageBox.Ok)
        dialog.setTextInteractionFlags(Qt.TextSelectableByMouse)
        dialog.exec_()


def show_trt_builder_dialog(nodes: Union[str, List[str]] = None):
    """Create a new TRT builder window.
        Deals with Maya/PySide2 environment.

    Args:
        nodes (str | list): a list of nodes to add to the builder window.
            If not provided, all A2FAnimationPlayer nodes will be used.

    Returns:
        (TRTBuilderDialog) the window instance.
    """
    # keep the window instance alive after the function returns
    global _BUILDER_WINDOW

    try:
        # When opening the window on Maya
        import maya.OpenMayaUI as omui
        from maya import cmds

        # Import the correct shiboken version based on PySide version
        if PYSIDE_VERSION == 6:
            from shiboken6 import wrapInstance  # PySide6
        else:
            from shiboken2 import wrapInstance  # PySide2

        # inject the project model directory to the environment variables
        project_dir = cmds.workspace(query=True, rootDirectory=True)
        a2f_model_dirs = os.getenv("A2F_MODEL_DIRS", "").split(";") or []
        a2e_model_dirs = os.getenv("A2E_MODEL_DIRS", "").split(";") or []
        project_a2f_dir = os.path.join(project_dir, "models", "audio2face-models")
        project_a2e_dir = os.path.join(project_dir, "models", "audio2emotion-models")
        if project_a2f_dir not in a2f_model_dirs:
            a2f_model_dirs += [project_a2f_dir]
        if project_a2e_dir not in a2e_model_dirs:
            a2e_model_dirs += [project_a2e_dir]
        os.environ["A2F_MODEL_DIRS"] = ";".join(a2f_model_dirs)
        os.environ["A2E_MODEL_DIRS"] = ";".join(a2e_model_dirs)

        # Get Maya's main window as parent if not provided
        maya_main_window_ptr = omui.MQtUtil.mainWindow()
        maya_main_window = wrapInstance(int(maya_main_window_ptr), QtWidgets.QWidget)
        _BUILDER_WINDOW = TRTBuilderDialog(parent=maya_main_window)

        # Add current model paths to the builder window
        a2f_nodes = []
        if nodes:
            a2f_nodes = cmds.ls(nodes) or []
        else:
            a2f_nodes = cmds.ls(type="A2FAnimationPlayer") or []

        for node in a2f_nodes:
            # get extra models from the current scene
            node_name = node.split("|")[-1]
            a2f_model_path = cmds.getAttr(f"{node}.a2fModelPath")
            a2e_model_path = cmds.getAttr(f"{node}.a2eModelPath")
            if a2f_model_path and os.path.exists(a2f_model_path):
                _BUILDER_WINDOW.add_custom_a2f_model_path(a2f_model_path, notes=node_name)
            if a2e_model_path and os.path.exists(a2e_model_path):
                _BUILDER_WINDOW.add_custom_a2e_model_path(a2e_model_path, notes=node_name)
        # update the tree with the current model paths
        _BUILDER_WINDOW.populate_trees()

    except ImportError:
        # outside of Maya environment
        _BUILDER_WINDOW = TRTBuilderDialog()

    # Use exec_() for proper modal behavior instead of open()
    _BUILDER_WINDOW.exec_()

    return _BUILDER_WINDOW


def main():
    import sys

    app = QtWidgets.QApplication(sys.argv)
    window = TRTBuilderDialog()
    window.exec_()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
