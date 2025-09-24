# Build and Test

## Table of Contents

- [Setting up Development Environment](#setting-up-development-environment)
  - [Build Tools](#build-tools)
  - [CUDA Toolkit](#cuda-toolkit)
  - [TensorRT](#tensorrt)
  - [Audio2Face-3D SDK](#audio2face-3d-sdk)
  - [Maya DevKit](#maya-devkit)
- [Building Maya ACE Plugin](#building-maya-ace-plugin)
  - [Build Commands](#build-commands)
  - [Output Files and Packaging](#output-files-and-packaging)
- [Testing](#testing)
  - [Maya Plugin Tests](#maya-plugin-tests)
  - [ACE Client Library Tests](#ace-client-library-tests)
- [Troubleshooting](#troubleshooting)

## Setting up Development Environment

Please check the [requirements](requirements.md#build-environment-for-developers) and make sure the build tools are installed.

### Build Tools

Please ensure the following build tools are installed and accessible in your PATH environment variable.

- MSBuild (Visual Studio 2019+)
- Python 3.10

> **Note:** CMAKE and Jinja2 are build dependencies that will be automatically downloaded through `fetch_deps.bat`. These tools are required for generating build files and processing templates. You can also install them manually and add them to your PATH if you prefer to use your own versions.

### CUDA Toolkit

Maya ACE uses Audio2Face-3D SDK which requires NVIDIA CUDA for GPU acceleration and inference. You may skip this step if you already set up to build the Audio2Face-3D SDK.

> Minimum Required Version: CUDA 12.9.x or higher

1. Download and Install [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads)
1. Verify Installation: Run `nvcc --version` in your terminal to confirm CUDA is properly installed.
1. Environment Setup:
    - No additional steps should be required. CMake should automatically detect the CUDA Toolkit if it is installed in the standard location.
    - If you have multiple CUDA versions installed or custom install process, you can specify the version to use by setting the `CUDA_PATH` environment variable.
    - **NOTE:** Please remove other versions of CUDA if you have multiple CUDA versions installed.

    ```powershell
    # Example: CUDA 12.9
    $env:CUDA_PATH="C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.9"
    ```

### TensorRT

Maya ACE uses TensorRT for GPU acceleration and inference. You may skip this step if you already set up to build the Audio2Face-3D SDK.

1. Download and Install [TensorRT](https://developer.nvidia.com/tensorrt)
1. Environment Setup:
    - TensorRT is distributed as a ZIP archive and does not have a standard installation path, so additional configuration is required to specify its location.
    - Make sure the TensorRT libraries and binaries are in your PATH.
    - Create `TENSORRT_ROOT_DIR` env variable pointing to the root folder of your downloaded TensorRT.
        - This environment variable is used only by `build.bat`. For those who want to run CMake manually, you can refer to the `-DTENSORRT_ROOT_DIR` option.

    ```powershell
    # Example: TensorRT-10.13.3.6
    C:\path\to\TensorRT-10.13.3.6
    ├───bin
    ├───include
    ├───lib
    ...etc

    $env:TENSORRT_ROOT_DIR="C:\path\to\TensorRT-10.13.3.6"
    ```

### Audio2Face-3D SDK

Audio2Face-3D SDK is the core library for Audio2Face-3D. You may need to build the SDK first before starting to build Maya ACE.

1. Download the [Audio2Face-3D SDK](https://github.com/NVIDIA/Audio2Face-3D-SDK) and follow the instructions to build the SDK.
    > Use the same versions of CUDA Toolkit and TensorRT to build the SDK and Maya-ACE.
1. Create `A2X_SDK_ROOT` env variable pointing to the audio2x-sdk build output directory or the root folder of your downloaded Audio2Face-3D SDK.
    - This environment variable is used only by [`build.bat`](../build.bat). For those who want to run CMake manually, you can refer to the `-DA2X_SDK_ROOT` option.

    ```powershell
    # Example: Audio2Face-3D SDK build output directory
    C:\path\to\Audio2Face-3D-SDK\_build\windows-x86_64\release\audio2x-sdk
    ├───bin
    ├───include
    ├───lib
    ...etc

    $env:A2X_SDK_ROOT="C:\path\to\Audio2Face-3D-SDK\_build\windows-x86_64\release\audio2x-sdk"
    ```

### Maya DevKit

Maya devkit is required to build Maya plugins, and it must be located at the correct path. Please download at least one supported version of Maya devkit, and locate it as explained below.

1. Download Maya devkit from [Autodesk website](https://aps.autodesk.com/developer/overview/maya).
2. Extract the downloaded devkit archive.
3. Create a `deps` directory in your repository root if it doesn't exist.
4. Copy the extracted `devkitBase` folder into the `deps` directory and rename it according to your Maya version:
   - For Maya 2024: rename to `devkitBase2024`
   - For Maya 2025: rename to `devkitBase2025`
   - For other versions: follow the pattern `devkitBase{YEAR}`

   Please refer to [FindMaya.cmake](../cmake/modules/FindMaya.cmake) for the exact naming convention.

    ```powershell
    ./deps
    ├───devkitBase2024 # Could be other years depending on your build target
    │   ├───cmake
    │   ├───devkit
    │   ├───include
    │   ├───lib
    │   └───mkspecs
    └───devkitBase2025 # Optional. Can be one or multiple versions. Follow the naming convention.
        ├───cmake
        ├───devkit
        ├───include
        ├───lib
        └───mkspecs
    ```

#### Maya Version Support

**Compatibility Note:** Maya versions earlier than 2024 are not extensively tested. You may need to make adjustments for compatibility with older versions.

## Building Maya ACE Plugin

**Prerequisites:**

- Maya Devkit, CUDA Toolkit, TensorRT, and Audio2Face-3D SDK are properly set up.
- Python 3.10 is in the system path.
- Internet connection is available to download dependencies.

### Build Commands

**Why Python 3.10?** Maya 2024 uses Python 3.10 internally, and Maya ACE is developed with Python 3.10. It is safe to use the same version for compatibility.

```powershell
.\fetch_deps.bat
.\build.bat mace # generate builds at _build\windows-x86_64\release
```

#### Debug Build

```powershell
.\fetch_deps.bat debug
.\build.bat mace debug # generate debug builds at _build\windows-x86_64\debug
```

`fetch_deps.bat` will automatically generate a Python virtual environment and install dependencies described in `deps/target-deps.packman.xml`, `deps/build-deps.packman.xml`, and `deps/requirements.txt`.

### Output Files and Packaging

The plugin is built at `_build\windows-x86_64\release` by default. The output files are:

```powershell
release or debug
├───bin  # libraries and unit test applications
└───plugins
    ├───mace  # plugin and script files
    └───maya.mod  # Maya module file
```

To package the plugin, please copy or archive the `mace` directory and `mace.mod` file together. Follow the [install guide](prerequisites.md#install-the-plugin) to install the plugin.

> **Important:** If you used [`MACE_DEV_MODE`](prerequisites.md#environment-variables) during development (which creates symlinks instead of copying files), you must disable it before packaging. Otherwise, the package will contain broken symlinks instead of actual files.

✅ **Success Indicator:** After successful build, verify that the `_build\windows-x86_64\release\plugins` directory contains the `mace` folder and `maya.mod` file.

## Testing

### Maya Plugin Tests

#### Build TRT Engines

Some tests require TensorRT engines pre-built.
[Download the models](gettingstarted-a2f.md#download-ai-models)
under the mace install location, and use below script or open
[TRT Model Manager](gettingstarted-a2f.md#build-trt-engines)
through Maya.

```powershell
.\trtgen.bat build *
```

> Please refer to [managing models directory](gettingstarted-a2f.md#managing-models-directory) for more information.

#### Run Tests for the Maya Plugin

```powershell
# run unit test with mayapy
.\test_maya_scripts.bat

# run tests which match the given substring
.\test_maya_scripts.bat -k test_plugin
```

The tests are located in `source/maya_ace/tests`.
`test_maya_scripts.bat` runs the tests with proper maya environment with running
[mock a2f service](#starting-a-local-mock-ace-server) on background.

#### Example: Overriding Maya Version for Tests (Powershell)

```powershell
$ENV:MAYA_LOCATION="C:\Program Files\Autodesk\Maya2025"
.\test_maya_scripts.bat
```

#### Example: Overriding Maya Version for Tests (Windows Command Prompt)

```cmd
set "MAYA_LOCATION=C:\Program Files\Autodesk\Maya2025"
.\test_maya_scripts.bat
```

### Launching Maya with test environment

```powershell
.\run_maya.bat
```

> Please follow the [install guide](prerequisites.md#install-the-plugin) to use the plugin outside of the `run_maya.bat` environment after testing.

### Starting a local mock ace server

```powershell
# run .\fetch_deps.bat if you haven't
.\run_mock_server.bat
```

### ACE Client Library Tests

The ACE Client Library is a static library that handles communication with ACE services.

**Build and run tests:**

```powershell
# Build test executables
.\build.bat tests_ace_client

# Run the tests
.\test_ace_client.bat
```

✅ **Success Indicator:** Tests should pass with output showing "All tests passed" or similar success message.

## ACE gRPC C++ Library

For information on updating gRPC generated files, see [grpc library readme](../source/ace_grpc_cpp/README.md).

## Troubleshooting

### Common Issues and Solutions

#### CMake Error: Could not find CUDAToolkit

Please check the [requirements](requirements.md#build-environment-for-developers) and make sure the CUDA Toolkit is installed. Make sure using the correct CUDA version and [CUDA_PATH](prerequisites.md#environment-variables) is set correctly.

#### CMake Error: Could not find TensorRT

Please check the [requirements](requirements.md#build-environment-for-developers) and make sure using the correct TensorRT version and [TENSORRT_ROOT_DIR](prerequisites.md#environment-variables) is set correctly.

#### CMake Error: Could not find A2X_SDK

Please check the [requirements](requirements.md#build-environment-for-developers) and make sure using the correct Audio2X-SDK version(or build from source) and [A2X_SDK_ROOT](prerequisites.md#environment-variables) is set correctly.

Also, make sure the CUDA and TensorRT versions are matching or compatible to the Audio2X-SDK.

#### Python, Python Interpreter Path, and Virtual Environment

Maya-ACE uses python in many places. Please make sure the compatible version of python is set correctly. For example,

```powershell
$env:PATH="C:\path\to\python.exe"
```

Also, Maya-ACE build process creates a virtual environment to install the required dependencies. If you encounter an error with python interpreter path, please delete `_venv` directory, and re-run `fetch_deps.bat`. This will initialize the virtual environment with the correct python interpreter path.

### Getting Help

If you encounter issues not covered here:

1. Check the [Maya script editor](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-7C861047-C7E0-4780-ACB5-752CD22AB02E) for detailed error messages
2. Review build logs in `_build` directory
3. Consult the [troubleshooting guide](troubleshooting.md) for more solutions
