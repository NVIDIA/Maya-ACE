# Using the sample batch_a2f.py application

This application is a sample Python3 application to send audio and receive
animation data and emotion data through the A2F pipeline.

## ⚠️ Important Message

> **Note**: This application requires a running A2F microservice instance to function properly.
> Make sure your A2F microservice is running before executing this script.
>
> **Warning**: Ensure you have the necessary audio files and configuration files ready 
> before running the batch process. The script will process all audio files specified 
> in your BatchList.json configuration.

## Prerequisites

This application requires the following dependencies:

- Python 3.10

You will need to provide an audio file to test out.

You will need to have a running instance of A2F microservice.

## Setting up the environment

Start by creating a Python venv:

```bash
py -m venv .venv
.venv/Scripts/activate.bat
```

Download and install the python FBX SDK:

- [FBX SDK](https://aps.autodesk.com/developer/overview/fbx-sdk) -> look for the Python SDK at the bottom of the page
- Run the whl from the installation folder -> `c:/Program Files/Autodesk/FBX/FBX Python SDK/2020.3.7/fbx-2020.3.7-cp310-none-win_amd64.whl`

Install the gRPC proto for Python by:

- Quick installation: Install the provided `nvidia_ace` python wheel package from the
[sample_wheel/](../proto/sample_wheel) folder.

  ```bash
  pip install ../proto/sample_wheel/nvidia_ace-1.0.0-py3-none-any.whl
  ```

Then install the required dependencies:

```bash
pip install -r requirements.txt
```

## Running the script

Help:

```bash
py ./batch_a2f.py --help
```

```bash
py ./batch_a2f.py

Optional argument -url: default "127.0.0.1:52000"
Optional argument -mhm: default "MH_Mapping.json"
Optional argument -bl: default "BatchList.json"

Examples:
py ./batch_a2f.py
py ./batch_a2f.py -url "127.0.0.1:52000" -mhm "MyOwnMapping.json" -bl "MyOwnBatchList.json"
```

## Other files required by the script

The script requires the batch list and the MetaHuman mapping.json files.

BatchList.json file format:

```json
{
    "audiofiles_folder": "Full Path/To/AudioFiles/Folder",
    "paramsfiles_folder": "Full Path/To/ConfigFiles/Folder",
    "destination_folder": "Full Path/To/ExportFiles/Folder",
    "audiofiles": [
        ["Audio_Example.wav", "Config_Example.json"],
        ["Audio_Example2.wav", "Config_Example2.json"],
        ["Audio_Example3.wav", "Config_Example3.json"]
    ],
    "FBX_axis_system": "Unreal"
}
```

Notes:

- Config files are saved from Maya. Test parameters in Maya on your character, save those settings from the ACE menu. It will save a .json file.
- Each audio file must be assigned a config file. Use the example provided if none have been prepared in Maya.
- "FBX_axis_system" can be "Unreal" or "Maya" (if you plan to load the FBX on MetaHuman rig in Maya, use Maya. If you load in Unreal, use Unreal).
