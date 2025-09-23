# SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

import maya.cmds as cmds
import yaml


class Create_Batch_Dialog:

    UI_Elements = {}
    Audio_Files = []

    def __init__(self):
        self.create_ui()

    def create_ui(self):

        # Check if the window already exists and delete it
        if cmds.window("createBatchWindow", exists=True):
            cmds.deleteUI("createBatchWindow", window=True)

        self.UI_Elements["Main_Window"] = cmds.window(
            "createBatchWindow", title="Create Batch List", widthHeight=(474, 600), sizeable=False
        )
        main_layout = cmds.columnLayout(adjustableColumn=True, rowSpacing=4)
        cmds.text(label="")

        row_a1_layout = cmds.rowLayout(
            numberOfColumns=4,
            columnWidth4=(125, 279, 20, 20),
            adjustableColumn=1,
            columnAlign4=("center", "right", "left", "right"),
        )
        self.UI_Elements["Audio_Folder_Field"] = cmds.text(
            label="Audio Folder:", align="right", enable=True
        )
        self.UI_Elements["Audio_Folder_Filepath_Field"] = cmds.textField(
            placeholderText="", width=279, enable=True
        )
        self.UI_Elements["Audio_Folder_browser"] = cmds.symbolButton(
            image="fileOpen.png",
            enable=True,
            command=lambda *args: self.browse_folder(
                field_name="Audio_Folder_Filepath_Field", message="Select Audio Files Folder"
            ),
            width=20,
        )
        cmds.text(label="", width=20)
        cmds.setParent("..")

        row_a2_layout = cmds.rowLayout(
            numberOfColumns=4,
            columnWidth4=(125, 279, 20, 20),
            adjustableColumn=1,
            columnAlign4=("center", "right", "left", "right"),
        )
        self.UI_Elements["Parameters_Folder_Field"] = cmds.text(
            label="A2F Parameters Folder:", align="right", enable=True
        )
        self.UI_Elements["Parameters_Folder_Filepath_Field"] = cmds.textField(
            placeholderText="", width=279, enable=True
        )
        self.UI_Elements["Parameters_Folder_browser"] = cmds.symbolButton(
            image="fileOpen.png",
            enable=True,
            command=lambda *args: self.browse_folder(
                field_name="Parameters_Folder_Filepath_Field",
                message="Select A2F Parameters Folder",
            ),
            width=20,
        )
        cmds.text(label="", width=20)
        cmds.setParent("..")

        row1_layout = cmds.rowLayout(
            numberOfColumns=5,
            adjustableColumn=3,
            columnWidth5=(20, 100, 152, 20, 20),
            columnAlign5=("center", "center", "left", "right", "right"),
        )
        cmds.text(label="", width=20)
        self.UI_Elements["Add_Audio_File"] = cmds.button(
            label="Add Audio Clips", width=100, command=self.add_audio, enable=False
        )
        self.UI_Elements["Parameters_file"] = cmds.optionMenu(width=152, enable=False)
        cmds.menuItem(label="Choose Parameters File")
        self.UI_Elements["Remove_Audio_File"] = cmds.symbolButton(
            image="delete.png", enable=True, command=self.remove_selected, width=20
        )
        cmds.text(label="", width=20)
        cmds.setParent("..")

        row5_layout = cmds.rowLayout(
            numberOfColumns=3,
            columnWidth3=(20, 428, 20),
            columnAlign3=("center", "center", "center"),
        )
        cmds.text(label="")
        self.UI_Elements["Audio_Files_List"] = cmds.treeView(
            numberOfButtons=0, enableKeyboardFocus=True, width=428, height=400
        )
        cmds.text(label="")
        cmds.setParent("..")

        row6_layout = cmds.rowLayout(
            numberOfColumns=4,
            columnWidth4=(125, 279, 20, 20),
            adjustableColumn=1,
            columnAlign4=("center", "right", "left", "right"),
        )
        self.UI_Elements["Output_Folder_Label"] = cmds.text(
            label="Anims Output Folder:", align="right", enable=True
        )
        self.UI_Elements["Output_Folder_Path_Field"] = cmds.textField(
            placeholderText="Select Output Folder...", width=279, enable=True
        )
        self.UI_Elements["Output_Folder_Path_Browser"] = cmds.symbolButton(
            image="fileOpen.png",
            enable=True,
            command=lambda *args: self.browse_folder(
                field_name="Output_Folder_Path_Field", message="Select Animations Output Folder"
            ),
        )
        cmds.text(label="", width=20)
        cmds.setParent("..")

        row7_layout = cmds.rowLayout(
            numberOfColumns=4,
            columnWidth4=(20, 100, 150, 172),
            adjustableColumn=2,
            columnAlign4=("center", "right", "left", "right"),
        )
        cmds.text(label="")
        self.UI_Elements["Output_Format_Label"] = cmds.text(label="Export FBX for:", align="right")
        self.UI_Elements["Output_Format_Options"] = cmds.optionMenu(width=100)
        cmds.menuItem(label="Maya Yup")
        cmds.menuItem(label="Maya Zup")
        cmds.menuItem(label="Unreal")
        cmds.text(label="", width=172)
        cmds.setParent("..")

        row8_layout = cmds.rowLayout(
            numberOfColumns=4,
            columnWidth4=(10, 150, 150, 10),
            adjustableColumn=1,
            columnAlign4=("left", "right", "right", "right"),
        )
        cmds.text(label="")
        self.UI_Elements["Create_Batch_Button"] = cmds.button(
            label="Create Batch List", width=145, enable=False, command=self.create_batch
        )
        self.UI_Elements["Cancel_Button"] = cmds.button(
            label="Cancel", width=145, command=self.close_window
        )
        cmds.text(label="", width=10)
        cmds.setParent("..")

        # Show the window
        cmds.showWindow(self.UI_Elements["Main_Window"])

    def browse_folder(self, *args, field_name, message):
        OutputFolder = cmds.fileDialog2(dialogStyle=2, fileMode=3, caption=message)

        if OutputFolder != None:
            # set the field text to the selected folder
            cmds.textField(self.UI_Elements[field_name], edit=True, text=OutputFolder[0])

            # if the folder is the parameters folder, update the parameters file menu
            if field_name == "Parameters_Folder_Filepath_Field":
                cmds.optionMenu(self.UI_Elements["Parameters_file"], edit=True, deleteAllItems=True)

                # get the files in the parameters folder and build a list of A2F parameters files with the display name and type (json or yaml)
                files = os.listdir(OutputFolder[0])
                for filename in files:
                    if os.path.splitext(filename)[1] == ".json":
                        # check if it's an A2F parameters file
                        with open(OutputFolder[0] + "/" + filename, "r") as f:
                            data = json.load(f)
                            keys = list(data.keys())
                            if "face_params" in keys or "a2f" in keys:
                                cmds.menuItem(
                                    parent=self.UI_Elements["Parameters_file"], label=filename
                                )
                    if os.path.splitext(filename)[1] == ".yaml":
                        # check if it's an A2F parameters file
                        with open(OutputFolder[0] + "/" + filename, "r") as f:
                            data = yaml.safe_load(f)
                            keys = list(data.keys())
                            if "face_params" in keys or "a2f" in keys:
                                cmds.menuItem(
                                    parent=self.UI_Elements["Parameters_file"], label=filename
                                )

                labels = cmds.optionMenu(
                    self.UI_Elements["Parameters_file"], query=True, numberOfItems=True
                )

                if labels == 0:
                    cmds.optionMenu(self.UI_Elements["Parameters_file"], edit=True, enable=False)
                    cmds.menuItem(
                        parent=self.UI_Elements["Parameters_file"], label="Choose Parameters File"
                    )
                    cmds.button(self.UI_Elements["Add_Audio_File"], edit=True, enable=False)
                else:
                    cmds.optionMenu(self.UI_Elements["Parameters_file"], edit=True, enable=True)
                    if (
                        cmds.textField(
                            self.UI_Elements["Audio_Folder_Filepath_Field"], query=True, text=True
                        )
                        != ""
                    ):
                        cmds.button(self.UI_Elements["Add_Audio_File"], edit=True, enable=True)
                    else:
                        cmds.button(self.UI_Elements["Add_Audio_File"], edit=True, enable=False)

        self.check_batch_ready()

        return

    def add_audio(self, *args):

        Params = os.path.join(
            cmds.textField(
                self.UI_Elements["Parameters_Folder_Filepath_Field"], query=True, text=True
            ),
            cmds.optionMenu(self.UI_Elements["Parameters_file"], query=True, value=True),
        )
        ParamsDisplay = cmds.optionMenu(self.UI_Elements["Parameters_file"], query=True, value=True)

        # open dialog to allow load of multiple wav audio files
        # To restrict navigation, set the 'startingDirectory' to the desired folder and check if user selects files outside it.
        # For example, restrict to the folder in the audio field:
        audio_folder = cmds.textField(
            self.UI_Elements["Audio_Folder_Filepath_Field"], query=True, text=True
        )
        selected_audio_files = cmds.fileDialog2(
            fileMode=4,
            fileFilter="wav files (*.wav)",
            caption="Select wav audio files",
            okCaption="Add Audio Files",
            startingDirectory=audio_folder,
        )
        # Optionally, filter out files not in the allowed directory:
        if selected_audio_files:
            selected_audio_files = [
                f for f in selected_audio_files if os.path.dirname(f) == audio_folder
            ]

        # add audio files to the tree list with A2F parameters
        for a in selected_audio_files:
            IDAudio = len(self.Audio_Files) + 1
            # AudioEntry = {'id': IDAudio, 'Audio': a, 'Params': Params}
            # self.Audio_Files.append(AudioEntry)
            DisplayAudio = str(IDAudio) + " : " + os.path.basename(a)
            DisplayParams = str(IDAudio) + " : " + ParamsDisplay
            AudioEntry = {
                "id": IDAudio,
                "Audio": a,
                "DisplayAudio": DisplayAudio,
                "Params": Params,
                "DisplayParams": DisplayParams,
            }
            self.Audio_Files.append(AudioEntry)

        print(self.Audio_Files)
        self.update_batch_treeview()

        return

    def remove_selected(self, *args):

        # grab selected items in the tree view
        selecteditems = cmds.treeView(
            self.UI_Elements["Audio_Files_List"], query=True, selectItem=True
        )

        # define audio / params pairs selected in the tree
        if selecteditems:
            for a in selecteditems:
                id = int(a.split(" : ")[0])
                parent = cmds.treeView(
                    self.UI_Elements["Audio_Files_List"], query=True, itemParent=a
                )
                if parent == "":
                    params = cmds.treeView(
                        self.UI_Elements["Audio_Files_List"], query=True, children=a
                    )[1]
                    audio = a
                else:
                    params = a
                    audio = parent

                # remove the audio/params entry in the audio files list
                for b in self.Audio_Files:
                    if b.get("id") == id:
                        if b.get("DisplayAudio") == audio:
                            if b.get("DisplayParams") == params:
                                self.Audio_Files.remove(b)

            self.update_batch_treeview()

    def update_batch_treeview(self, *args):

        # update ids of audio files and display names from new id's
        counter = 1
        for a in self.Audio_Files:
            a["id"] = counter
            a["DisplayAudio"] = str(counter) + " : " + os.path.basename(a["Audio"])
            if a["Params"] != "Use Current A2F Parameters" and a["Params"] != "Defaults":
                a["ParamsDisplay"] = str(counter) + " : " + os.path.basename(a["Params"])
            else:
                a["DisplayParams"] = str(counter) + " : " + a["Params"]
            counter += 1

        # clear and update treeview
        cmds.treeView(self.UI_Elements["Audio_Files_List"], edit=True, removeAll=True)
        for a in self.Audio_Files:
            cmds.treeView(
                self.UI_Elements["Audio_Files_List"], edit=True, addItem=(a["DisplayAudio"], "")
            )
            cmds.treeView(
                self.UI_Elements["Audio_Files_List"],
                edit=True,
                addItem=(a["DisplayParams"], a["DisplayAudio"]),
            )

        self.check_batch_ready()

    def check_batch_ready(self, *args):

        if len(self.Audio_Files) > 0:
            if (
                cmds.textField(self.UI_Elements["Output_Folder_Path_Field"], query=True, text=True)
                != ""
            ):
                cmds.button(self.UI_Elements["Create_Batch_Button"], edit=True, enable=True)
            else:
                cmds.button(self.UI_Elements["Create_Batch_Button"], edit=True, enable=False)
        else:
            cmds.button(self.UI_Elements["Create_Batch_Button"], edit=True, enable=False)

        return

    def create_batch(self, *args):

        JsonBatchFile = cmds.fileDialog2(
            fileFilter="*.json", dialogStyle=2, fileMode=0, okCaption="Save Json File"
        )

        if JsonBatchFile != None:
            output_path = JsonBatchFile[0]
        else:
            cmds.warning("Please select a Json File")
            return

        audiofiles = []

        for a in self.Audio_Files:
            audio_name = os.path.basename(a["Audio"])
            params_name = os.path.basename(a["Params"])
            audiofiles.append([audio_name, params_name])

        batch_list_data = {
            "audiofiles_folder": cmds.textField(
                self.UI_Elements["Audio_Folder_Filepath_Field"], query=True, text=True
            ),
            "paramsfiles_folder": cmds.textField(
                self.UI_Elements["Parameters_Folder_Filepath_Field"], query=True, text=True
            ),
            "destination_folder": cmds.textField(
                self.UI_Elements["Output_Folder_Path_Field"], query=True, text=True
            ),
            "audiofiles": audiofiles,
            "FBX_axis_system": cmds.optionMenu(
                self.UI_Elements["Output_Format_Options"], query=True, value=True
            ),
        }

        with open(output_path, "w", encoding="utf-8") as f:
            json.dump(batch_list_data, f, indent=4, ensure_ascii=False)

        cmds.confirmDialog(
            title="Success", message=f"Batch List {output_path} created successfully.", button="OK"
        )

    def close_window(self, *args):
        cmds.deleteUI("createBatchWindow", window=True)


if __name__ == "__main__":
    BatchUIDialog = Create_Batch_Dialog()
