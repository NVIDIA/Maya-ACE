# Maya ACE

## A Reference Client Implementation for NVIDIA ACE Audio2Face Service

Maya-ACE is a reference implementation designed as a client for the ACE Audio2Face service, which leverages NVIDIA's cutting-edge Digital Human Technology to generate high-quality, audio-driven facial animation. With Maya-ACE, users can effortlessly access and utilize the Audio2Face service through a simple, streamlined interface, or dive into the source code to develop their own custom clients.

This repository includes a Maya plugin, gRPC client libraries, test assets, and a sample scene—everything you need to explore, learn, and innovate with the ACE Audio2Face service. Whether you're looking to integrate this technology into your own projects or simply experiment with its capabilities, Maya-ACE provides a robust foundation.

The project is released under the MIT license, granting you freedom to use and modify the code, with the responsibility to ensure its appropriate use. Please note that NVIDIA assumes no liability for any issues arising from the use of this software.

The ACE Audio2Face service is accessible through [NVIDIA NIM](https://build.nvidia.com/nvidia/audio2face)
, and all the information for the service can be found from the [Document Portal](https://docs.nvidia.com/ace/latest/modules/a2f-docs/index.html).

![preview](/docs/resource/samplescene_play.gif)

## Contents

- [What can be done](#what-can-be-done)
- [Requirements](#requirements)
- [Getting Started](#getting-started)
  - [Quickstart](#quickstart)
  - [Set up with Sample Assets](#set-up-with-sample-assets)
- [User Interface](#user-interface)
- [Best Practices](#best-practices)
  - [Proposed Workflow](#proposed-workflow)
  - [Parameter Tuning Guide](#parameter-tuning-guide)
  - [Save Animation through Bake Animation](#save-animation-through-bake-animation)
  - [Connecting Custom Blendshapes](#connecting-custom-blendshapes)
- [Troubleshooting Tips](#troubleshooting-tips)
- [Build and Test](#build-and-test)
  - [Maya ACE Client plugin](#maya-ace-client)
  - [ACE Client library](#ace-client-library)
  - [ACE gRPC C++ Library](#ace-grpc-c-library)
- [Additional Knowledge](#additional-knowledge)
  - [Audio File Requirements](#audio-file-requirements)
  - [About ACE](#about-ace)

## What can be done

![overview](/docs/resource/mace_overview.svg)

### Send Audio, Receive Animation

Maya-ACE allows users to send audio inputs and receive corresponding facial animations. These animations can be directly connected to a blendshape node, enabling you to animate any character in Maya seamlessly.

### Learning with User-Friendly Interface

Maya-ACE provides all the necessary functionalities through a straightforward UI. It serves as an excellent tool for learning and experiencing the ACE Audio2Face service, helping users gain a deeper understanding of how it works.

### Seamless Transition to Unreal Engine and Tokkio

For those looking to expand their workflow, users can elevate their projects by transitioning to the [Kairos Unreal Engine integration](https://docs.nvidia.com/ace/latest/workflows/kairos/index.html) or [Tokkio: an interactive avatar virtual customer service assistant product SDK](https://docs.nvidia.com/ace/latest/workflows/tokkio/index.html).
This allows for the continued use of Audio2Face within other platforms, sharing the same parameters from Maya-ACE for a consistent experience.

### Integration with Standard Maya Nodes

Maya-ACE is designed to work seamlessly with standard Maya nodes, including the blendshape node, making it adaptable to drive any character in your scene.

### Customizable and Extendable

The source code and scripts provided with Maya-ACE can be modified to create a custom pipeline within Maya or to develop a client for other platforms, giving users the flexibility to tailor the tool to their specific needs.

## Requirements

- [Autodesk Maya](https://www.autodesk.com/products/maya/overview) 2023, 2024(recommended), or 2025
- Microsoft Windows 10 64bit or 11
- Internet connection
- [API key](https://build.nvidia.com/nvidia/audio2face/api) or a private [Audio2Face Service](https://catalog.ngc.nvidia.com/orgs/eevaigoeixww/teams/animation/helm-charts/a2f-service)

Other Maya versions and platforms can be supported in the future.

## Getting Started

### Quickstart

1. [Set an environment variable](https://learn.microsoft.com/en-us/previous-versions/office/developer/sharepoint-2010/ee537574(v=office.14))
, `NVCF_API_KEY`,
with a valid [API key](https://build.nvidia.com/nvidia/audio2face/api)
    - To get a key, click the green `Get API Key` text on the right of the page and proceed with instructions.
<br /><img src="docs/resource/get_api_key.png" width="400" />
1. Download the `mace` package, unzip and
[copy contents to a Maya module path](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=Maya_SDK_Distributing_Maya_Plug_ins_DistributingUsingModules_InstallingModules_html)
<br /><img src="docs/resource/copy_mace_module.png" width="440" />
1. Download the [sample maya scene](sample_data/maya_project/scenes/sample_ace_animation_player.mb) and sample audio files:
[English(male)](sample_data/maya_project/sound/english_voice_male_p1_neutral.wav) or
[Chinese(female)](sample_data/maya_project/sound/chinese_voice_female_p01_neutral.wav)
1. Launch Maya, load `maya_aceclient` plugin, and open the sample scene.
<br /><img src="docs/resource/samplescene_static.png" width="440" />
1. Adjust [**Time Slider preferences**](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-51A80586-9EEA-43A4-AA1F-BF1370C24A64)
: framerate=30, playback speed=30fps, looping=once
<br /><img src="docs/resource/timeline_preferences.png" width="440" />
1. Disable `Cached Playback` on the Time Slider
<br /><img src="docs/resource/disable_cached_playback.png" width="440" />
1. [Import an audio into the Maya scene](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-CF2B0358-6946-4C9D-9F8C-A783921CAECC) and set it to the Time Slider sound
<br /><img src="docs/resource/import_audio_menu.png" width="440" />
1. Adjust **Time Slider range** to fit to the audio length
1. Select `AceAnimationPlayer1` node and open Attribute Editor
    - You may turn off **DAG Objects only** from the Display menu to see AceAnimationPlayer1 on the outliner window.
    <br /><img src="docs/resource/disable_dag_only.png" width="400" />
    <br /><img src="docs/resource/select_aceanimationplayer.png" width="400" />
    - To open attribute editor, click menu -> Windows -> General Editors -> Attribute Editor
    <br /><img src="docs/resource/open_attribute_editor.png" width="400" />
1. Click the option menu next to Audiofile attribute, and select the audio imported before.
<br /><img src="docs/resource/audiofile_menu.png" width="440" />
1. Click `Request New Animation` button. Wait for the **Received ### frames** message.
1. Click Maya's play button. Watch how face is moving

### Set up with sample assets

1. Get an API key and setup Maya-ACE for Maya. Please follow [Quickstart](#quickstart) 1. and 2.
1. Download sample fbx files:
[Mark](sample_data/mark_bs_arkit_v2.fbx),
[Claire](sample_data\claire_bs_arkit_v2.fbx)
1. Launch maya with a new scene. Load maya plugins; `maya_aceclient` and `fbxmaya`
<br /><img src="docs/resource/plugin_manager.png" width="440" />
1. Create references of sample fbx files
<br /><img src="docs/resource/reference_editor.png" width="440" />
1. Adjust [**Time Slider preferences**](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-51A80586-9EEA-43A4-AA1F-BF1370C24A64)
: framerate=30, playback speed=30fps, looping=once
<br /><img src="docs/resource/timeline_preferences.png" width="440" />
1. Disable `Cached Playback` on the Time Slider
<br /><img src="docs/resource/disable_cached_playback.png" width="440" />
1. Import a sample audio:
[English](sample_data/maya_project/sound/english_voice_male_p1_neutral.wav) or
[Chinese](sample_data/maya_project/sound/chinese_voice_female_p01_neutral.wav)
<br /><img src="docs/resource/import_audio.png" width="440" />
1. [Adjust timeslider range](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-827ED8CD-C6AA-4495-8B5E-2FC98C8D49EE)
to fit to the audio
<br /><img src="docs/resource/timeslider_waveform.png" width="440" />
1. Select `c_headWatertight_mid` of Mark, and click **menu->ACE->Attach a new Animation Player**
<br /><img src="docs/resource/menu_attach_player.png" width="440" />
    - (Optional) Tips to optimize the viewport for faces
      - Hide unused groups; such as **c_mouth_grp_mid**, **r_eye_grp_mid**, **l_eye_grp_mid**, **root**
      - Setup a viewport camera with focal length between **75mm** and **150mm** looking at the face(s)
      - Change viewport camera's **Near Clip Plane** to 10
      - Assign **standardSurface1** material to face(s)
1. Open Attribute Editor, Select `AceAnimationPlayer1` node
<br /><img src="docs/resource/open_attribute_editor.png" width="440" />
1. Enter
[valid `Network Address`, `Api Key`, and `Function ID`](https://build.nvidia.com/nvidia/audio2face/api)
on the Attribute Editor.
<br /><img src="docs/resource/attribute_network_info.png" width="440" />
1. To change audio, Select an option from the AudioFile, and also update Time Slider to use the same audio.
<br /><img src="docs/resource/select_audio.png" width="440" />
<br /><img src="docs/resource/change_timeslider_audio.png" width="440" />
1. Click `Request New Animation` button, and wait for the **Received ### frames** message.
1. Click Maya's play button. Check animation on Mark's face.
<br /><img src="docs/resource/mace_play_mark.gif" width="440" />
1. Select both `AceAnimationPlayer1` and `c_headWatertight_mid` of Claire in order,
and click **menu->ACE->Connect an existing Animation Player**
<br /><img src="docs/resource/select_to_connect.png" width="440" />
    - You may turn off **DAG Objects only** from the Display menu to see AceAnimationPlayer1 on the outliner window.
    <br /><img src="docs/resource/disable_dag_only.png" width="400" />
1. Click Maya's play button again. Check animation on both faces.
<br /><img src="docs/resource/mace_play_both.gif" width="440" />

## User Interface

### Attribute Editor - AceAnimationPlayer

#### Network and Audio

Information to connect ACE Audio2Face service, and audio file information to request animation.

<img src="docs/resource/ui_network_and_audio.png" width="480" />

- Network Address: A full url with protocol and port number. example: https://grpc.nvcf.nvidia.com:443
- Api Key: A valid api key acquired from the [web page](https://build.nvidia.com/nvidia/audio2face/api)
- Function Id: A valid function id that is specific for the service and an AI model.
Find a proper Function ID from the [web page](https://build.nvidia.com/nvidia/audio2face/api)
  - Examples (as of August 21, 2024)
    - Mark model: 945ed566-a023-4677-9a49-61ede107fd5a
    - Claire model: 462f7853-60e8-474a-9728-7b598e58472c
- Audiofile: A path to an audio file to request animation from the service. It can be also selected from imported audios through the drop down option.

#### Emotion Parameters

Parameters to control generated emotion and preferred(manual) emotion.
Audio2Face generate animation with the emotion input which includes generated emotion and preferred emotion.
[Please watch this video to understand how it works.](https://www.nvidia.com/en-us/on-demand/session/omniverse2020-om1537/)

<img src="docs/resource/ui_emotion_params.png" width="480" />

- Emotion Strength: the strength of the overall emotion; the total of auto emotion and preferred(manual) emotion.
  - emotion = emotion strength * (preferred weight * preferred emotion + (1.0 - preferred weight) * generated emotion)
- Preferred Emotion: Enable/disable and the ratio of the user driven emotion in the overall emotion (1.0 = 100% preferred emotion, 0.0 = 100% generated emotion).
- Auto Emotion: Parameters to control generated emotion.

#### Face Parameters

Parameters to control overal face animation.
Please check [Audio2Face Microservice documents](https://docs.nvidia.com/ace/latest/modules/a2f-docs/text/architecture/audio2face_ms.html)
for the updated information.

<img src="docs/resource/ui_face_params.png" width="480" />

#### Blendshape Multipliers

Override specific expressions by multiplying the Audio2Face result.

<img src="docs/resource/ui_bs_multipliers.png" width="480" />

#### Blendshape Offsets

Override specific expressions by adding constant values to the Audio2Face result;

- each output = (raw result * multipler) + offset

<img src="docs/resource/ui_bs_offsets.png" width="480" />

### Main Menu

<img src="docs/resource/ui_menu.png" width="480" />

### Attach a new Animation Player

Create a new AceAnimationPlayer and connect to the selected blendshape node

### Connect an existing Animation Player

Connect the select AceAnimationPlayer to the secondly selected blendshape node

### Export A2F Parameters

Export a json file with parameters from a AceAnimationPlayer

- **DISCLAIMER**: This export may not fully reflect recent updates from the server or ACE services. The following limitations may result in potential discrepancies:
  - Parameters not set and controlled by the Maya plugin will be exported with default values.
  - Default values in the exported configuration files may differ if the A2F server is deployed with custom settings.
  - To ensure consistency between the client and server, carefully compare the server's configuration with the exported parameters.
  - For the most accurate and up-to-date information, please refer to the official website or the server configuration.

## Best Practices

### Proposed Workflow

Generally, it requires multiple tries to find an optimal setting. Users are expected to work with an iterative process with using Maya-ACE in their projects.
All animations should be downloaded to see the result through any changes; such as paramters. The typical workflow is,

1. Adjust **Parameters**
2. Update animation by clicking **Request New Animation** button
3. **Play** the scene and check the animation with audio
4. **Repeat** from 1 to 3 until satisfied with the result
5. Bake animation of the blendshape weights
6. Save the scene

![](docs/resource/standard_workflow.png)

### Parameter Tuning Guide

Please check
[ACE Parameter Tuning Guide](https://docs.nvidia.com/ace/latest/modules/a2f-docs/text/param_tuning.html)
to understand individual parameters.

### Save Animation through Bake Animation

Maya provides
[`Bake Simulation`](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-A11424B4-8384-4832-B18D-01264E1A19D1)
to freeze and save animation that is downloaded from the service.

1. Select blendshape node and select all blendshape weights from **the channel box**
1. Click **Bake Simulation** through menu; (Animation Layout) menu -> Keys -> Bake Simulation
<br /><img src="docs/resource/bake_animation_dialog.png" width="440" />

### Connecting Custom Blendshapes

Maya-ACE assumes to use
[ARKit compatible blendshape set](https://developer.apple.com/documentation/arkit/arfaceanchor/blendshapelocation)
, but it is still usable with a partial set of blendshapes through this process.

Once AceAnimationPlayer receives animation, it will update output weights with blendshape names that is received from the service,
and then `Connect existing Animation Player` menu will use the names to find the best possible blendshape targets based on the names.

Rules to match blendshape names

- The blendshape name must match with one of the arkit face location
- Case insensitive

Steps to use the name-based connection

1. Finish receiving animation and update `Output Weights` of AceAnimationPlayer node with blendshape names
1. Select the AceAnimationPlayer node and the blendshape node to connect
1. Click `Connect existing Animation Player` from the menu -> ACE

<img src="docs/resource/partial_blendshape_2.png" width="480" />

## Troubleshooting Tips

### Communication errors

These are common errors when requesting new animation through Maya-ACE.

| Error Messages | Possible Reasons (not all) |
|----------------|----------------------------|
| Unauthenticated | wrong api key, using http for an https endpoint |
| no authorization was passed | missing api key |
| SSL_ERROR_SSL | mismatched SSL version |
| Deadline Exceeded | bad network connection, slow machine or service, long audio |
| Cloud credits expired | out of credits |
| DNS resolution failed | network error, wrong address |
| Connection refused | service is unavailable |

### Cannot attach or connect AceAnimationPlayer

To attach AceAnimationPlayer, the selection should be a blendshape node or a mesh node with one blendshape connected.

To connect an existing AceAnimationPlayer to a new blendshape node, the selection should be one AceAnimationPlayer node and a blendshape node or a mesh with a blendshape node connected. There could be errors when,

- Selection is not in the correct order
- Selected multiple AceAnimationPlayer node or multiple blendshape node
- Selected a mesh connected with multiple blendshape nodes

Please make sure the selection is correct for each operation.

## Build and Test

### Maya Ace Client

#### Setting up Development Environment

##### Step 1: Build Tools
Please install the following build tools are accessible in your PATH environment variable.
1. MSBuild (Visual Studio 2019+)
2. python3

##### Step 2: Prepare Maya devkit
To build Maya plugins, Maya devkit is required and must be located at the correct path.

Download Maya devkit from [Autodesk website](https://aps.autodesk.com/developer/overview/maya)
and locate the devkitBase directory under `deps` directory of the local repo.

```
./deps
└───devkitBase
    ├───cmake
    ├───devkit
    ├───include
    ├───lib
    └───mkspecs
```

To run maya tests and use plugins, Maya application is required, and ***MAYA_LOCATION*** environment variable should be set to the [Maya's install directory](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-228CCA33-4AFE-4380-8C3D-18D23F7EAC72).

By default, Maya 2024 is expected, and the build script will copy plugin(s) to `_build/windows-x86_64/release/plguins/mace/plug-ins/2024`.

##### Step 3: Download Dependencies

The script will download required dependencies and place in `_build\target-deps`

```powershell
.\fetch_deps.bat
```

For convenience, Premake5 will be downloaded to `.\deps\premake`. Please use this version for subsequent builds or add it to your PATH variable. If you already have Premake5 installed, it should also be compatible.

#### Local Build

For a quick release build with Visual Studio 2022

```powershell
build.bat
```

To specify visual Studio version other than 2022, please change vs2022 to your installed Visual Studio version

- To run MSBuild.exe, it needs to use Developer Powershell or to set up the system path.

```powershell
# build maya_acecleint plugin
premake5.exe vs2022 --solution-name=maya-ace --aceclient_log_level=1
MSBuild.exe /NoWarn:MSB8003,MSB8005 .\_compiler\maya-ace.sln /p:Configuration=release /p:Platform=x64 /verbosity:minimal /m /t:mace

# for debug log level
premake5.exe vs2022 --solution-name=maya-ace --aceclient_log_level=3
```

#### Running Unit Tests

Note: Please build first before running the test. The build process will also pull pip packages needed for unit tests.

```powershell
# run unit test with mayapy
.\test_maya.bat

#  run tests which match the given substring
.\test_maya.bat -k test_plugin
```

##### Example: Overriding Maya Version for Tests (Powershell)

```powershell
$ENV:MAYA_LOCATION="C:\Program Files\Autodesk\Maya2025"
.\test_maya.bat
```

##### Example: Overriding Maya Version for Tests (Windows Command Prompt)

```powershell
SET "MAYA_LOCATION=C:\Program Files\Autodesk\Maya2025"
.\test_maya.bat
```

#### Launching Maya with test environment

```powershell
.\run_maya.bat
```

#### Starting a local mock ace server

```bash
.\venv\Scripts\activate
.\run_mock_server.bat
```

### ACE Client Library

A Static library that handles communication with ACE; sends and receives data.

#### Local Build

For a quick release build with Visual Studio 2022

```powershell
build.bat
```

To specify visual Studio version other than 2022, please change vs2022 to your installed Visual Studio version

- To run MSBuild.exe, it needs to use Developer Powershell or to set up the system path.

```powershell
# build aceclient test
premake5.exe vs2022 --solution-name=tests-aceclient --aceclient_log_level=1
MSBuild.exe /NoWarn:MSB8003,MSB8005 .\_compiler\tests-aceclient.sln /p:Configuration=release /p:Platform=x64 /verbosity:minimal /m /t:tests-aceclient
```

#### Running Unit Tests

```powershell
# run tests for aceclient
.\test_aceclient.bat
```

### ACE gRPC C++ Library

Please read [Generating the ACE gRPC module](https://github.com/NVIDIA/ACE/tree/main/microservices/audio_2_face_microservice/proto#readme) to know how to update the grpc generated files.

## Additional Knowledge

### Audio File Requirements

Maya-ACE uses [AudioFile](https://github.com/adamstark/AudioFile) to read audio files,
and supported formats may vary from
[Maya's supported audio file formats'](https://help.autodesk.com/view/MAYAUL/2024/ENU/?guid=GUID-CF2B0358-6946-4C9D-9F8C-A783921CAECC).

We recommend using the formats below for the best compatiblity and efficiency.

- file format: wav
- data format: 16bit PCM or 32bit IEEE Float
- samplerate: 16kHz, 32kHz, or 48kHz
- channels: 1 channel (mono)

### About ACE

> NVIDIA ACE is a suite of real-time AI solutions for end-to-end development of interactive avatars and digital human applications at-scale.

Additional information about ACE is available from [NVIDIA Documentation Hub](https://docs.nvidia.com/ace/latest/index.html).

For ACE customers to get support, please contact through [NVIDIA Enterprise Support](https://www.nvidia.com/en-us/support/enterprise/)
