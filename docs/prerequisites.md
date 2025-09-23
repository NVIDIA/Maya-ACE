# Prerequisites

## Table of Contents

- [Install the Plugin](#install-the-plugin)
- [Download the Sample Project](#download-the-sample-project)
- [Download AI Models](#download-ai-models)
- [Build TRT Engines](#build-trt-engines)
- [Prepare Access to the Service](#prepare-access-to-the-service)
- [Environment Variables](#environment-variables)

## Install the Plugin

1. Download the pre-built `mace` package from [releases](https://github.com/NVIDIA/Maya-ACE/releases), and unzip it.
    - Or [build the plugin from the source code](buildandtest.md#build-the-plugin).
    - The contents will look like below:
        ```text
        mace-v2.0.0/
        ├── mace/  # The plugin contents
        ├── PACKAGE-LICENSES/  # License files
        └── mace.mod  # Maya module file
        ```

2. Copy the contents to a [Maya module path](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=Maya_SDK_Distributing_Maya_Plug_ins_DistributingUsingModules_InstallingModules_html).
    - For example, `C:/Users/<username>/Documents/maya/modules/`
        ```text
        C:/Users/<username>/Documents/maya/modules
        ├── mace/
        ├── PACKAGE-LICENSES/
        └── mace.mod
        ```
    - Maya has multiple default module paths, and you can also set a custom module path through the [MAYA_MODULE_PATH](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-228CCA33-4AFE-4380-8C3D-18D23F7EAC72) environment variable. The following are some of the default module paths:
        ```text
        C:/Program Files/Autodesk/Maya<version>/modules
        C:/Users/<username>/Documents/maya/<version>/modules
        C:/Users/<username>/Documents/maya/modules
        C:/Program Files/Common Files/Autodesk Shared/Modules/maya/<version>
        ```

## Download the Sample Project

The sample project provides example scenes for learning Maya-ACE. Download the [sample project](../sample_project) or clone this repository.

```text
sample_project/
├── models/      # AI model files
├── scenes/      # Example Maya scenes
└── sound/       # Sample audio files
```

> **Next Steps:** Use the sample project with the [Quickstart](quickstart.md) and [Setup Guides](setupinstructions.md).

## Download AI Models

### Option 1: Automated Download (Recommended)

Use the provided scripts to automatically download all required models.

**Script Locations:**

| Script Path | Downloads To | Use When |
| ----------- | ------------ | -------- |
| [`/download_models.bat`](../download_models.bat) | `sample_project/models/` | - Working with repository.<br>- For first-time users. |
| `{maya module path}/mace/download_models.bat` | `mace/models/` | - Using binary package.<br>- Global model path. |

```powershell
# Run from the appropriate directory
.\download_models.bat
```

> **Important Notes:**
>
> - First-time users: Use `sample_project/models/` location
> - Maintain the [required directory structure](../sample_project/models/README.md#instructions)
> - Hugging Face authentication may be required

### Option 2: Manual Download

Download models directly from the [NVIDIA Hugging Face collection](https://huggingface.co/collections/nvidia/audio2face-3d-6865d22d6daec4ac85887b17).

1. Visit the model collection page
2. Download required models
3. Follow the [placement instructions](../sample_project/models/README.md)

> **Note:** Model availability may vary by region.

### Model Paths

- To use models in the sample project, locate them under the `sample_project/models` directory. This is the default path for the sample scenes, and it is recommended for the first-time users.
  For example:
    ```text
    sample_project/
    └── models/
        ├── audio2face-models/
        │   ...
        │   └── audio2face-3d-v3.0/
        └── audio2emotion-models/
            └── audio2emotion-v2.2/
    ```

- To use models globally, locate them under the mace install directory in a Maya module path.
  For example:
    ```text
    {maya module path}/mace/
    └── models/
        ├── audio2face-models/
        │   ...
        │   └── audio2face-3d-v3.0/
        └── audio2emotion-models/
            └── audio2emotion-v2.2/
    ```

- To use custom model paths, please set `A2F_MODEL_DIRS` and `A2E_MODEL_DIRS` [environment variables](#environment-variables) to the model directories.
  For example:
    ```powershell
    # An example of customizing Audio2Face-3D models
    $env:A2F_MODEL_DIRS = "D:\models\audio2face-models"
    # An example of customizing Audio2Emotion models
    $env:A2E_MODEL_DIRS = "D:\models\audio2emotion-models"
    ```

## Build TRT Engines

> [Download the models](#download-ai-models) to the `sample_project/models` directory if not done yet.

1. Launch Maya and set **project** to the [sample project](../sample_project) directory
   <br /><img src="resource/set-sample-project.png" width="440" alt="Maya Set Project dialog" />
    - Open Maya and select `File -> Set Project`
    - Select the sample project directory
    - Click `OK`

1. Open the TRT Model Manager window through `menu -> ACE/A2F -> Model -> Open TRT Model Manager`, or click the `Manage Model Files` button on the A2FAnimationPlayer1 attribute editor.
1. Select all audio2face and audio2emotion models, and click the `(Re)Build TRT Files` button
    <br /><img src="resource/window-trt-manager.png" width="400" alt="TRT Model Manager window" />
1. Check the [TRT Manager document](userinterface.md#open-trt-model-manager) to know more about the TRT Manager window.

## Prepare Access to the Service

> **Important:** Audio2Face-3D cloud services will be discontinued after July 25, 2025. Please plan to migrate to local/private service deployment.

### Choose Your Access Method

- **Local/Private Service (Recommended)**: Deploy your own hosted service and configure its URL. No API key required.
- **Cloud Service (NVCF)**: Obtain an API key and configure the environment variable.

### Local/Private Service

The Audio2Face-3D Service is a containerized microservice that can be deployed on-premises or in private cloud environments.

1. Deploy the service following the [Audio2Face-3D Microservice documentation](https://docs.nvidia.com/ace/audio2face-3d-microservice/1.3/text/deployment/container-config.html)
2. Configure the service address (e.g., `http://127.0.0.1:50051`) in the **Service Address** field of the **AceAnimationPlayer** node

### Cloud Service

NVIDIA provides the Audio2Face-3D NIM on NVIDIA Cloud Functions (NVCF) at `https://grpc.nvcf.nvidia.com:443`.

> **Note:** Service availability varies by region and capacity. Check the [current status](https://build.nvidia.com/nvidia/audio2face-3d/api).

1. Get an [API key](https://build.nvidia.com/nvidia/audio2face-3d/api), click the green `Get API Key` text on the right of the page and follow the instructions.
    <br /><img src="resource/get-api-key.png" width="440" alt="Get API Key button location" />

2. [Set an environment variable](https://learn.microsoft.com/en-us/previous-versions/office/developer/sharepoint-2010/ee537574(v=office.14)) `NVCF_API_KEY` with a valid API key.

## Environment Variables

There are some environment variables that can be set to customize the behavior of Maya-ACE.

| Variable | Description | Scope | Default Value |
| -------- | ----------- | ------------- | ------- |
| A2F_MODEL_DIRS | Directories of the Audio2Face-3D models, separated by `;`. | local inference | `sample_project/models/audio2face-models;{mace_module_path}/models/audio2face-models` |
| A2E_MODEL_DIRS | Directories of the Audio2Emotion models, separated by `;` | local inference | `sample_project/models/audio2emotion-models;{mace_module_path}/models/audio2emotion-models` |
| NVCF_API_KEY | The API key for the NVIDIA Cloud Functions used by default settings. | cloud service | |
| A2X_SDK_ROOT | The directory of the Audio2Face-3D SDK(Audio2X-SDK). | build | [`deps/audio2x-sdk`](../build.bat#L27) |
| TENSORRT_ROOT_DIR | The directory of the TensorRT. | build | [`deps/tensorrt`](../build.bat#L13) |
| CUDA_PATH | The directory of the CUDA Toolkit. | build | [`deps/cuda`](../build.bat#L20) |
| MACE_DEV_MODE | Development mode for the [mace build output](../source/maya_ace/CMakeLists.txt#L72). Switches to use symlinks for scripts instead of copying. | build | [`0`](../build.bat#L11) |

## Audio File Requirements

Maya-ACE leverages the [AudioFile](https://github.com/adamstark/AudioFile) library to read audio data, providing support for various WAV and AIFF files. However, when importing audio into the Maya timeline, users must adhere to the file format constraints set by Maya. Refer to [Maya's supported audio file formats](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-CF2B0358-6946-4C9D-9F8C-A783921CAECC) for more details on compatibility.

### Important Note

As of Maya 2025, only **PCM** audio formats are supported. Please ensure your audio files are in the correct format for successful import.

### Recommended Audio Specifications

For optimal compatibility and performance, it is recommended to use the following audio file specifications with Maya-ACE:

- File Format: WAV
- Data Format: 16-bit PCM
- Sample Rate: 16 kHz, 32 kHz, or 48 kHz
- Channels: Mono (1 channel)

By following these specifications, you can ensure the smooth integration of audio files within Maya and avoid common import issues.
