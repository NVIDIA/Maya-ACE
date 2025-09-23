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

# PySide compatibility layer for Maya 2024 (PySide2) and Maya 2025 (PySide6)
try:
    from PySide6 import QtCore, QtGui, QtWidgets
    from PySide6.QtCore import Qt, Signal, Slot

    PYSIDE_VERSION = 6
except ImportError:
    from PySide2 import QtCore, QtGui, QtWidgets
    from PySide2.QtCore import Qt, Signal, Slot

    PYSIDE_VERSION = 2


class ProcessOutputDialog(QtWidgets.QDialog):
    """Dialog to display realtime output from a subprocess."""

    # Signal to update text safely from thread
    update_text = Signal(str)
    process_finished = Signal(int)

    def __init__(self, parent=None, title="Process Output"):
        super().__init__(parent)
        self.setWindowTitle(title)
        self.setModal(True)
        self.resize(800, 600)

        # Create layout
        layout = QtWidgets.QVBoxLayout(self)

        # Create text widget for output
        self.output_text = QtWidgets.QTextEdit(self)
        self.output_text.setReadOnly(True)
        self.output_text.setFont(QtGui.QFont("Courier", 9))
        layout.addWidget(self.output_text)

        # Create button layout
        button_layout = QtWidgets.QHBoxLayout()

        # Add cancel button
        self.cancel_button = QtWidgets.QPushButton("Cancel", self)
        self.cancel_button.clicked.connect(self.cancel_process)
        button_layout.addWidget(self.cancel_button)

        # Add close button (disabled initially)
        self.close_button = QtWidgets.QPushButton("Close", self)
        self.close_button.setEnabled(False)
        self.close_button.clicked.connect(self.accept)
        button_layout.addWidget(self.close_button)

        layout.addLayout(button_layout)

        self.process = None
        self.thread = None
        self.cancelled = False

        # Connect signals
        self.update_text.connect(self.append_text)
        self.process_finished.connect(self.on_process_finished)

    @Slot(str)
    def append_text(self, text):
        """Append text to the output widget."""
        self.output_text.moveCursor(QtGui.QTextCursor.End)
        self.output_text.insertPlainText(text)
        self.output_text.moveCursor(QtGui.QTextCursor.End)

    @Slot(int)
    def on_process_finished(self, return_code):
        """Handle process completion."""
        self.cancel_button.setEnabled(False)
        self.close_button.setEnabled(True)
        if return_code == 0:
            self.append_text("\n\nProcess completed successfully!")
        else:
            self.append_text(f"\n\nProcess failed with return code: {return_code}")

    def run_command(self, cmd):
        """Run a command and display output in realtime."""
        self.cancelled = False
        self.thread = threading.Thread(target=self._run_process, args=(cmd,))
        self.thread.start()

    def _run_process(self, cmd):
        """Run the process in a separate thread."""
        try:
            self.process = subprocess.Popen(
                cmd,
                universal_newlines=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,  # Combine stderr with stdout
                bufsize=1,  # Line buffered
            )

            # Read output line by line
            for line in iter(self.process.stdout.readline, ""):
                if self.cancelled:
                    break
                if line:
                    self.update_text.emit(line)

            self.process.wait()
            self.process_finished.emit(self.process.returncode)

        except Exception as e:
            self.update_text.emit(f"\nError: {str(e)}\n")
            self.process_finished.emit(-1)

    def cancel_process(self):
        """Cancel the running process."""
        self.cancelled = True
        if self.process:
            self.process.terminate()
            self.append_text("\n\nProcess cancelled by user.\n")
        self.cancel_button.setEnabled(False)
        self.close_button.setEnabled(True)

    def closeEvent(self, event):
        """Handle dialog close event."""
        if self.process and self.process.poll() is None:
            # Process is still running
            if PYSIDE_VERSION == 6:
                button_yes = QtWidgets.QMessageBox.StandardButton.Yes
                button_no = QtWidgets.QMessageBox.StandardButton.No
            else:
                button_yes = QtWidgets.QMessageBox.Yes
                button_no = QtWidgets.QMessageBox.No
            reply = QtWidgets.QMessageBox.question(
                self,
                "Process Running",
                "The process is still running. Do you want to terminate it?",
                button_yes | button_no,
                button_no,
            )
            if reply == button_yes:
                self.cancel_process()
            else:
                event.ignore()
                return
        event.accept()
