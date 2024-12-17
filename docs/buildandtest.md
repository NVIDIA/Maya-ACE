# Build and Test

## Setting up Development Environment

### Step 1: Build Tools

Please install the following build tools are accessible in your PATH environment variable.

1. MSBuild (Visual Studio 2019+)
2. python3

### Step 2: Prepare Maya devkit

To build Maya plugins, Maya devkit is required and must be located at the correct path.

Download Maya devkit from [Autodesk website](https://aps.autodesk.com/developer/overview/maya)
and locate the devkitBase directory under `deps` directory of the local repo.

```powershell
./deps
└───devkitBase
    ├───cmake
    ├───devkit
    ├───include
    ├───lib
    └───mkspecs
```

To build and test maya plugins, Maya 2024 or Maya 2025 is required and ***MAYA_LOCATION*** environment variable should be set to the [Maya's install directory](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-228CCA33-4AFE-4380-8C3D-18D23F7EAC72).

## Maya Ace Plugin

### Local Build

```powershell
.\fetch_deps.bat
.\build.bat mace # generate builds at _build\windows-x86_64\release

# to build debug version
.\fetch_deps.bat debug
.\build.bat mace debug # generate debug builds at _build\windows-x86_64\debug
```

### Running Unit Tests

Note: Please build first before running the test. The build process will also pull pip packages needed for unit tests.

```powershell
# run unit test with mayapy
.\test_maya.bat

#  run tests which match the given substring
.\test_maya.bat -k test_plugin
```

#### Example: Overriding Maya Version for Tests (Powershell)

```powershell
$ENV:MAYA_LOCATION="C:\Program Files\Autodesk\Maya2025"
.\test_maya.bat
```

#### Example: Overriding Maya Version for Tests (Windows Command Prompt)

```powershell
SET "MAYA_LOCATION=C:\Program Files\Autodesk\Maya2025"
.\test_maya.bat
```

### Launching Maya with test environment

```powershell
.\run_maya.bat
```

### Starting a local mock ace server

```powershell
# run .\fetch_deps.bat if you haven't
.\run_mock_server.bat
```

## ACE Client Library

A Static library that handles communication with ACE; sends and receives data.

### Local Build and Running Unit Tests

```powershell
.\build.bat tests-aceclient
.\test_aceclient.bat
```

## ACE gRPC C++ Library

Please read [grpc library readme](source/ace_grpc_cpp/README.md) to know how to update the grpc generated files.
